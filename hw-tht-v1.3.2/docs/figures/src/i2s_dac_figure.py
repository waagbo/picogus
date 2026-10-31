# Usage: python3 i2s_dac_figure.py NOW.json NOW.kicad_pro OLD.json OLD.kicad_pro OUT.png
#   NOW.json / NOW.kicad_pro  copper_dump.py output and project file of the routed v1.3.2 board
#   OLD.json / OLD.kicad_pro  the same for the 103.7 mm layout (git show 8dbb6c6:hw-tht-v1.3.2/PicoGUS-THT.kicad_{pcb,pro})
#   (the .kicad_pro supplies the net-class patterns that define the audio class)
# Draws i2s-around-dac.png: the I2S nets (BCK, DIN, LRCK) from the Pico (U1) to the DAC (U9, PCM5100A on a DIP-20
# adapter), the audio-class and DAC-output copper, U9's analog pins 1-10 and its CAPP/CAPM/VNEG/AVDD copper with a
# 1 mm keep-away, on v1.3.2 and on 8dbb6c6, plus close-ups at U9 pin 1. Every number printed is measured here.
# Needs Pillow, numpy, shapely; figlib.py next to this script.
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from figlib import *

NOWJ, NOWP, OLDJ, OLDP, OUT = sys.argv[1:6]
I2S = ('BCK', 'DIN', 'LRCK')
ANA = {'Net-(U9-CAPP)', 'Net-(U9-CAPM)', 'Net-(U9-VNEG)', '/PCM5100 audio/AVDD'}
NOT_DIGITAL = {'', 'GND', '+3V3', '+5V', '/VSYS', '/USB host port/VBUS_USB', '+12V', '-12V', '/Wavetable & line out/+12VA',
               '/Wavetable & line out/-12VA', '/Wavetable & line out/LINE_L', '/Wavetable & line out/LINE_R', 'Net-(U9-LDOO)'}
KEEP = 1.0

def prep(jp, pro):
    b = Board(jp, pro)
    d = b.d
    b.aud = {n for n in d['nets'] if b.netclass(n) == 'audio'} | {'/PCM5100 audio/DAC_OUTL', '/PCM5100 audio/DAC_OUTR'}
    b.u9a = [p for p in d['pads'] if p['ref'] == 'U9' and int(p['num']) <= 10]
    b.sens = {}          # per layer: audio, analog, U9 pins 1-10
    for L in ('F.Cu', 'B.Cu'):
        b.sens[L] = {'audio': b.copper(L, b.aud, pours=False), 'analog': b.copper(L, ANA, pours=False),
                     'u9': unary_union([p['g'][L] for p in b.u9a if L in p['g']])}
    b.keep = unary_union([g.buffer(KEEP, quad_segs=16) for L in b.sens for g in b.sens[L].values()])
    # I2S totals
    b.i2s_len = sum(t['cl'].length for t in d['tracks'] if t['net'] in I2S)
    b.i2s_vias = [v for v in d['vias'] if v['net'] in I2S]
    # nearest I2S copper (same layer; vias on both) to each sensitive set
    b.near = {}
    for key in ('audio', 'analog', 'u9'):
        best = None
        for L in ('F.Cu', 'B.Cu'):
            G = b.sens[L][key]
            if G.is_empty:
                continue
            items = [(t['net'], t['g']) for t in d['tracks'] if t['net'] in I2S and t['layer'] == L]
            items += [('via ' + v['net'], v['g']) for v in b.i2s_vias]
            for n, g in items:
                dd, pa, pb = gap(g, G)
                if best is None or dd < best[0]:
                    best = (dd, pa, pb, n, L)
        b.near[key] = best
    # digital copper (any net that is not power, GND, audio, DAC analog or line out) within 1 mm, same layer
    b.dig_close = []
    b.dig_min = None
    from shapely.prepared import prep as prepared
    for L in ('F.Cu', 'B.Cu'):
        S_ = unary_union(list(b.sens[L].values()))
        near3 = prepared(S_.buffer(3.0, quad_segs=8))          # only items within 3 mm need an exact distance
        cand = [(t['net'], t['g'], t) for t in d['tracks'] if t['layer'] == L]
        cand += [('via ' + v['net'], v['g'], v) for v in d['vias']]
        for n, g, it in cand:
            net = n.replace('via ', '')
            if net in NOT_DIGITAL or net in b.aud or net in ANA or not near3.intersects(g):
                continue
            dd = g.distance(S_)
            if b.dig_min is None or dd < b.dig_min[0]:
                b.dig_min = (dd, n)
            if dd < KEEP:
                ln = it['cl'].intersection(S_.buffer(KEEP + it['w'] / 2, quad_segs=16)).length if 'cl' in it else 0.0
                b.dig_close.append((n, L, dd, ln, it))
    # I2S tracks crossing audio copper on the other layer
    b.cross = []
    for t in d['tracks']:
        if t['net'] in I2S:
            O = 'B.Cu' if t['layer'] == 'F.Cu' else 'F.Cu'
            if t['g'].intersects(b.sens[O]['audio']):
                b.cross.append(t)
    # LRCK (any I2S) length within 1 mm of U9 pins 1-10
    b.i2s_u9_len = 0.0
    b.i2s_u9_geo = []
    for t in d['tracks']:
        if t['net'] in I2S:
            K = b.sens[t['layer']]['u9'].buffer(KEEP + t['w'] / 2, quad_segs=16)
            x = t['cl'].intersection(K)
            if x.length > 0:
                b.i2s_u9_len += x.length
                b.i2s_u9_geo.append((t['net'], x.buffer(t['w'] / 2, cap_style=2).intersection(t['g'])))
    return b

now, old = prep(NOWJ, NOWP), prep(OLDJ, OLDP)
for name, b in (('v1.3.2', now), ('8dbb6c6', old)):
    print(name, 'I2S %.1f mm, %d vias' % (b.i2s_len, len(b.i2s_vias)))
    for k, v in b.near.items():
        print('   nearest I2S to %-6s %.3f mm (%s, %s)' % (k, v[0], v[3], v[4]))
    print('   digital within 1 mm:', [(n, L, round(dd, 3), round(ln, 2)) for n, L, dd, ln, _ in b.dig_close],
          'nearest digital %.3f (%s)' % b.dig_min)
    print('   I2S crossing audio on the other layer:', len(b.cross), ' I2S length within 1 mm of U9 pins 1-10: %.2f mm' % b.i2s_u9_len)

# ---------------- drawing
CTX_F = (226, 211, 211); CTX_B = (208, 214, 228); CTX_PAD = (232, 214, 160)
AUD_F = (196, 32, 120); AUD_B = (232, 128, 186)
ANA_C = (0, 150, 150)
I2S_F = (24, 88, 208); I2S_B = (110, 168, 245); I2S_EDGE = (16, 60, 150)
KEEP_C = (247, 196, 218)
U9_C = PAD

def draw(b, box_, S, SS=3, lw=1.0):
    p = Panel(*box_, S, SS=SS)
    d = b.d
    p.fill(b.keep, KEEP_C)
    for L, col in (('B.Cu', CTX_B), ('F.Cu', CTX_F)):
        for t in d['tracks']:
            if t['layer'] == L and t['net'] not in I2S and t['net'] not in b.aud and t['net'] not in ANA:
                p.fill(t['g'], col)
    for v in d['vias']:
        if v['net'] not in I2S:
            p.fill(v['g'], (190, 190, 196))
    for q in d['pads']:
        if q['ref'] == 'U9' and int(q['num']) <= 10:
            continue
        for L, g in q['g'].items():
            p.fill(g, CTX_PAD)
    for L, col in (('B.Cu', AUD_B), ('F.Cu', AUD_F)):
        for t in d['tracks']:
            if t['layer'] == L and t['net'] in b.aud:
                p.fill(t['g'], col)
            if t['layer'] == L and t['net'] in ANA:
                p.fill(t['g'], ANA_C if L == 'F.Cu' else (110, 200, 200))
    for q in d['pads']:
        if q['net'] in b.aud:
            for g in q['g'].values():
                p.fill(g, AUD_F)
        elif q['net'] in ANA:
            for g in q['g'].values():
                p.fill(g, ANA_C)
    for q in b.u9a:
        for g in q['g'].values():
            p.fill(g, U9_C)
            p.outline(g, (150, 60, 20), width=1.6 * lw)
    for L, col in (('B.Cu', I2S_B), ('F.Cu', I2S_F)):
        for t in d['tracks']:
            if t['layer'] == L and t['net'] in I2S:
                if L == 'B.Cu':
                    p.fill(t['g'].buffer(0.05), I2S_EDGE)
                p.fill(t['g'], col)
    for v in b.i2s_vias:
        p.fill(Point(v['pos']).buffer(v['dia'] / 2 + 0.12), I2S_EDGE)
        p.fill(v['g'], I2S_F)
    p.outline(b.keep, (205, 90, 150), width=1.3 * lw, dash=(7, 5))
    holes = [(v['pos'][0], v['pos'][1], v['drill']) for v in d['vias']]
    holes += [(q['pos'][0], q['pos'][1], q['hole'][0]) for q in d['pads'] if q['hole']]
    p.holes(holes, col=(255, 255, 255))
    return p

# ---------------- layout
pad = 36; gp = 32; S = 18.5
NB_ = (136.5, 66.0, 182.0, 120.0)
OB_ = (148.5, 66.0, 194.0, 120.0)
wP = round((NB_[2] - NB_[0]) * S); hP = round((NB_[3] - NB_[1]) * S)
SZ = 58
ZN = (141.6, 74.4, 149.4, 81.4)        # close-up at U9 pin 1, v1.3.2
ZO = (156.6, 74.4, 164.4, 81.4)        # close-up at U9 pin 1, 8dbb6c6
wZ = round((ZN[2] - ZN[0]) * SZ); hZ = round((ZN[3] - ZN[1]) * SZ)
W = pad + wP + gp + wP + gp + wZ + pad
head = 160
y1 = head + 44
stat_h = 120
H = y1 + hP + stat_h + 260
can = Image.new('RGB', (W, H), (255, 255, 255))
dr = ImageDraw.Draw(can)
dr.text((pad, 26), 'PicoGUS THT v1.3.2: I2S routed around the DAC\'s analog half', font=font(40, 'Bold'), fill=INK)
dr.text((pad, 82), 'BCK, DIN and LRCK from the Pico (U1) to the PCM5100A (U9, on a DIP-20 adapter). U9\'s pins 1-10 are its analog side (charge pump,',
        font=font(24), fill=GREY)
dr.text((pad, 114), 'outputs, AVDD); with the audio-class copper they get a 1 mm keep-away (pink). Both copper layers shown, from the component side.',
        font=font(24), fill=GREY)

def panel(b, box_, x, title):
    p = draw(b, box_, S)
    can.paste(p.done(), (x, y1)); frame(dr, x, y1, p.W, p.H)
    heading(dr, (x, y1 - 8), title, size=26)
    return Annot(can, p, x, y1)

xN = pad; xO = pad + wP + gp; xZ = xO + wP + gp
aN = panel(now, NB_, xN, 'v1.3.2 (102 mm)')
aO = panel(old, OB_, xO, '8dbb6c6 (103.7 mm)')

def labels(a, b, ox):
    # U9 and U1
    u9 = b.d['fps']['U9']['pos']
    a.box_label(a.P((u9[0] + 0.6, u9[1] - 9.7)), 'U9 PCM5100A (pins 11-20: digital)', size=18, col=INK, w='Bold')
    # the I2S nets named at their U9 pins
    for q in b.d['pads']:
        if q['ref'] == 'U9' and q['net'] in I2S:
            a.box_label(a.P((q['pos'][0], q['pos'][1] - 1.75)), q['net'], size=15, col=I2S_EDGE, w='Bold')
    p1 = next(p for p in b.u9a if p['num'] == '1'); p10 = next(p for p in b.u9a if p['num'] == '10')
    a.text((p1['pos'][0], p1['pos'][1] + 1.75), '1', size=17, col=(150, 60, 20), w='Bold')
    a.text((p10['pos'][0], p10['pos'][1] + 1.75), '10', size=17, col=(150, 60, 20), w='Bold')
    u1p = [p for p in b.d['pads'] if p['ref'] == 'U1' and p['net'] in I2S]
    ux = sum(p['pos'][0] for p in u1p) / len(u1p)
    a.box_label(a.P((ux, 119.2)), 'U1 (Pico) GP pins', size=18, col=INK, w='Bold')

labels(aN, now, xN)
labels(aO, old, xO)

def net_tags(a, b, spots):
    for net, pt in spots:
        a.box_label(a.P(pt), net, size=17, col=I2S_EDGE, w='Bold')

for net, x_, y_ in (('DIN', 142.6, 80.0), ('BCK', 143.3, 82.2), ('LRCK', 144.0, 84.4)):
    P0 = aN.P((x_, y_)); X = aN.P((140.6, y_))
    dr.line([P0, (X[0] + 4, X[1])], fill=I2S_EDGE, width=2)
    dr.ellipse([P0[0] - 3, P0[1] - 3, P0[0] + 3, P0[1] + 3], fill=I2S_EDGE)
    aN.box_label((X[0], X[1]), net, size=17, col=I2S_EDGE, w='Bold', anchor='rm')
net_tags(aO, old, (('LRCK', (159.7, 87.0)),))

# the old LRCK stretch inside the keep-away of pins 1-10
def overlay_red(a, b):
    for net, g in b.i2s_u9_geo:
        for q in polys(g):
            pts = [a.P(c) for c in q.exterior.coords]
            dr.polygon(pts, fill=RED)
overlay_red(aO, old)

def stats(x, y, b, lines):
    for i, (t, bold, col) in enumerate(lines):
        dr.text((x, y + i * 28), t, font=font(20, 'Bold' if bold else 'Medium'), fill=col)

ys = y1 + hP + 14
nn = now.near; no = old.near
stats(xN, ys, now, [('I2S: %.1f mm of track, %d vias' % (now.i2s_len, len(now.i2s_vias)), True, INK),
                    ('Nearest I2S copper: %.2f mm to U9 pin 1, %.2f mm to the DAC\'s' % (nn['u9'][0], nn['analog'][0]), False, SUB),
                    ('analog copper, %.2f mm to audio copper. No digital track or' % nn['audio'][0], False, SUB),
                    ('via within 1 mm (nearest: %s, %.2f mm).' % (now.dig_min[1].replace('via ', 'a via of '), now.dig_min[0]), False, SUB)])
lr = [x for x in old.dig_close if x[0] == 'LRCK']
stats(xO, ys, old, [('I2S: %.1f mm of track, %d via' % (old.i2s_len, len(old.i2s_vias)), True, INK),
                    ('LRCK runs past U9 pin 1 at %.2f mm: %.1f mm of it' % (no['u9'][0], old.i2s_u9_len), False, SUB),
                    ('lies within 1 mm of the analog pins (red).', False, SUB)])

# close-ups
def zoom(b, box_, y, title, which):
    p = draw(b, box_, SZ, lw=1.4)
    can.paste(p.done(), (xZ, y)); frame(dr, xZ, y, p.W, p.H)
    heading(dr, (xZ, y - 8), title, size=23)
    a = Annot(can, p, xZ, y)
    if which == 'old':
        for net, g in b.i2s_u9_geo:
            for q in polys(g):
                dr.polygon([a.P(c) for c in q.exterior.coords], fill=RED)
    n = b.near['u9']
    a.measure(n[1], n[2], '%.2f mm' % n[0], at=('px', 0, -40) if which == 'old' else ('px', -6, 52), size=18)
    if which == 'now':
        n2 = b.near['analog']
        a.measure(n2[1], n2[2], '%.2f mm' % n2[0], at=('px', 36, -30), size=18)
    p1 = next(q for q in b.u9a if q['num'] == '1')
    a.text((p1['pos'][0] + 0.15, p1['pos'][1] + 1.35), 'pin 1 (+3V3)', size=17, col=(150, 60, 20), w='Bold')
    return a

zy1 = y1
zoom(old, ZO, zy1, '8dbb6c6: LRCK at U9 pin 1', 'old')
dr.text((xZ, zy1 + hZ + 12), 'LRCK on F.Cu passes the pin %.2f mm away;' % old.near['u9'][0], font=font(19), fill=SUB)
dr.text((xZ, zy1 + hZ + 38), '%.1f mm of it inside the 1 mm keep-away.' % old.i2s_u9_len, font=font(19), fill=SUB)
zy2 = zy1 + hZ + 120
zoom(now, ZN, zy2, 'v1.3.2: nearest I2S at U9 pin 1', 'now')
nn_u9 = now.near['u9']; nn_an = now.near['analog']
an_net = min(ANA, key=lambda n: min((now.copper(L, {n}, pours=False).distance(Point(nn_an[2])) for L in ('F.Cu', 'B.Cu')), default=9))
dr.text((xZ, zy2 + hZ + 12), '%s (B.Cu) %.2f mm from pin 1; an %s via' % (nn_u9[3], nn_u9[0], nn_an[3].replace('via ', '')), font=font(19), fill=SUB)
dr.text((xZ, zy2 + hZ + 38), '%.2f mm from %s copper: both outside.' % (nn_an[0], an_net.replace('Net-(U9-', '').replace(')', '')), font=font(19), fill=SUB)

# legend
ly = y1 + hP + stat_h + 22
x = pad; fs = font(21)
x = swatch(dr, x, ly, 'I2S on F.Cu', fill=I2S_F, kind='line', fs=fs) - 8
x = swatch(dr, x, ly, 'I2S on B.Cu', fill=I2S_B, kind='line', fs=fs) - 8
dr.ellipse([x + 6, ly, x + 28, ly + 22], fill=I2S_EDGE); dr.ellipse([x + 9, ly + 3, x + 25, ly + 19], fill=I2S_F)
dr.ellipse([x + 13, ly + 7, x + 21, ly + 15], fill=(255, 255, 255))
dr.text((x + 44, ly + 11), 'I2S via', font=fs, fill=INK, anchor='lm'); x += 44 + dr.textlength('I2S via', font=fs) + 28
x = swatch(dr, x, ly, 'audio class + DAC outputs (F / B)', fill=AUD_F, kind='line', fs=fs) - 8
x = swatch(dr, x, ly, 'CAPP / CAPM / VNEG / AVDD', fill=ANA_C, kind='line', fs=fs) - 8
ly2 = ly + 36; x = pad
x = swatch(dr, x, ly2, 'U9 pins 1-10 (analog side)', fill=U9_C, outline=(150, 60, 20), fs=fs) - 8
x = swatch(dr, x, ly2, '1 mm keep-away', fill=KEEP_C, outline=(205, 90, 150), fs=fs) - 8
x = swatch(dr, x, ly2, 'I2S inside it', fill=RED, fs=fs) - 8
x = swatch(dr, x, ly2, 'other copper F.Cu', fill=CTX_F, kind='line', fs=fs) - 8
x = swatch(dr, x, ly2, 'other copper B.Cu', fill=CTX_B, kind='line', fs=fs) - 8
x = swatch(dr, x, ly2, 'other pads', fill=CTX_PAD, fs=fs)

vy = ly2 + 52; fz = font(21)
lines = (
    'Why accepted: the I2S nets get longer (%.1f mm with %d vias against %.1f mm with %d via on 8dbb6c6) because they go round the DAC\'s analog half'
    % (now.i2s_len, len(now.i2s_vias), old.i2s_len, len(old.i2s_vias)),
    'instead of past its pins: no digital track or via is within 1 mm of U9 pins 1-10, the CAPP/CAPM/VNEG/AVDD copper or any audio net, and no I2S',
    'track crosses an audio net on the other layer. The GND pours are not drawn.',
)
for ln in lines:
    dr.text((pad, vy), ln, font=fz, fill=SUB); vy += 30
can = can.crop((0, 0, W, vy + 22))
can.save(OUT)
print(can.size)
