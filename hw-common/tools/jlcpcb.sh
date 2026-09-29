#!/usr/bin/env bash
# JLCPCB fabrication (and assembly) files for a KiCad board, with kicad-cli (KiCad 10).
#
#   hw-common/tools/jlcpcb.sh hw-tht/PicoGUS-THT.kicad_pcb [OUT_DIR]
#
# Output (default OUT_DIR: <board dir>/build/jlcpcb):
#   <name>-gerbers.zip     upload this to jlcpcb.com (gerbers + Excellon drill files)
#   <name>-drc.rpt         DRC report (errors only, incl. schematic parity)
#   <name>-bom-jlc.csv     JLCPCB assembly BOM  (Comment, Designator, Footprint, LCSC Part #, MPN)
#   <name>-cpl-jlc.csv     JLCPCB placement list (Designator, Mid X, Mid Y, Layer, Rotation), SMD parts only
#
# The script stops when DRC finds errors; FORCE=1 exports anyway.
set -euo pipefail

pcb=$(realpath "$1")
name=$(basename "$pcb" .kicad_pcb)
dir=$(dirname "$pcb")
sch="$dir/$name.kicad_sch"
out=$(realpath -m "${2:-$dir/build/jlcpcb}")
rm -rf "$out"
mkdir -p "$out/gerbers"

echo "== DRC ($name)"
if ! kicad-cli pcb drc --severity-error --schematic-parity --exit-code-violations \
        -o "$out/$name-drc.rpt" "$pcb" >/dev/null; then
    grep -E '^\*\* Found' "$out/$name-drc.rpt" || true
    if [ "${FORCE:-0}" != 1 ]; then
        echo "DRC errors: see $out/$name-drc.rpt (FORCE=1 to export anyway)" >&2
        exit 1
    fi
fi
grep -E '^\*\* Found' "$out/$name-drc.rpt" || true

echo "== Gerbers"
# JLCPCB: Protel extensions (default), no X2 attributes, silkscreen clipped by the mask openings
kicad-cli pcb export gerbers -o "$out/gerbers" \
    --layers F.Cu,B.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts \
    --exclude-value --subtract-soldermask --no-x2 --no-netlist --check-zones "$pcb" >/dev/null

echo "== Drill"
kicad-cli pcb export drill -o "$out/gerbers/" --format excellon --excellon-units mm \
    --excellon-zeros-format decimal --excellon-oval-format alternate --excellon-separate-th \
    --drill-origin absolute "$pcb" >/dev/null

python3 - "$out/gerbers" "$out/$name-gerbers.zip" <<'EOF'
import os, sys, zipfile
src, dst = sys.argv[1:]
with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as z:
    for f in sorted(os.listdir(src)):
        z.write(os.path.join(src, f), f)
print('   ', os.path.basename(dst), '-', len(os.listdir(src)), 'files')
EOF

if [ -f "$sch" ]; then
    echo "== Assembly BOM + CPL"
    kicad-cli sch export bom -o "$out/$name-bom-jlc.csv" \
        --fields 'Value,Reference,Footprint,LCSC,MPN,${QUANTITY}' \
        --labels 'Comment,Designator,Footprint,LCSC Part #,MPN,Qty' \
        --group-by 'Value,Footprint,LCSC,MPN' --ref-range-delimiter '' --exclude-dnp "$sch" >/dev/null
    kicad-cli pcb export pos -o "$out/$name-pos.csv" --format csv --units mm --side both \
        --smd-only --exclude-dnp "$pcb" >/dev/null
    python3 - "$out/$name-pos.csv" "$out/$name-cpl-jlc.csv" <<'EOF'
import csv, sys
src, dst = sys.argv[1:]
rows = list(csv.DictReader(open(src)))
with open(dst, 'w', newline='') as f:
    w = csv.writer(f)
    w.writerow(['Designator', 'Mid X', 'Mid Y', 'Layer', 'Rotation'])
    for r in rows:
        w.writerow([r['Ref'], r['PosX'] + 'mm', r['PosY'] + 'mm',
                    'Top' if r['Side'] == 'top' else 'Bottom', r['Rot']])
print('   ', len(rows), 'SMD placements')
EOF
    rm -f "$out/$name-pos.csv"
fi
rm -rf "$out/gerbers"
echo "== Done: $out"
ls -1 "$out"
