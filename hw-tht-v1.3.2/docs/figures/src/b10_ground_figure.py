# Usage: python3 b10_ground_figure.py NOW.json OLD.json OUT.png
#   NOW.json  copper_dump.py output of the routed v1.3.2 board (PicoGUS-THT.kicad_pcb)
#   OLD.json  copper_dump.py output of the 103.7 mm layout (git show 8dbb6c6:hw-tht-v1.3.2/PicoGUS-THT.kicad_pcb)
# Draws b10-ground-finger.png: the GND tracks from the ISA ground fingers B1, B10 and B31 (J3.1/.10/.31) to the
# pours, their widths and lengths, the neighbours that keep B10's track narrow (with the clearance a 1.0 mm track
# would leave), and B10 on 8dbb6c6 for comparison. Every number printed is measured here from the two dumps.
# Needs Pillow, numpy, shapely; figlib.py next to this script.
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from figlib import *
from shapely.ops import polygonize, substring

NOW, OLD, OUT = sys.argv[1:4]

def prep(path):
    b = Board(path)
    b.outline = unary_union(list(polygonize(unary_union(b.edges))))
    b.fing = {p['num']: p for p in b.d['pads'] if p['ref'] == 'J3' and 'B.Cu' in p['g']}
    g = next(x for x in b.d['fps']['J3']['graphics'] if x['layer'] == 'B.Mask')
    b.common_open = unary_union([Polygon(c) for c in g['geo'][1]]).buffer(g['w'] / 2, join_style=2)
    b.rule = unary_union([poly_of(r['outline']) for r in b.d['rules'] if r['no_fill'] and not r['name']])
    return b

def near(a, c, e=0.03):
    return math.dist(a, c) < e

def trace(b, num):
    """GND segments from finger J3.num to the first segment whose copper touches a GND pour (same layer)"""
    pad = b.fing[num]['g']['B.Cu']
    gt = [t for t in b.d['tracks'] if t['net'] == 'GND']
    gv = [v for v in b.d['vias'] if v['net'] == 'GND']
    start = [t for t in gt if t['layer'] == 'B.Cu' and (pad.buffer(0.01).contains(Point(t['a'])) or pad.buffer(0.01).contains(Point(t['b'])))]
    # orient: walk from the finger outwards
    path, seen, vias = [], set(), []
    cur = max(start, key=lambda t: t['w'])
    a, c = (cur['a'], cur['b']) if pad.contains(Point(cur['a'])) else (cur['b'], cur['a'])
    if pad.contains(Point(cur['a'])) and pad.contains(Point(cur['b'])):
        a, c = (cur['a'], cur['b']) if cur['a'][1] > cur['b'][1] else (cur['b'], cur['a'])
    layer = cur['layer']
    while True:
        path.append({'t': cur, 'a': a, 'b': c, 'layer': layer})
        seen.add(id(cur))
        fill = b.fill.get(('GND', layer))
        if fill is not None and cur['g'].intersects(fill):
            break
        nxt = [t for t in gt if id(t) not in seen and t['layer'] == layer and (near(t['a'], c) or near(t['b'], c))]
        if not nxt:
            v = next((v for v in gv if near(v['pos'], c, 0.05)), None)
            if v is None:
                break
            vias.append(v)
            layer = 'F.Cu' if layer == 'B.Cu' else 'B.Cu'
            nxt = [t for t in gt if id(t) not in seen and t['layer'] == layer and (near(t['a'], c, 0.05) or near(t['b'], c, 0.05))]
            if not nxt:
                break
        cur = nxt[0]
        a, c = (cur['a'], cur['b']) if near(cur['a'], c, 0.05) else (cur['b'], cur['a'])
    return path, vias

def touch_len(b, seg_list, w, layer):
    """length along a chain of points until a disc of diameter w first touches the GND pour on that layer"""
    F = b.fill[('GND', layer)]
    s = 0
    for a, c in seg_list:
        L = math.dist(a, c); n = max(2, int(L / 0.005))
        for i in range(n + 1):
            p = (a[0] + (c[0] - a[0]) * i / n, a[1] + (c[1] - a[1]) * i / n)
            if Point(p).buffer(w / 2, quad_segs=16).intersects(F):
                return s + L * i / n, p
        s += L
    return s, c

def runs(path):
    """group consecutive segments by (layer, width)"""
    out = []
    for p in path:
        k = (p['layer'], p['t']['w'])
        if out and out[-1]['k'] == k and near(out[-1]['pts'][-1], p['a'], 0.05):
            out[-1]['pts'].append(p['b']); out[-1]['segs'].append((p['a'], p['b']))
        else:
            out.append({'k': k, 'pts': [p['a'], p['b']], 'segs': [(p['a'], p['b'])]})
    for r in out:
        r['len'] = sum(math.dist(a, c) for a, c in r['segs'])
    return out

def neighbours(b, segs, w, layer, exclude=('GND',), lim=0.5):
    """min clearance from a chain (at width w) to every other-net item on the layer, as {label: (gap, pa, pb)}"""
    g = unary_union([LineString(s).buffer(w / 2, quad_segs=24) for s in segs])
    out = {}
    def put(key, geo):
        dd = g.distance(geo)
        if dd < lim and (key not in out or dd < out[key][0]):
            _, pa, pb = gap(g, geo)
            out[key] = (dd, pa, pb)
    for t in b.d['tracks']:
        if t['layer'] == layer and t['net'] not in exclude:
            put(('trk', t['net']), t['g'])
    for v in b.d['vias']:
        if v['net'] not in exclude:
            put(('via', v['net']), v['g'])
    for p in b.d['pads']:
        if layer in p['g'] and p['net'] not in exclude:
            put(('pad', p['ref'] + '.' + p['num'], p['net']), p['g'][layer])
    return out

now, old = prep(NOW), prep(OLD)
P = {n: trace(now, n) for n in ('1', '10', '31')}
P_old = {n: trace(old, n) for n in ('1', '10', '31')}
R = {n: runs(P[n][0]) for n in P}
R_old = {n: runs(P_old[n][0]) for n in P_old}
for tag, RR in (('v1.3.2', R), ('8dbb6c6', R_old)):
    for n, rr in RR.items():
        print(tag, 'B%s:' % n, ' -> '.join('%s %.1f mm x %.2f mm' % (r['k'][0], r['k'][1], r['len']) for r in rr))

# B10 (now): the narrow runs
r10 = R['10']
rB = next(r for r in r10 if r['k'] == ('B.Cu', 0.6))
rF = next(r for r in r10 if r['k'][0] == 'F.Cu')
via10 = P['10'][1][0]
lenF, touchF = touch_len(now, rF['segs'], rF['k'][1], 'F.Cu')
in_finger = sum(r['len'] for r in r10 if r['k'] == ('B.Cu', 1.0))
r31 = R['31']
r31n = next(r for r in r31 if r['k'][1] < 1.0)
len31, touch31 = touch_len(now, r31n['segs'], r31n['k'][1], 'B.Cu')
r1 = R['1']
# clearances now and with a 1.0 mm track on the same centre lines
nbB = neighbours(now, rB['segs'], rB['k'][1], 'B.Cu')
nbB1 = neighbours(now, rB['segs'], 1.0, 'B.Cu', lim=0.35)
fsegs = []
s = 0
for a, c in rF['segs']:                      # the F.Cu run only up to where it touches the pour
    L = math.dist(a, c)
    if s + L >= lenF:
        fsegs.append((a, (a[0] + (c[0] - a[0]) * (lenF - s) / L, a[1] + (c[1] - a[1]) * (lenF - s) / L))); break
    fsegs.append((a, c)); s += L
# clearances over the whole narrow run (up to and including the segment that touches the pour)
nbF = neighbours(now, rF['segs'], rF['k'][1], 'F.Cu')
nbF1 = neighbours(now, rF['segs'], 1.0, 'F.Cu', lim=0.35)
nb31 = neighbours(now, r31n['segs'], r31n['k'][1], 'B.Cu')
nb31_1 = neighbours(now, r31n['segs'], 1.0, 'B.Cu', lim=0.35)
# IRQ2 is squeezed between the B10 track and the +5V run
irq_seg = [t for t in now.d['tracks'] if t['layer'] == 'B.Cu' and t['net'] == 'IRQ2']
p5_seg = [t for t in now.d['tracks'] if t['layer'] == 'B.Cu' and t['net'] == '+5V']
win_irq = box(172.5, 126.0, 175.5, 128.6)
irq_g = unary_union([t['g'] for t in irq_seg]).intersection(win_irq)
p5_g = unary_union([t['g'] for t in p5_seg]).intersection(win_irq.buffer(1))
irq_p5 = gap(irq_g, p5_g)
for nm, nb in (('B.Cu 0.6', nbB), ('B.Cu at 1.0', nbB1), ('F.Cu 0.5', nbF), ('F.Cu at 1.0', nbF1), ('B31 0.5', nb31), ('B31 at 1.0', nb31_1)):
    print(nm, sorted(((round(v[0], 3), k) for k, v in nb.items())))
print('B10 F.Cu to pour %.2f mm, B31 to pour %.2f mm; IRQ2-+5V %.3f' % (lenF, len31, irq_p5[0]))

def draw_layer(b, L, box_, S, path, extra_layer=None):
    p = Panel(*box_, S, SS=3)
    d = b.d
    p.fill(p.win.difference(b.outline), OUTSIDE)
    p.fill(b.common_open.intersection(b.outline), MASK_OPEN)
    if ('GND', L) in b.fill:
        p.fill(b.fill[('GND', L)], POUR)
    p.hatch(b.rule, (205, 205, 205), step=12, width=1.2)
    p.outline(b.rule, (150, 150, 150), width=1.5, dash=(9, 6))
    pid = {id(x['t']) for x in path}
    for t in d['tracks']:
        if t['layer'] != L or id(t) in pid:
            continue
        col = NET_COL.get(t['net'], SIG)
        if t['net'] == 'GND':
            col = (120, 185, 145)
        p.fill(t['g'], col)
    for v in d['vias']:
        p.fill(v['g'], NET_COL.get(v['net'], SIG) if v['net'] != 'GND' else (120, 185, 145))
    for q in d['pads']:
        if L in q['g']:
            p.fill(q['g'][L], PAD)
    for x in path:
        if x['layer'] == L:
            p.fill(x['t']['g'], NET_COL['GND'])
        elif extra_layer:
            p.fill(x['t']['g'], (150, 205, 170))
            p.outline(x['t']['g'], NET_COL['GND'], width=2, dash=(7, 5))
    p.holes([(v['pos'][0], v['pos'][1], v['drill']) for v in d['vias']], col=(255, 255, 255))
    p.holes([(q['pos'][0], q['pos'][1], q['hole'][0]) for q in d['pads'] if q['hole']], col=(255, 255, 255))
    p.outline(b.outline, (60, 60, 60), width=2)
    return p

GD = (20, 110, 65)       # dark green for GND labels
NB = (60, 62, 70)

def wlabel(a, pt, txt, dx, dy, col=GD, size=18):
    P0 = a.P(pt); X = (P0[0] + dx, P0[1] + dy)
    a.dr.line([P0, X], fill=col, width=2)
    a.dr.ellipse([P0[0] - 3, P0[1] - 3, P0[0] + 3, P0[1] + 3], fill=col)
    a.box_label(X, txt, size=size, col=col, w='Bold', anchor='mm')

def mid(segs):
    L = sum(math.dist(a, c) for a, c in segs); s = 0
    for a, c in segs:
        l = math.dist(a, c)
        if s + l >= L / 2:
            t = (L / 2 - s) / l
            return (a[0] + (c[0] - a[0]) * t, a[1] + (c[1] - a[1]) * t)
        s += l

def pick(nb, kind, net):
    return next(v for k, v in nb.items() if k[0] == kind and (k[-1] == net or k[1] == net))

# ---------------- layout
pad = 36; gp = 36; S = 66
BX = (169.8, 120.6, 180.0, 131.0)      # B10 now, both layers
BO = (169.5, 120.6, 175.3, 131.0)      # B10 on 8dbb6c6
B1X = (193.1, 123.6, 197.5, 131.0)
B31X = (116.3, 123.6, 121.4, 131.0)
wB = round((BX[2] - BX[0]) * S); hB = round((BX[3] - BX[1]) * S); wO = round((BO[2] - BO[0]) * S)
w1 = round((B1X[2] - B1X[0]) * S); w31 = round((B31X[2] - B31X[0]) * S); h2 = round((B1X[3] - B1X[1]) * S)
W = pad + wB + gp + wB + gp + wO + pad
head = 160
y1 = head + 40
cap_h = 118                              # captions under the row-1 panels
y2 = y1 + hB + cap_h + 70
H = y2 + h2 + 300
can = Image.new('RGB', (W, H), (255, 255, 255))
dr = ImageDraw.Draw(can)
dr.text((pad, 26), 'PicoGUS THT v1.3.2: why the B10 ground-finger track is narrower than B1\'s', font=font(40, 'Bold'), fill=INK)
dr.text((pad, 82), 'The ISA ground fingers B1, B10 and B31 each have a track into a GND pour. No pour may come down to the fingers (no-fill strip),',
        font=font(24), fill=GREY)
dr.text((pad, 114), 'so each track crosses the strip. At B10 the strip is crossed by IRQ2, +5V and ±12V on the solder side and by the data bus on the component side.',
        font=font(24), fill=GREY)

def tag(a, pt, txt, col, size=17):
    """net name on a white tag placed on its own track"""
    a.box_label(a.P(pt), txt, size=size, col=col, w='Bold', fill=(255, 255, 255))

def caption(x, y, lines, width):
    for i, (txt, bold, col) in enumerate(lines):
        dr.text((x, y + i * 27), txt, font=font(19, 'Bold' if bold else 'Medium'), fill=col)

# --- row 1: B10 now, B.Cu and F.Cu; B10 on 8dbb6c6
xB_ = pad; xF_ = pad + wB + gp; xO_ = xF_ + wB + gp
pB = draw_layer(now, 'B.Cu', BX, S, P['10'][0]); can.paste(pB.done(), (xB_, y1)); frame(dr, xB_, y1, wB, hB)
heading(dr, (xB_, y1 - 8), 'B10, v1.3.2: solder side (B.Cu)', size=25)
aB = Annot(can, pB, xB_, y1)
pF = draw_layer(now, 'F.Cu', BX, S, P['10'][0]); can.paste(pF.done(), (xF_, y1)); frame(dr, xF_, y1, wB, hB)
heading(dr, (xF_, y1 - 8), 'B10, v1.3.2: component side (F.Cu)', size=25)
aF = Annot(can, pF, xF_, y1)
pO = draw_layer(old, 'B.Cu', BO, S, P_old['10'][0], extra_layer=True); can.paste(pO.done(), (xO_, y1)); frame(dr, xO_, y1, wO, hB)
heading(dr, (xO_, y1 - 8), 'B10 on 8dbb6c6', size=25)
aO = Annot(can, pO, xO_, y1)

def finger_tags(a, b, lay, names):
    for num, txt in names:
        q = next(p for p in b.d['pads'] if p['ref'] == 'J3' and p['num'] == num)
        bb = q['g'][lay].bounds
        col = GD if q['net'] == 'GND' else NET_COL.get(q['net'], (90, 70, 20))
        if col == NET_COL.get('+12V'):
            col = (175, 85, 10)
        tag(a, ((bb[0] + bb[2]) / 2, 130.45), txt, col, size=17)

# B.Cu annotations
finger_tags(aB, now, 'B.Cu', (('10', 'B10 GND'), ('9', 'B9 +12V'), ('8', 'B8')))
r1in = [r for r in r10 if r['k'] == ('B.Cu', 1.0)]
wlabel(aB, r1in[-1]['pts'][-2], '1.0 mm', -84, -14)
wlabel(aB, (175.9, 127.3), '0.6 mm × %.1f mm' % rB['len'], 6, -94)
tag(aB, (178.25, 127.3), 'via', GD)
tag(aB, (170.55, 128.45), 'IRQ2', NB)
tag(aB, (170.7, 127.2), '+5V', NET_COL['+5V'])
tag(aB, (178.6, 128.3), '+12V', (175, 85, 10))
tag(aB, (175.3, 124.2), '+3V3', NET_COL['+3V3'])
g_irq = pick(nbB, 'trk', 'IRQ2'); g_b9 = next(v for k, v in nbB.items() if k[0] == 'pad' and k[1] == 'J3.9')
aB.measure(g_irq[1], g_irq[2], '%.2f' % g_irq[0], at=('px', 44, 34), size=18)
aB.measure(irq_p5[1], irq_p5[2], '%.2f' % irq_p5[0], at=('px', -44, -34), size=18)
aB.measure(g_b9[1], g_b9[2], '%.2f' % g_b9[0], at=('px', 48, 18), size=18)
c_irq = pick(nbB1, 'trk', 'IRQ2')[0]; c_b9 = next(v for k, v in nbB1.items() if k[0] == 'pad' and k[1] == 'J3.9')[0]
c_12 = pick(nbB1, 'trk', '+12V')[0]
yc = y1 + hB + 14
caption(xB_, yc, [('0.6 mm for %.1f mm to a via in the no-fill strip, between' % rB['len'], False, SUB),
                  ('IRQ2 (%.2f mm; IRQ2 is %.2f mm from +5V) and B9 (%.2f mm).' % (g_irq[0], irq_p5[0], g_b9[0]), False, SUB),
                  ('A 1.0 mm track on the same path: %.2f mm to IRQ2,' % c_irq, True, RED),
                  ('%.2f mm to B9, %.2f mm to the +12V track.' % (c_b9, c_12), True, RED)], wB)

# F.Cu annotations
finger_tags(aF, now, 'F.Cu', (('41', 'A10 I/OCHRDY'), ('40', 'A9 D0'), ('39', 'A8 D1')))
wlabel(aF, mid(fsegs), '0.5 mm × %.1f mm' % lenF, 96, 4)
tag(aF, (178.0, 127.25), 'via', GD)
TP = aF.P(touchF)
dr.ellipse([TP[0] - 13, TP[1] - 13, TP[0] + 13, TP[1] + 13], outline=GD, width=3)
X = (TP[0] + 70, TP[1] - 10)
dr.line([(TP[0] + 13, TP[1] - 2), (X[0] - 4, X[1])], fill=GD, width=2)
for i, t in enumerate(('touches the', 'front pour')):
    dr.text((X[0], X[1] + (i - 0.5) * 22), t, font=font(18, 'Bold'), fill=GD, anchor='lm', stroke_width=3, stroke_fill=(255, 255, 255))
g_d4 = pick(nbF, 'trk', 'D4'); g_d3 = pick(nbF, 'trk', 'D3')
g_c1 = next(v for k, v in nbF.items() if k[0] == 'pad' and k[1] == 'C1.1')
aF.measure(g_d4[1], g_d4[2], '%.2f' % g_d4[0], at=('px', 50, 6), size=18)
aF.measure(g_d3[1], g_d3[2], '%.2f' % g_d3[0], at=('px', -50, 20), size=18)
aF.measure(g_c1[1], g_c1[2], '%.2f' % g_c1[0], at=('px', -54, 26), size=18)
tag(aF, (177.65, 126.15), 'D4', NB)
tag(aF, (173.7, 126.4), 'D3', NB)
tag(aF, (174.2, 127.6), 'D2', NB)
tag(aF, (171.05, 124.45), 'C1 pad 1', NET_COL['+3V3'])
tag(aF, (171.0, 122.0), '+3V3', NET_COL['+3V3'])
c_d4 = pick(nbF1, 'trk', 'D4')[0]; c_c1 = next(v for k, v in nbF1.items() if k[0] == 'pad' and k[1] == 'C1.1')[0]
c_d3 = pick(nbF1, 'trk', 'D3')[0]
caption(xF_, yc, [('Then 0.5 mm for %.1f mm on the front until it touches the pour,' % lenF, False, SUB),
                  ('between D4 (%.2f mm), C1 pad 1 (+3V3, %.2f mm) and D3 (%.2f mm).' % (g_d4[0], g_c1[0], g_d3[0]), False, SUB),
                  ('A 1.0 mm track on the same path: %.2f mm to D4,' % c_d4, True, RED),
                  ('%.2f mm to C1 pad 1, %.2f mm to D3.' % (c_c1, c_d3), True, RED)], wB)

# 8dbb6c6 annotations
ro = R_old['10']
for r, (dx, dy) in zip(ro, ((56, 34), (70, 0), (64, 0))):
    txt = '%.1f mm%s' % (r['k'][1], ' (F.Cu)' if r['k'][0] == 'F.Cu' else '')
    wlabel(aO, mid(r['segs']), txt, dx, dy, size=17)
finger_tags(aO, old, 'B.Cu', (('10', 'B10 GND'), ('9', 'B9')))
caption(xO_, yc, [('1.0 mm all the way: B.Cu, a', False, SUB), ('1.0 mm F.Cu jumper between', False, SUB),
                  ('two vias, B.Cu into the pour.', False, SUB)], wO)

# --- row 2: B1 and B31 now, and the summary
x = pad
p1 = draw_layer(now, 'B.Cu', B1X, S, P['1'][0]); can.paste(p1.done(), (x, y2)); frame(dr, x, y2, w1, h2)
heading(dr, (x, y2 - 8), 'B1, v1.3.2 (B.Cu)', size=25)
a1 = Annot(can, p1, x, y2)
f1 = now.fing['1']['g']['B.Cu'].bounds
a1.text(((f1[0] + f1[2]) / 2, 130.4), 'B1 GND', size=19, col=GD, w='Bold')
wlabel(a1, (195.0, 127.4), '1.0 mm', -70, -40)
x += w1 + gp
p31 = draw_layer(now, 'B.Cu', B31X, S, P['31'][0]); can.paste(p31.done(), (x, y2)); frame(dr, x, y2, w31, h2)
heading(dr, (x, y2 - 8), 'B31, v1.3.2 (B.Cu)', size=25)
a31 = Annot(can, p31, x, y2)
f31 = now.fing['31']['g']['B.Cu'].bounds
a31.text(((f31[0] + f31[2]) / 2, 130.4), 'B31 GND', size=19, col=GD, w='Bold')
r31w = next(r for r in r31 if r['k'][1] == 1.0)
wlabel(a31, mid(r31w['segs']), '1.0 mm', 70, 26)
wlabel(a31, (r31n['pts'][0][0], (r31n['pts'][0][1] + touch31[1]) / 2), '0.5 mm', 74, -10)
g_v = pick(nb31, 'via', '+5V')
a31.measure(g_v[1], g_v[2], '%.2f' % g_v[0], at=('px', 30, -34), size=18)
tag(a31, (120.6, 126.6), '+5V', NET_COL['+5V'])

# summary table
xs = x + w31 + 60
ty = y2 + 4
dr.text((xs, ty), 'GND track from each finger to the pour', font=font(25, 'SemiBold'), fill=INK); ty += 46
cols = (xs, xs + 120, xs + 640)
f = font(20); fb = font(20, 'Bold')
dr.text((cols[0], ty), 'finger', font=fb, fill=GREY); dr.text((cols[1], ty), 'v1.3.2', font=fb, fill=GREY)
dr.text((cols[2], ty), '8dbb6c6', font=fb, fill=GREY); ty += 34
dr.line([xs, ty - 6, W - pad, ty - 6], fill=FRAME, width=1)

def desc(n):
    rr = R[n]
    if all(r['k'][1] == 1.0 for r in rr):
        return ['1.0 mm all the way (B.Cu)']
    if n == '10':
        return ['1.0 mm inside the finger, then 0.6 mm',
                'for %.1f mm (B.Cu) to a via in the no-fill' % rB['len'],
                'strip, then 0.5 mm for %.1f mm (F.Cu)' % lenF,
                'before it reaches the front pour']
    return ['1.0 mm inside the finger, then 0.5 mm',
            '(%.2f mm to a +5V via); reaches the' % g_v[0],
            'back pour %.1f mm further on' % len31]
def desc_old(n):
    ws = sorted({r['k'][1] for r in R_old[n][:3]})
    lay = ' + '.join(sorted({r['k'][0] for r in R_old[n]}))
    return ['%s mm to the pour (%s)' % ('/'.join('%.1f' % w for w in ws), lay)]
for n in ('1', '10', '31'):
    a_ = desc(n); b_ = desc_old(n)
    dr.text((cols[0], ty), 'B%s' % n, font=fb, fill=GD)
    for i, ln in enumerate(a_):
        dr.text((cols[1], ty + i * 28), ln, font=f, fill=INK)
    for i, ln in enumerate(b_):
        dr.text((cols[2], ty + i * 28), ln, font=f, fill=INK)
    ty += 28 * max(len(a_), len(b_)) + 16
    dr.line([xs, ty - 8, W - pad, ty - 8], fill=FRAME, width=1)

# legend
ly = y2 + h2 + 30
x = pad; fs = font(21)
x = swatch(dr, x, ly, 'GND path finger → pour', fill=NET_COL['GND'], kind='line', fs=fs)
x = swatch(dr, x, ly, 'path on the other layer (8dbb6c6)', fill=(150, 205, 170), outline=NET_COL['GND'], fs=fs)
x = swatch(dr, x, ly, 'GND pour', fill=POUR, fs=fs)
x = swatch(dr, x, ly, 'no-fill strip', fill=(246, 246, 246), outline=(150, 150, 150), fs=fs, hatch=(190, 190, 190))
x = swatch(dr, x, ly, 'pad / finger', fill=PAD, fs=fs)
ly2 = ly + 36; x = pad
for net in ('+12V', '+5V', '+3V3'):
    x = swatch(dr, x, ly2, net, fill=NET_COL[net], kind='line', fs=fs) - 10
x = swatch(dr, x, ly2, 'other nets', fill=SIG, kind='line', fs=fs)
dr.rounded_rectangle([x, ly2 - 1, x + 56, ly2 + 23], radius=4, fill=(255, 255, 255), outline=RED, width=1)
dr.text((x + 28, ly2 + 11), '0.21', font=font(16, 'Bold'), fill=RED, anchor='mm')
dr.text((x + 66, ly2 + 11), 'clearance in mm (rule 0.20 mm)', font=fs, fill=INK, anchor='lm')

vy = ly2 + 54; fz = font(21)
lines = (
    'Why accepted: B10 sits between the +12V finger B9 and a strip crossed by IRQ2 and the +5V run (solder side) and by D2-D4 (component side);',
    'a 1.0 mm track there breaks the 0.20 mm clearance on both layers. B10 is one of three GND fingers: B1 has 1.0 mm all the way and B31 reaches',
    'its pour after %.1f mm. 8dbb6c6 had 1.0 mm from all three (the README lists it under "worse").' % len31,
)
for ln in lines:
    dr.text((pad, vy), ln, font=fz, fill=SUB); vy += 30
can = can.crop((0, 0, W, vy + 22))
can.save(OUT)
print(can.size)
