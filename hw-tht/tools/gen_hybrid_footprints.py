#!/usr/bin/python3.12
"""Generate the DIP/TSSOP "hybrid" footprints used by the PicoGUS THT board.

Each footprint carries a through-hole DIP pad set and an SMD TSSOP pad set with
identical pad numbers, so either package can be fitted:
  * DIP-14 + TSSOP-14: the TSSOP is rotated 90 deg and sits *inside* the DIP rows
    (a SOIC-14's ~6 mm lead span equals the DIP row gap, so SOIC cannot fit inside).
  * DIP-20 + TSSOP-20: the TSSOP sits *beside* the DIP (too long to fit inside).
Pads with the same number are the same net; the PCB traces join them.

Run with KiCad 10's Python:  python3.12 gen_hybrid_footprints.py <output .pretty dir>
"""
import os, sys
sys.path.insert(0, '/usr/lib/python3/dist-packages')
import pcbnew
from pcbnew import VECTOR2I, FromMM, EDA_ANGLE, DEGREES_T

STD = '/usr/share/kicad/footprints'
LIB_NAME = 'PicoGUS-THT'


def load(lib, name):
    fp = pcbnew.FootprintLoad(os.path.join(STD, lib + '.pretty'), name)
    assert fp is not None, name
    return fp


def merge_smd(dip, smd, rot_deg, dx_mm, dy_mm, label):
    """Copy SMD pads + fab outline of `smd` into `dip`, rotated then translated."""
    ang = EDA_ANGLE(rot_deg, DEGREES_T)
    off = VECTOR2I(FromMM(dx_mm), FromMM(dy_mm))
    origin = VECTOR2I(0, 0)
    for pad in smd.Pads():
        p = pcbnew.PAD(pad)
        p.SetParent(dip)
        p.Rotate(origin, ang)
        p.Move(off)
        dip.Add(p)
    for item in smd.GraphicalItems():
        if item.GetLayer() not in (pcbnew.F_Fab, pcbnew.F_SilkS, pcbnew.F_CrtYd):
            continue
        if item.GetClass() != 'PCB_SHAPE':
            continue
        g = item.Duplicate()
        g.SetParent(dip)
        g.Rotate(origin, ang)
        g.Move(off)
        dip.Add(g)


def bbox_courtyard(fp, margin_mm=0.25):
    """Replace courtyard with one rectangle around every pad/fab shape."""
    for item in list(fp.GraphicalItems()):
        if item.GetLayer() == pcbnew.F_CrtYd:
            fp.Remove(item)
    xs, ys = [], []
    for pad in fp.Pads():
        bb = pad.GetBoundingBox()
        xs += [bb.GetLeft(), bb.GetRight()]; ys += [bb.GetTop(), bb.GetBottom()]
    for item in fp.GraphicalItems():
        if item.GetLayer() == pcbnew.F_Fab:
            bb = item.GetBoundingBox()
            xs += [bb.GetLeft(), bb.GetRight()]; ys += [bb.GetTop(), bb.GetBottom()]
    m = FromMM(margin_mm)
    grid = FromMM(0.05)
    snap = lambda v, up: (v // grid + (1 if up and v % grid else 0)) * grid
    x0, x1 = snap(min(xs) - m, False), snap(max(xs) + m, True)
    y0, y1 = snap(min(ys) - m, False), snap(max(ys) + m, True)
    r = pcbnew.PCB_SHAPE(fp, pcbnew.SHAPE_T_RECT)
    r.SetLayer(pcbnew.F_CrtYd)
    r.SetStart(VECTOR2I(x0, y0)); r.SetEnd(VECTOR2I(x1, y1))
    r.SetWidth(FromMM(0.05))
    fp.Add(r)


def add_text(fp, txt, x, y, layer, size=0.8):
    t = pcbnew.PCB_TEXT(fp)
    t.SetText(txt); t.SetLayer(layer)
    t.SetPosition(VECTOR2I(FromMM(x), FromMM(y)))
    t.SetTextSize(VECTOR2I(FromMM(size), FromMM(size)))
    t.SetTextThickness(FromMM(size * 0.15))
    fp.Add(t)


def add_dot(fp, x, y, layer, r=0.2):
    c = pcbnew.PCB_SHAPE(fp, pcbnew.SHAPE_T_CIRCLE)
    c.SetLayer(layer); c.SetFilled(True)
    c.SetCenter(VECTOR2I(FromMM(x), FromMM(y)))
    c.SetEnd(VECTOR2I(FromMM(x + r), FromMM(y)))
    c.SetWidth(0)
    fp.Add(c)


def finish(fp, name, descr, tags):
    fp.SetFPID(pcbnew.LIB_ID(LIB_NAME, name))
    fp.SetLibDescription(descr)
    fp.SetKeywords(tags)
    fp.Value().SetText(name)


def dip14_tssop14():
    dip = load('Package_DIP', 'DIP-14_W7.62mm')
    tssop = load('Package_SO', 'TSSOP-14_4.4x5mm_P0.65mm')
    # DIP origin = pin 1; DIP centre = (3.81, 7.62).  Rotating the TSSOP by -90 deg
    # (clockwise on screen) puts TSSOP pins 1-7 on the top row and 8-14 on the bottom.
    merge_smd(dip, tssop, -90, 3.81, 7.62, 'TSSOP-14')
    # TSSOP pin-1 marker on silk, just outside the top-right pad (pin 1)
    p1 = [p for p in dip.Pads() if p.GetNumber() == '1' and p.GetAttribute() == pcbnew.PAD_ATTRIB_SMD][0]
    pos = p1.GetPosition()
    add_dot(dip, pcbnew.ToMM(pos.x) + 0.55, pcbnew.ToMM(pos.y) - 0.6, pcbnew.F_SilkS, 0.15)
    bbox_courtyard(dip)
    finish(dip, 'DIP-14_W7.62mm_TSSOP-14_Hybrid',
           'Hybrid footprint: DIP-14 (300 mil) through-hole OR TSSOP-14 (4.4x5 mm, P0.65) '
           'rotated 90 deg inside the DIP rows. Same pad numbers = same pins; fit ONE package only.',
           'DIP TSSOP hybrid THT SMD alternative 14')
    return dip


def dip20_tssop20():
    dip = load('Package_DIP', 'DIP-20_W7.62mm')
    tssop = load('Package_SO', 'TSSOP-20_4.4x6.5mm_P0.65mm')
    # TSSOP-20 is too long to sit inside the DIP rows -> place it beside the DIP,
    # centred on the DIP's vertical centre, to the right of pins 11-20.
    merge_smd(dip, tssop, 0, 7.62 + 0.8 + 1.0 + 3.6, 11.43, 'TSSOP-20')
    p1 = [p for p in dip.Pads() if p.GetNumber() == '1' and p.GetAttribute() == pcbnew.PAD_ATTRIB_SMD][0]
    pos = p1.GetPosition()
    add_dot(dip, pcbnew.ToMM(pos.x) - 0.2, pcbnew.ToMM(pos.y) - 0.85, pcbnew.F_SilkS, 0.15)
    bbox_courtyard(dip)
    finish(dip, 'DIP-20_W7.62mm_TSSOP-20_Hybrid',
           'Hybrid footprint: DIP-20 (300 mil) through-hole OR TSSOP-20 (4.4x6.5 mm, P0.65) '
           'beside it. Same pad numbers = same pins; fit ONE package only.',
           'DIP TSSOP hybrid THT SMD alternative 20')
    return dip


def cap_disc_dual_pitch():
    """Radial MLCC footprint taking either 2.5 mm or 5.0 mm lead spacing (3 holes)."""
    fp = load('Capacitor_THT', 'C_Disc_D5.0mm_W2.5mm_P5.00mm')
    p2 = [p for p in fp.Pads() if p.GetNumber() == '2'][0]
    extra = pcbnew.PAD(p2)
    extra.SetParent(fp)
    extra.SetPosition(VECTOR2I(FromMM(2.5), 0))
    fp.Add(extra)
    finish(fp, 'C_Disc_D5.0mm_W2.5mm_P2.50mm_P5.00mm',
           'Radial ceramic capacitor, D5.0 x W2.5 mm body, accepts 2.5 mm OR 5.0 mm lead spacing '
           '(pin 2 has two holes)', 'C disc radial ceramic MLCC 2.5 5.0 dual pitch')
    return fp


def ferrite_axial_0805():
    """Axial THT bead/inductor (10.16 mm pitch) OR an 0805 bead OR a wire link."""
    fp = load('Resistor_THT', 'R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal')
    smd = load('Inductor_SMD', 'L_0805_2012Metric')
    merge_smd(fp, smd, 0, 5.08, 0, '0805')
    bbox_courtyard(fp)
    finish(fp, 'FerriteBead_Axial_P10.16mm_0805_Hybrid',
           'Hybrid ferrite bead footprint: small axial THT bead / bead-on-lead / axial inductor on '
           '10.16 mm pitch, OR an 0805 SMD bead between the holes, OR a wire link. Fit ONE only.',
           'ferrite bead axial 0805 hybrid THT SMD')
    return fp


def resistor_axial_0805():
    """1/4 W axial resistor (10.16 mm pitch) OR an 0805 SMD resistor between the holes."""
    fp = load('Resistor_THT', 'R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal')
    smd = load('Resistor_SMD', 'R_0805_2012Metric')
    merge_smd(fp, smd, 0, 5.08, 0, '0805')
    bbox_courtyard(fp)
    finish(fp, 'R_Axial_DIN0207_P10.16mm_0805_Hybrid',
           'Hybrid resistor footprint: 1/4 W axial THT resistor (DIN0207, 10.16 mm pitch) OR an '
           '0805 SMD resistor between the holes. Fit ONE only.',
           'resistor axial 0805 hybrid THT SMD')
    return fp


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), '..', LIB_NAME + '.pretty')
    os.makedirs(out, exist_ok=True)
    io = pcbnew.PCB_IO_MGR.FindPlugin(pcbnew.PCB_IO_MGR.KICAD_SEXP)
    for fp in (dip14_tssop14(), dip20_tssop20(), cap_disc_dual_pitch(), ferrite_axial_0805(),
               resistor_axial_0805()):
        io.FootprintSave(out, fp)
        print('wrote', fp.GetFPID().GetLibItemName())
