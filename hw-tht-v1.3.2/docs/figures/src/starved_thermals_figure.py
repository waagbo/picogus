# Usage: kipython starved_thermals_figure.py DUMP.json DRC.json OUT.png
#   DUMP.json  from 'kipython misc_dump.py PicoGUS-THT.kicad_pcb DUMP.json --refill'
#   DRC.json   from 'kicad-cli pcb drc --severity-all --refill-zones --schematic-parity --format json -o DRC.json <board>'
#              (run it on a copy of the board directory: kicad-cli rewrites the .kicad_pro next to the board)
# Needs misc_figlib.py next to this script.
#
# Draws starved-thermals.png: every starved_thermal warning of the DRC report, grouped by pad, as a front (F.Cu) and a
# back (B.Cu) close-up with the GND pour, the thermal spokes the zone filler made (green), a red cross on each side
# where a spoke has no room with the copper in the way and its distance from the pad, and hatching on copper that is a
# dead end (it reaches GND only through the pad itself). The column captions say how each pad is joined to GND.
# Everything printed is measured here: spokes = poured copper crossing the thermal gap around the pad; "joins GND" =
# connected to the main GND pour without passing through the pad (two-layer connectivity of pour, GND tracks, GND
# vias and plated GND holes).
import sys, os, json, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from misc_figlib import *

d = load(sys.argv[1]); drc = json.load(open(sys.argv[2])); OUT = sys.argv[3]

# ---------------------------------------------------------------- DRC list -> sites
warn = []
for v in drc['violations']:
    if v['type'] != 'starved_thermal':
        continue
    layer = 'F' if 'layer F.Cu' in v['description'] else 'B'
    for it in v['items']:
        desc = it['description']
        if ' of ' in desc and desc.split()[0] in ('PTH', 'Pad'):
            ref = desc.split(' of ')[1].split()[0]
            num = desc.split('pad ' if 'pad ' in desc else 'Pad ')[1].split()[0]
            kind = 'castellation pad' if desc.startswith('Pad ') else 'THT pad'
            warn.append({'ref': ref, 'num': num, 'layer': layer, 'kind': kind, 'island': 'isolated island' in v['description'],
                         'msg': v['description']})
print('%d starved_thermal warnings' % len(warn))
for w in warn:
    print('  %s.%s %s.Cu %s: %s' % (w['ref'], w['num'], w['layer'], w['kind'], w['msg']))
order = []
for w in warn:
    if (w['ref'], w['num']) not in order:
        order.append((w['ref'], w['num']))
order.sort(key=lambda k: (k[0] != 'U1', -sum(1 for w in warn if (w['ref'], w['num']) == k), int(k[1])))

zone = [z for z in d['zones'] if not z['rule'] and z['net'] == 'GND'][0]
GAP, SPW, ZCL, MINW = zone['thermal_gap'], zone['spoke'], zone['clearance'], zone['minw']
ROOM = GAP + MINW + ZCL
fills = {L: [rings_to_poly(p) for z in d['zones'] if not z['rule'] and z['layer'] == L for p in z['fill']] for L in 'FB'}
pour = {L: unary_union([g for g in fills[L] if g is not None]) for L in 'FB'}

cu = {'F': [], 'B': []}
for t in d['tracks']:
    cu[t['layer']].append({'net': t['net'], 'kind': 'track', 'w': t['w'], 'g': track_geom(t), 'pl': t['pl']})
for v in d['vias']:
    for L in 'FB':
        cu[L].append({'net': v['net'], 'kind': 'via', 'g': Point(v['pos']).buffer(v['d'] / 2, quad_segs=24), 'pos': v['pos'], 'drill': v['drill']})
for r, f in d['fps'].items():
    for p in f['pads']:
        for L, pl in p['cu'].items():
            cu[L].append({'net': p['net'], 'kind': 'pad', 'ref': r, 'num': p['num'], 'g': polys_to_geom(pl), 'pos': p['pos'],
                          'hole': p['hole'] is not None})


# ---------------------------------------------------------------- GND connectivity without the pad under test
def connectivity(excl):
    """per layer: components of GND copper (pour, GND tracks, GND vias, GND pads except excl); union-find across layers
    through vias and plated holes. Returns comps, find(), and the root of the main GND (largest front component)."""
    comps = {}
    for L in 'FB':
        items = [g for g in fills[L]] + [c['g'] for c in cu[L] if c['net'] == 'GND' and not (c['kind'] == 'pad' and (c['ref'], c['num']) == excl)]
        U = unary_union([g.buffer(0.001) for g in items])
        comps[L] = sorted(getattr(U, 'geoms', [U]), key=lambda g: -g.area)
    par = {}

    def find(x):
        par.setdefault(x, x)
        while par[x] != x:
            x = par[x]
        return x

    def idx(L, g):
        for i, c in enumerate(comps[L]):
            if c.intersects(g):
                return i
    for c in cu['F']:
        if c['net'] != 'GND':
            continue
        if c['kind'] == 'via' or (c['kind'] == 'pad' and c['hole'] and (c['ref'], c['num']) != excl):
            a = idx('F', c['g']); b = idx('B', Point(c['pos']).buffer(0.05))
            if a is not None and b is not None:
                par[find(('F', a))] = find(('B', b))
    return comps, find, idx


def name_of(c):
    if c['kind'] == 'pad':
        if c['ref'] == 'U1':
            return 'next pin'
        return '%s %s' % (c['ref'], 'shield pad' if c['num'] == 'SH' else 'pad ' + c['num'])
    n = c['net'].replace('{slash}', '/')
    if n.startswith('~{'):
        n = '~' + n[2:]
    return n.replace('}', '')


def analyse(ref, num, L, conn):
    comps, find, idx = conn
    main = find(('F', 0))
    pads = [p for p in d['fps'][ref]['pads'] if p['num'] == num and L in p['cu']]
    shp = unary_union([polys_to_geom(p['cu'][L]) for p in pads])
    cx, cy = pads[0]['pos']
    ring = shp.buffer(GAP - 0.02).difference(shp.buffer(0.001))
    spokes = []
    for f in fills[L]:
        x = f.intersection(ring)
        for part in getattr(x, 'geoms', [x]):
            if part.area > 0.05:
                i = idx(L, f)
                spokes.append({'g': f.intersection(shp.buffer(GAP - 0.01)).difference(shp), 'piece': f, 'area': f.area,
                               'comp': comps[L][i], 'joins': find((L, i)) == main})
    gtr = []
    for c in cu[L]:
        if c['net'] == 'GND' and c['kind'] == 'track' and c['g'].intersects(shp.buffer(0.01)):
            i = idx(L, c['g'].difference(shp.buffer(0.05)) if not c['g'].difference(shp.buffer(0.05)).is_empty else c['g'])
            gtr.append({'w': c['w'], 'comp': comps[L][i], 'joins': find((L, i)) == main, 'c': c})
    off = pads[0]['spoke_angle'] % 90
    dirs = []
    for k in range(4):
        a = math.radians(off + 90 * k)
        ux, uy = math.cos(a), math.sin(a)
        strip = LineString([(cx, cy), (cx + 6 * ux, cy + 6 * uy)]).buffer(SPW / 2, cap_style=2).difference(shp)
        has = [s for s in spokes if s['g'].intersects(strip)]
        obs = []
        for c in cu[L]:
            if c['kind'] == 'pad' and c['ref'] == ref and c['num'] == num:
                continue
            if c['net'] == 'GND' and c['kind'] != 'pad':
                continue
            if c['g'].intersects(strip):
                hit = c['g'].intersection(strip)
                obs.append((c['g'].distance(shp), hit.distance(shp), c, hit))
        obs.sort(key=lambda o: o[1])
        ray = LineString([(cx, cy), (cx + 8 * ux, cy + 8 * uy)])
        e = ray.intersection(shp.boundary)
        pts = [e] if e.geom_type == 'Point' else list(getattr(e, 'geoms', []))
        ep = max(pts, key=lambda p: math.hypot(p.x - cx, p.y - cy))
        dirs.append({'ang': round(off + 90 * k) % 360, 'u': (ux, uy), 'spokes': has, 'obs': obs[:1], 'edge': (ep.x, ep.y)})
    return {'shp': shp, 'pads': pads, 'spokes': spokes, 'dirs': dirs, 'gtr': gtr, 'c': (cx, cy)}


A = {}
for ref, num in order:
    conn = connectivity((ref, num))
    for L in 'FB':
        a = A[(ref, num, L)] = analyse(ref, num, L, conn)
        print('%s.%s %s.Cu: %d spoke(s) %s; GND tracks %s' % (ref, num, L, len(a['spokes']),
              ['into %.2f mm2 of pour, %s' % (s['area'], 'joins GND' if s['joins'] else 'DEAD END') for s in a['spokes']],
              ['%.2f mm %s' % (t['w'], 'joins GND' if t['joins'] else 'DEAD END') for t in a['gtr']]))
        for dd in a['dirs']:
            print('   %3d deg: %s; nearest in line: %s' % (dd['ang'], 'spoke' if dd['spokes'] else 'no spoke',
                  ['%s %.2f mm (in line %.2f)' % (name_of(o[2]), o[0], o[1]) for o in dd['obs']]))

# ---------------------------------------------------------------- panels
S = 76
HALF = 3.3
VIEW = {('U1', '8'): (199.23, 99.25), ('U1', '3'): (211.93, 99.25), ('U1', '28'): (186.53, 118.55), ('C5', '2'): (159.69, 103.34)}
# blocker label placement: (ref, num, layer, angle) -> (dx, dy) px from the blocker point, anchor
LAB = {
    ('U1', '8', 'F', 270): (0, -16, 'mb'), ('U1', '8', 'F', 90): (0, 18, 'mt'),
    ('U1', '8', 'B', 90): (0, 16, 'mt'), ('U1', '8', 'B', 180): (-10, 44, 'mt'),
    ('U1', '3', 'F', 270): (60, 4, 'lm'), ('U1', '3', 'F', 90): (0, 18, 'mt'),
    ('U1', '3', 'B', 180): (-10, 44, 'mt'),
    ('U1', '28', 'F', 270): (0, -16, 'mb'), ('U1', '28', 'B', 180): (-10, -44, 'mb'),
    ('C5', '2', 'F', 135): (-6, 6, 'rt'), ('C5', '2', 'F', 225): (-6, -6, 'rb'), ('C5', '2', 'F', 315): (6, -6, 'lb'),
    ('C5', '2', 'F', 45): (14, 10, 'lt'),
    ('C5', '2', 'B', 45): (8, 4, 'lt'), ('C5', '2', 'B', 135): (-8, 4, 'rt'),
}
# 'dead end' callouts: (ref, num, layer) -> list of (x, y) mm label point, anchor, target point in mm
DEADLAB = {('U1', '8', 'F'): [((198.88, 101.13), 'rm', None)],
           ('U1', '3', 'F'): [((211.58, 101.13), 'rm', None)],
           ('U1', '28', 'B'): [((188.6, 120.2), 'mm', None), ((184.0, 116.2), 'mm', None)]}
# callouts on spokes / tracks: (ref, num, layer) -> list of (text-key, label position in mm, anchor)
DEADCOL = (70, 70, 70)
fls = font(18, 'SemiBold')

U1body = None
xs = [p[0] for g in d['fps']['U1']['fab'] if g['side'] == 'F' for pl in g['pl'] for p in pl]
ys = [p[1] for g in d['fps']['U1']['fab'] if g['side'] == 'F' for pl in g['pl'] for p in pl]
if xs:
    U1body = box(min(xs), min(ys), max(xs), max(ys))


def panel(ref, num, L):
    a = A[(ref, num, L)]
    cx, cy = VIEW[(ref, num)]
    v = View(cx - HALF, cy - HALF, cx + HALF, cy + HALF, S)
    v.grid(0.5)
    v.fill(pour[L], FPOUR if L == 'F' else BPOUR)
    win = v.clip
    for c in cu[L]:
        if c['kind'] == 'track' and c['g'].intersects(win):
            col = (GNDTRK_F if L == 'F' else GNDTRK_B) if c['net'] == 'GND' else (FTRK if L == 'F' else BTRK)
            v.fill(c['g'], col, 235)
    dead = [s['comp'] for s in a['spokes'] if not s['joins']] + [t['comp'] for t in a['gtr'] if not t['joins']]
    for g in dead:
        v.hatch(g.difference(a['shp']), DEADCOL, 8, 1.4)
    for c in cu[L]:
        if c['kind'] == 'pad' and c['g'].intersects(win):
            v.fill(c['g'], PAD)
    for c in cu[L]:
        if c['kind'] == 'via' and c['g'].intersects(win):
            v.fill(c['g'], (125, 125, 125) if c['net'] != 'GND' else ((120, 60, 55) if L == 'F' else (40, 65, 115)))
            v.fill(Point(c['pos']).buffer(c['drill'] / 2), HOLE)
    for f in d['fps'].values():
        for p in f['pads']:
            h = hole_geom(p)
            if h is not None and h.intersects(win):
                v.fill(h, HOLE)
    for s in a['spokes']:
        v.fill(s['g'], SPOKE)
    if ref == 'U1' and U1body is not None:
        v.outline(U1body.exterior, (90, 90, 90), 1.6, dash=(7, 5))
    im = v.finish()
    dr = ImageDraw.Draw(im)
    CL = (0, 0, im.size[0], im.size[1])
    done = set()
    for xy, an, tgt in DEADLAB.get((ref, num, L), []):
        q = v.T(xy)
        if tgt:
            arrow(dr, q, v.T(tgt), DEADCOL, 2, 9)
        tag(dr, q, 'dead end', fls, DEADCOL, anchor=an, clamp=CL)
    for dd in a['dirs']:
        ex, ey = dd['edge']; ux, uy = dd['u']
        dead_spoke = dd['spokes'] and not any(s['joins'] for s in dd['spokes'])
        if not dd['spokes']:
            px, py = v.T((ex + ux * GAP / 2, ey + uy * GAP / 2))
            r = 9
            dr.line([px - r, py - r, px + r, py + r], fill=RED, width=4)
            dr.line([px - r, py + r, px + r, py - r], fill=RED, width=4)
        if dd['obs'] and (not dd['spokes'] or dead_spoke):
            dist, dline, c, hit = dd['obs'][0]
            nm = name_of(c)
            if nm == 'next pin' or dline > (1.0 if dd['spokes'] else 1.6) or (nm, round(dist, 2)) in done:
                continue
            done.add((nm, round(dist, 2)))
            lp = LAB.get((ref, num, L, dd['ang']))
            if lp is None:
                print('   (no label position for %s.%s %s %d: %s %.2f)' % (ref, num, L, dd['ang'], nm, dist))
                continue
            q = v.T((ex + ux * (dline + 0.12), ey + uy * (dline + 0.12)))
            tag(dr, (q[0] + lp[0], q[1] + lp[1]), '%s %.2f mm' % (nm, dist), fls, RED, anchor=lp[2], clamp=CL)
    return im, v


tiles = {k: panel(*k) for k in A}

# ---------------------------------------------------------------- captions (numbers from the analysis)
def fmt_mm2(x):
    return '%.2f mm²' % x


def caption(ref, num):
    out = []
    for L in 'FB':
        a = A[(ref, num, L)]
        n = len(a['spokes'])
        good = [x for x in a['spokes'] if x['joins']]
        bad = [x for x in a['spokes'] if not x['joins']]
        tj = [t for t in a['gtr'] if t['joins']]
        tb = [t for t in a['gtr'] if not t['joins']]
        sp = '%d spoke%s' % (n, '' if n == 1 else 's')
        if good or tj:
            txt = sp + (' into the pour' if n else '') + (' + a %.1f mm GND track' % tj[0]['w'] if tj else '') + '. Joined here.'
        elif n:
            areas = ' + '.join('%.2f' % x['area'] if x['area'] < 2 else '%.1f' % x['area'] for x in bad) + ' mm²'
            if tb:
                txt = '%s and a %.1f mm GND track, all into pour that joins nothing else (%s).' % (sp, tb[0]['w'], areas)
            elif n == 1 and bad[0]['area'] < 0.6:
                txt = '1 spoke, a %s stub that joins nothing.' % areas
            else:
                txt = '%s into a %s piece of pour that joins nothing else.' % (sp, areas)
        else:
            txt = 'no spoke.'
        out.append((L, bool(good or tj), txt))
    return out


# blocking tracks beside the Pico pads (sides without a spoke that joins the pour)
blk = [o[0] for (ref, num, L), a in A.items() if ref == 'U1' for dd in a['dirs'] for o in dd['obs']
       if o[2]['kind'] == 'track' and o[1] <= 1.6 and not any(s['joins'] for s in dd['spokes'])]
viaF = [k for k in order if any(s['joins'] for s in A[k + ('F',)]['spokes']) or any(t['joins'] for t in A[k + ('F',)]['gtr'])]
viaB = [k for k in order if any(s['joins'] for s in A[k + ('B',)]['spokes']) or any(t['joins'] for t in A[k + ('B',)]['gtr'])]
def nms(ks):
    u = [k[1] for k in ks if k[0] == 'U1']; o = ['%s.%s' % k for k in ks if k[0] != 'U1']
    t = (['Pico pin%s %s' % ('s' if len(u) > 1 else '', ' and '.join(u))] if u else []) + o
    return ' and '.join(t) if len(t) < 3 else ', '.join(t[:-1]) + ' and ' + t[-1]
print('joined on F.Cu:', viaF, ' on B.Cu:', viaB, ' castellation-side tracks %.2f-%.2f mm' % (min(blk), max(blk)))
WHY = ['Why accepted: each pad is a plated through-hole, so it is joined to GND by the other layer where one side has no room: '
       + nms([k for k in viaB if k not in viaF]) + ' on B.Cu, ' + nms([k for k in viaF if k not in viaB]) + ' on F.Cu.',
       'These are low-current GND returns (Pico GND pins, the GND side of U4\'s %s decoupling cap), and the Pico\'s GND pins are also tied'
       % d['fps']['C5']['value'].replace('uF', ' µF'),
       'together inside the module. A second spoke would need the tracks beside the Pico pads moved: they pass %.2f–%.2f mm from the pads.'
       % (min(blk), max(blk))]

# ---------------------------------------------------------------- compose
pad = 40; gap = 34
TW, TH = tiles[(order[0][0], order[0][1], 'F')][0].size
ncol = len(order)
W = pad + ncol * TW + (ncol - 1) * gap + pad
head = 200
colhead = 78
H = head + colhead + 2 * TH + 46 + 600
can = Image.new('RGB', (W, H), WHITE); dr = ImageDraw.Draw(can)
ft = font(40, 'Bold'); fs = font(24); fh = font(26, 'SemiBold'); fsm = font(21); fz = font(20); fzb = font(20, 'SemiBold')
nextpin = min(o[0] for a in A.values() for dd in a['dirs'] for o in dd['obs'] if name_of(o[2]) == 'next pin')
dr.text((pad, 28), 'PicoGUS THT v1.3.2: the %d starved thermals' % len(warn), font=ft, fill=INK)
for k, ln in enumerate((
        'DRC wants at least two thermal spokes (green) from a GND pad into the pour; these pads get one. A red cross marks each side',
        'where a spoke has no room, labelled with the copper in the way and its distance from the pad. Pour beside a pad needs %.2f mm'
        % ROOM,
        '(%.1f mm thermal gap + %.2f mm minimum pour + %.1f mm clearance); between two Pico pins there is %.2f mm.' % (GAP, MINW, ZCL, nextpin))):
    dr.text((pad, 84 + 32 * k), ln, font=fs, fill=GREY)

TITLES = {('U1', '8'): 'Pico U1 pin 8 (GND)', ('U1', '3'): 'Pico U1 pin 3 (GND)', ('U1', '28'): 'Pico U1 pin 28 (GND)',
          ('C5', '2'): 'C5 pad 2 (GND, U4 decoupling)'}
y0 = head + colhead
for i, (ref, num) in enumerate(order):
    x = pad + i * (TW + gap)
    ws = [w for w in warn if (w['ref'], w['num']) == (ref, num)]
    dr.text((x, head + 6), TITLES.get((ref, num), '%s pad %s' % (ref, num)), font=fh, fill=INK, anchor='lt')
    kinds = []
    for L in 'FB':
        k = sorted({w['kind'] for w in ws if w['layer'] == L}, reverse=True)
        if k:
            kinds.append('%s.Cu %s' % (L, ' + '.join(x.replace(' pad', '') for x in k) + (' pad' if len(k) == 1 else ' pads')))
    tag(dr, (x + 5, head + 56), '%d warning%s: %s' % (len(ws), '' if len(ws) == 1 else 's', ', '.join(kinds)), font(17, 'Bold'), RED, anchor='lm')
    for j, L in enumerate('FB'):
        im, v = tiles[(ref, num, L)]
        yy = y0 + j * (TH + 46)
        can.paste(im, (x, yy)); frame(dr, x, yy, im)
        lab = 'F.Cu (front)' if L == 'F' else 'B.Cu (back)'
        tb = dr.textbbox((x + 10, yy + 10), lab, font=fls)
        dr.rectangle([tb[0] - 5, tb[1] - 4, tb[2] + 5, tb[3] + 4], fill=WHITE, outline=(170, 170, 170))
        dr.text((x + 10, yy + 10), lab, font=fls, fill=INK)

# column captions
yc = y0 + 2 * TH + 46 + 18
ymax = yc


def wrap(text, f, width):
    out = []; cur = ''
    for w in text.split(' '):
        t = (cur + ' ' + w).strip()
        if dr.textlength(t, font=f) <= width:
            cur = t
        else:
            out.append(cur); cur = w
    out.append(cur)
    return out


for i, (ref, num) in enumerate(order):
    x = pad + i * (TW + gap)
    yy = yc
    for L, joined, txt in caption(ref, num):
        head_ = '%s.Cu: ' % L
        body = txt
        lines = wrap(head_ + body, fz, TW - 4)
        for k, ln in enumerate(lines):
            if k == 0:
                dr.text((x, yy), head_, font=fzb, fill=INK)
                dr.text((x + dr.textlength(head_, font=fzb), yy), ln[len(head_):], font=fz, fill=TXT2 if not joined else (10, 110, 50))
            else:
                dr.text((x, yy), ln, font=fz, fill=TXT2 if not joined else (10, 110, 50))
            yy += 27
        yy += 6
    for ln in EXTRA.get((ref, num), []) if 'EXTRA' in globals() else []:
        for l2 in wrap(ln, fz, TW - 4):
            dr.text((x, yy), l2, font=fz, fill=TXT2); yy += 27
    ymax = max(ymax, yy)

# legend + why
ly = ymax + 26
xx = pad
xx = swatch(dr, xx, ly, FPOUR, 'GND pour, front', fsm) + 26
xx = swatch(dr, xx, ly, BPOUR, 'GND pour, back', fsm) + 26
xx = swatch(dr, xx, ly, SPOKE, 'thermal spoke', fsm) + 26
xx = swatch(dr, xx, ly, PAD, 'pad (white: hole)', fsm) + 26
xx = swatch(dr, xx, ly, FTRK, 'other-net track', fsm) + 26
xx = swatch(dr, xx, ly, GNDTRK_F, 'GND track', fsm) + 26
# hatch swatch
for k in range(-22, 34, 8):
    dr.line([xx + max(k, 0), ly + 22 - max(0, -k), xx + min(k + 22, 34), ly + max(0, k + 22 - 34)], fill=DEADCOL, width=2)
dr.rectangle([xx, ly, xx + 34, ly + 22], outline=DEADCOL, width=1)
dr.text((xx + 44, ly + 11), 'dead end (reaches GND only through this pad)', font=fsm, fill=INK, anchor='lm')
ly2 = ly + 38
for k in range(0, 34, 11):
    dr.line([pad + k, ly2 + 11, pad + min(k + 7, 34), ly2 + 11], fill=(90, 90, 90), width=2)
dr.text((pad + 44, ly2 + 11), 'Pico module outline', font=fsm, fill=INK, anchor='lm')
xr = pad + 44 + dr.textlength('Pico module outline', font=fsm) + 26
dr.line([xr + 4, ly2 + 2, xr + 22, ly2 + 20], fill=RED, width=4); dr.line([xr + 4, ly2 + 20, xr + 22, ly2 + 2], fill=RED, width=4)
dr.text((xr + 44, ly2 + 11), 'no room for a spoke on this side', font=fsm, fill=INK, anchor='lm')
vy = ly2 + 52
for ln in WHY:
    dr.text((pad, vy), ln, font=fsm, fill=TXT2); vy += 30
can = can.crop((0, 0, W, vy + 22))
can.save(OUT)
print('saved', OUT, can.size)
