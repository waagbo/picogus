# PicoGUS THT (through-hole) v1.3.2 (unrouted blank, work in progress) – based on PicoGUS v1.2

Board revision **v1.3.2**: the [v1.3.1 (rev B) THT board](../hw-tht-v1.3.1/README.md) made buildable without
fine-pitch soldering on the card itself.

* **U3/U4** (SN74CB3T3257) and **U9** (PCM5100A) only exist as TSSOPs. On v1.3.2 each one sits on a **300-mil
  TSSOP-to-DIP adapter board** that goes into a plain DIP-16/DIP-20 footprint, soldered or socketed. The
  adapters can be bought (Chip Quik PA0034C/PA0035C) or made from the two adapter PCBs in this folder
  (see [Adapter boards](#adapter-boards)).
* **The GY-PCM5102 module option (J10) is gone.** The on-board DAC is always fitted.
* **C36** (U9 charge-pump decoupling) is now a through-hole 0.1 µF disc right beside the DIP-20's pins 1–3 (it was
  an 0603 under the TSSOP).
* **C8/C10** (U8 PSRAM decoupling) are now **0805s on the back**, right under U8.

The main board is through-hole apart from **U8** (APS6404L PSRAM, SOIC-8, 1.27 mm pitch) with its two 0805 caps
C8/C10 on the back, and the optional **U12** (USBLC6-2SC6, SOT-23-6). The TSSOPs are soldered to the adapters, not
to the card. The rest of the circuit is v1.3.1's: USB-A host port, Waveblaster header, op-amp mixer.

> **Status: unrouted blank (placement only), not built or tested.** The board has no tracks or vias; the GND
> pour and the rule areas are kept, the pour is unfilled. The schematic's ERC is the same as v1.3.1's (280
> warnings, 0 errors). DRC with schematic parity finds no errors apart from the 309 unconnected items of an
> unrouted board: no missing or extra footprints, no net mismatches, no courtyard overlaps. **Routing, ground
> returns and gerbers are not done or checked yet**, and the back artwork is only re-cut around the new pads; it
> will be re-placed after the layout. Do not order this board. See [Before routing](#before-routing).

Files:

| File | What |
|---|---|
| `PicoGUS-THT.kicad_pro/.kicad_sch/.kicad_pcb` | KiCad **10** project (the root sheet is the v1.2 logic). The board is an **unrouted blank** |
| `audio.kicad_sch` | PCM510x DAC sheet: U9 on its DIP-20 adapter, C36 a THT disc, no J10 |
| `wavetable.kicad_sch` | Waveblaster header, WT volume, op-amp mixer, line-out jack (as v1.3.1) |
| `usb.kicad_sch` | USB host port (J11 USB-A, F1, C34/C35, U12) (as v1.3.1) |
| `PicoGUS-THT-schematic.pdf` | Schematic PDF |
| `BOM-THT.csv` | BOM with DigiKey **and** LCSC part numbers and a digikey.no link per line (v1.3.0 lines checked 2026-09-28, rev B lines 2026-10-18, v1.3.2 lines 2026-10-23) |
| `BOM-THT-digikey-upload.csv` (+ `-optional`, `-10x`) | DigiKey cart/BOM upload with the reference designators as Customer Reference. The bought adapters (PA0034C ×2, PA0035C) are in the main file; optional: sockets, U12 |
| `BOM-THT-sockets.csv` (+ `-digikey-upload`, `-10x`) | Socket build: every IC (and adapter) socketed, with heights (see [Socket build](#socket-build)) |
| `adapters/TSSOP16-DIP16-300mil/`, `adapters/TSSOP20-DIP20-300mil/` | The two adapter PCBs: KiCad 10 projects with ready JLCPCB files (see [Adapter boards](#adapter-boards)) |
| `adapters/adapter-pins-digikey-upload.csv` (+ `-10x`) | DigiKey upload: the round machined pin strips for the own adapter PCBs (one card: 4 × 1x8, 2 × 1x10) |

The shared `PiGUS library` symbols and footprints are linked from `hw-common/`, like the other boards. **New in the
shared library for v1.3.2:** `DIP-16_W7.62mm_TSSOP-16_Adapter` and `DIP-20_W7.62mm_TSSOP-20_Adapter`. They must be
added to `hw-common/PiGUS library.pretty` (the board file carries its own copies, so it opens without them, but
"Update footprint from library" and the library checks need them). The older `DIP-16_W7.62mm_TSSOP-16_Nested`
stays unused.

## What changed compared to v1.3.1

### U3/U4 and U9 on 300-mil TSSOP-to-DIP adapters

| | U3, U4 (SN74CB3T3257, TSSOP-16) | U9 (PCM5100A, TSSOP-20) |
|---|---|---|
| Footprint | `PiGUS library:DIP-16_W7.62mm_TSSOP-16_Adapter` | `PiGUS library:DIP-20_W7.62mm_TSSOP-20_Adapter` |
| Bought adapter (BOM line) | Chip Quik / Proto-Advantage **PA0034C** (DigiKey 315-PA0034C-ND, 28 in stock 2026-10-23, USD 7.19) | Chip Quik / Proto-Advantage **PA0035C** (DigiKey 315-PA0035C-ND, **1** in stock 2026-10-23, USD 7.79; the Proto-Advantage store lists it in stock) |
| Own adapter PCB | `adapters/TSSOP16-DIP16-300mil` + 2 × 1x8 round machined pin strips | `adapters/TSSOP20-DIP20-300mil` + 2 × 1x10 round machined pin strips |
| Other bought adapter | SparkFun BOB-13994 (1568-13994-ND, 2 boards per pack, no pins, 0.635 mm pad pitch: centre the TSSOP carefully) | none in stock (MIKROE-307 is sold out) |
| Optional socket | On Shore SA163000 (ED3016-ND) | On Shore SA203000 (ED3020-ND) |

* **TSSOP pin *n* = DIP pin *n*** on the own adapter PCBs (proved by their netlist check) and, by Proto-Advantage's
  convention, on the Chip Quik adapters. Their datasheets show no pin-out, so **check a bought adapter with a meter
  before soldering the chip**: the TSSOP pad at the adapter's pin-1 mark must ring through to the DIP pin at the
  marked corner, and pad *n* to pin *n* along both rows. The schematic pin numbers then keep their meaning.
  U9: 1 CPVDD, 2 CAPP, 3 CPGND, 4 CAPM, 5 VNEG, 6 OUTL, 7 OUTR, 8 AVDD, 9 AGND, 10 DEMP, 11 FLT, 12 SCK, 13 BCK, 14 DIN, 15 LRCK, 16 FMT, 17 XSMT, 18 LDOO, 19 DGND,
  20 DVDD. The U3/U4/U9 symbols carry an `Adapter` field naming the Chip Quik part.
* **Footprints**: origin at pin 1 (KiCad's DIP convention; the old TSSOP footprints had theirs at the package
  centre). Pads are 1.70 mm with 1.00 mm holes, pad 1 a rounded square. They take the adapters' round 0.46 mm
  pins, machined-socket tails (0.51 mm) and 0.64 mm square header pins. (The board's other DIPs keep KiCad's
  1.6 mm / 0.8 mm.)
* **Courtyard and silkscreen are the adapter outline.** Courtyard, measured from pin 1: DIP-16 (−1.78, −1.52) to
  (9.40, 19.30), wide enough for a snapped BOB-13994 (10.41 mm); DIP-20 (−1.52, −1.52) to (9.14, 24.38). Both
  cover the PA0034C/PA0035C boards and the SA163000/SA203000 socket bodies (10.16 × 20.32 / 10.16 × 25.40 mm) with
  0.25 mm to spare. The silk has the DIP notch and a filled pin-1 triangle above pin 1, which stays visible with
  the adapter fitted. The reference sits inside the outline, hidden by a fitted adapter like the other DIP
  references. F.Fab shows the PA0034C/PA0035C board (1 mm pin-1 chamfer), which is also the outline of the own
  adapter PCBs. No 3D model.
* Heights are in [Adapter boards](#heights). All of them stay under a Waveblaster daughterboard (~11 mm); the
  socketed own PCBs are the tightest at about 10.0–10.2 mm (0.8–1.0 mm to spare).
* Do **not** buy the wide adapters. The plain PA0034/PA0035 (0.700 in boards), Aries LCQT-TSSOP16/20, Adafruit
  1206/1207 and the common 0.5/0.6 in "SOP/TSSOP to DIP" boards do not fit 300-mil rows.

### No GY-PCM5102 module option

J10 (the GY-PCM5102 module, soldered on male pins over the DAC area as the alternative to the on-board DAC since
v1.3.0) is gone from the schematic and the board. Its dashed silkscreen outline and the "DAC -OR- GY-PCM5102"
heading went with it; the heading now just says "DAC". Its nets (BCK, DIN, LRCK, +5V, GND, AUDIO_L/R) stay,
because U9's circuit uses them. U9, C12–C20, C36, FB3, R7 and R8 are always fitted. The BOM's J10 line and its
"DAC AREA: leave out ..." notes are gone.

### C36: THT disc at U9 pins 1–3

C36 (U9 CPVDD decoupling) is now the board's standard 0.1 µF 50 V X7R radial disc (Murata RDER71H104K0M1H03A,
the 0.1 µF line of the BOM) instead of an 0603: an SMD part under an adapter board makes no sense. It sits
right beside DIP-20 pins 1 (CPVDD) and 3 (CPGND), its pads in line with them 3.2 mm away, its courtyard against
the DIP-20's. It cannot go under the adapter: a soldered adapter sits about 0.45 mm above the board, and a socket
body covers the area. The loop pin 1 → C36 → pin 3 is short by placement; its actual length is set by the routing.

### C8/C10: 0805 on the back under U8

| Ref | Part | DigiKey | LCSC |
|---|---|---|---|
| C8 | 0.1 µF 50 V X7R 0805, Yageo CC0805KRX7R9BB104 | 311-1140-1-ND | C49678 (JLCPCB basic) |
| C10 | 1 µF 50 V X7R 0805, Samsung CL21B105KBFNNNE | 478-KGM21AR71H105KUCT-ND (KYOCERA AVX 08055C105KAT2A: the Samsung part, 1276-1029-1-ND, had no DigiKey stock on 2026-10-23) | C28323 (JLCPCB basic) |

Both sit on the back (B.Cu), under U8 (SOIC-8, front), side by side between U8's +3V3 side (pin 8) and its GND
side (pin 4). Pad 1 (+3V3) of each faces pin 8's row and pad 2 (GND) faces pin 4's row. C8 is next to pin 8
(1.8 mm from it, 3.3 mm from pin 4) and C10 next to pin 4. Their old top-side spots next to U8 are free. The
JLCPCB placement file lists them on the **Bottom** layer. Their references are on the back silkscreen just below
U8, clear of the artwork. (LCSC C49678 is the Yageo part, a JLCPCB basic part; the Samsung CL21B104KBCNNNC is
C1711, an extended part, listed as the C8 alternative.)

### Placement (unrouted)

Everything else is where it was on v1.3.1. Moved: U9, U3, U4, C3, C5, C8, C10, C12–C20, C36, FB3, R7, R8 and the
"DAC" heading; J10 is removed.

* **DAC**: U9's DIP-20 at 180° (pin 1 at (156.40, 84.50), bottom right; the pin-1 triangle and the notch are at
  the bottom, 0.19 mm clear of U2 pad 18). Pins 1–10 (charge pump, outputs, AVDD) face the mixer on the right,
  pins 11–20 (I2S, DVDD, LDOO) face the logic on the left. The courtyard spans x 147.2–158.0, y 60.1–86.1.
  * Right of pins 1–5, in 3.2 mm columns: C36 (CPVDD/CPGND, pins 1/3), C16 (CAPP/CAPM, pins 2/4; the pin-2 track
    can run between C36's pads), C17 (VNEG, pin 5). The adapter pushes these caps further out than on v1.3.1:
    C36 is 3.2 mm from pins 1/3 (1.9 mm before), C16 about 6 mm from pins 2/4 and C17 about 10 mm from pin 5
    (4.9 mm before). Route CAPP/CAPM and VNEG short and direct, or bring C17 into the first column when you
    rearrange (R7/R8 would then move one column right).
  * Output filter: R7/R8 horizontal off pins 6/7, just below the mixer resistor row, then C19/C20 at their ends
    (AUDIO_L/AUDIO_R straight on to R10/R11).
  * AVDD in one row: pin 8 → C13 → C15 → FB3 pad 2. FB3 is vertical with its +3V3 pad at the top (R16's +3V3 pad
    is 9 mm away).
  * Left of the DIP: C12 (DVDD, pin 20), C18 (LDOO, pin 18), C14 (+3V3).
  * "DAC" heading centred above the DIP-20.
* **U3/U4**: two DIP-16s at 0° side by side (U4 left, U3 right), where the TSSOPs were (body centres within 1.2 mm
  of the old TSSOP centres), between U5, R2 and the Pico. C5/C3 are horizontal above each chip's pin 16 (+3V3).
  Pins 8/9 (y 123.10) reach into the no-fill strip above the fingers (y > 123.2).
* **C8/C10**: on the back under U8, see above.

| Part | v1.3.1 | v1.3.2 |
|---|---|---|
| U9 | TSSOP-20 centre (153.02, 66.14) | DIP-20 pin 1 (156.40, 84.50), 180° |
| U3 / U4 | TSSOP-16 centre (162.81, 114.11) / (149.19, 114.28) | DIP-16 pin 1 (157.90, 105.32) / (146.57, 105.32), 0° |
| C3 / C5 | (157.70, 111.25) / (144.40, 111.80) | (165.52, 102.18) / (154.19, 102.18), 180° |
| C36 | 0603 (150.40, 70.80) | disc (159.60, 84.50), 90° |
| C12, C14, C18 | (155.59, 60.28), (155.59, 57.28), (147.84, 61.00) | (145.55, 79.50), (145.55, 64.90), (145.55, 77.20) |
| C13, C15, FB3 | (158.39, 60.28), (158.39, 67.78), (162.00, 56.50) | (159.60, 61.72), (162.80, 66.72), (166.45, 54.02) |
| C16, C17 | (147.34, 71.00), (147.84, 68.50) | (162.80, 81.96), (166.00, 75.83) |
| R7, R8 | (151.34, 73.72), (154.84, 73.32), vertical | (159.13, 73.10), (159.13, 69.90), horizontal |
| C19, C20 | (147.84, 78.50), (158.34, 78.50) | (175.30, 78.10), (172.02, 74.90) |
| C8, C10 | discs (191.30, 94.50), (179.54, 94.03) | 0805 on the back (188.00, 91.50), (185.90, 91.50) |
| J10 | GY-PCM5102 module (154.03, 68.17) | removed |

### Silkscreen and back artwork

* Front `v1.3.2`, back `PicoGUS v1.3.2`.
* **The princess artwork is a placeholder for now.** It keeps v1.3.1's placement and was only re-cut, 0.3 mm
  around every new pad and hole (and around the back silkscreen of C8/C10). Several new pads now land on the
  faces (back view): on the little princess, C20 pad 1 is on her left eye (as seen in the picture), R7 pad 2 on her
  forehead, R8 pad 2 and C20 pad 2 on her hairline and crown, and C17 pad 1 at her other temple. On the big
  princess, the two C19 pads sit on her cheek and jaw line. The DIP-20 pads are on the castle, C36/C16 on the
  little princess's hand and hair, C12/C14/C18 on the castle's right side, and C8/C10 in the "c" of "Pico".
  **The artwork will be re-placed once the layout is routed and final.**
* Silkscreen DRC (with silk_over_copper/silk_overlap enabled) on the blank: 0 silk_over_copper and 2 silk_overlap
  (the IRQ/DMA divider line over J1, as on v1.3.1). J10's dashed outline and the TSSOP U9's pin-1 mark caused the
  others on v1.3.1. 8 silk_edge_clearance (the bracket parts), unchanged.

### Other

* Title blocks say v1.3.2 (2026-10-23). The schematic notes about the GY module and the SMD parts are updated.
* No reference designator changed and none is new; J10 is the only one removed. The design rules and net classes
  are v1.3.1's (the `.kicad_pro` is unchanged).
* 32 footprints (the wavetable sheet set, FB3, RV1, U11) still carried symbol links (KIID paths) from before the
  v1.3.0 schematic rebuild; DRC parity had only matched them by reference. They are re-linked to their symbols, so
  "Update PCB from schematic" keeps them.

## Before routing

* **Order the adapters first**: PA0035C stock is thin (1 at DigiKey on 2026-10-23). The own adapter PCBs are the
  fallback (see [Adapter boards](#adapter-boards)).
* U3/U4 pin 8 (GND) at y 123.10 is inside the no-fill strip above the fingers: give each a short track to the pour.
* C8/C10: one via beside the inner end of U8 pin 8 (+3V3) and of pin 4 (GND), and two short back rails (+3V3 along
  y 92.45, GND along y 90.55). On v1.3.1, SPI_TX and SPI_SCK ran on F.Cu under U8's body; keep that side free of
  the new vias.
* U9's I2S pins (13–15) are in its left column, so the I2S tracks from the Pico pass U2, as on v1.3.1. The
  channel between those pins and the C12/C18/C14 column is about 1.6 mm wide and the C12 → pin 20 and C18 → pin 18
  links cross it, so expect back-layer hops for BCK/DIN/LRCK there.
* AUDIO_L and AUDIO_R cross once on the way from R7/R8/C19/C20 to R10/R11 (one via): keep the crossing at right
  angles and away from the I2S.
* C36: route pin 1 → C36 → pin 3 as short as possible, with a CPGND via into the back pour (as on v1.3.1).
* Repeat the v1.3.1 routing rules in [Layout notes from v1.3.1](#layout-notes-from-v131).
* After routing: re-place the back artwork so that no pads land on the faces, check that no via hole lands in a
  letter of the back version block, run `hw-common/tools/jlcpcb.sh` (0 errors, 0 unconnected), and check the
  bottom-side rotations of C8/C10 in JLCPCB's placement preview if JLCPCB places them.

## Adapter boards

### Where

| Adapter | Project (KiCad 10, self-contained) | For | Size |
|---|---|---|---|
| TSSOP-16 → DIP-16, 300 mil | `adapters/TSSOP16-DIP16-300mil/TSSOP16-DIP16-300mil.kicad_pro` | U3, U4 (SN74CB3T3257) | 10.16 × 20.32 mm |
| TSSOP-20 → DIP-20, 300 mil | `adapters/TSSOP20-DIP20-300mil/TSSOP20-DIP20-300mil.kicad_pro` | U9 (PCM5100A) | 10.16 × 25.40 mm |

Each folder has the schematic, board, a project-local `adapter` symbol and footprint library (with its own
`fp-lib-table`/`sym-lib-table`; the 20-pin one also a `.kicad_dru`), a schematic PDF, a BOM CSV with the ordering
fields, assembly views (`<name>-assembly-top.png`, `-assembly-bottom.png`), the check reports and `jlcpcb/`:

* `jlcpcb/<name>-gerbers.zip`: upload this to jlcpcb.com (Gerbers + Excellon drill files).
* `jlcpcb/<name>-bom-jlc.csv`, `jlcpcb/<name>-cpl-jlc.csv`: optional JLCPCB assembly of the chip (U1) only.

The gerber zips themselves are not in git (the repository ignores `*.zip`, as for the main boards). Regenerate
them with `hw-common/tools/jlcpcb.sh hw-tht-v1.3.2/adapters/<name>/<name>.kicad_pcb`, which writes them to
`adapters/<name>/build/jlcpcb/`.

### Design

* **Outline = the main-board footprint's body outline**: the Edge.Cuts outline is the F.Fab polygon of
  `DIP-16_W7.62mm_TSSOP-16_Adapter` / `DIP-20_W7.62mm_TSSOP-20_Adapter` (the PA0034C/PA0035C boards), from DIP pin
  1: x −1.27 to 8.89, y −1.27 to 19.05 (16-pin) or 24.13 (20-pin), with a 1 mm 45° chamfer at the pin-1 corner. Its
  DIP holes coincide with the main-board footprint's. 2 layers, FR-4 1.6 mm, 1 oz.
* **Pin mapping**: TSSOP pin *n* goes to DIP pin *n* for every *n*, with nothing else in the signal path (nets
  `Pn_<pin name>`). A netlist and copper check proves it on both boards (`<name>-mapping-proof.txt`); ERC and DRC
  with schematic parity are clean (0 errors, 0 warnings, 0 unconnected; `<name>-erc.rpt`, `<name>-drc.rpt`).
* **DIP side**: two rows 7.62 mm apart at 2.54 mm pitch, numbered counter-clockwise from the top, pin 1 at the
  chamfered corner on a square pad with a chamfered corner.
  * 16-pin: 1.0 mm holes, 1.7 mm pads: round machined pins **or** 0.64 mm square header pins.
  * 20-pin: 0.85 mm holes, 1.6 mm pads (1.4 mm at pins 4–7 and 14–17, next to the chip): **round machined pins
    only**.
* **TSSOP land pattern**: KiCad's TSSOP-16/TSSOP-20 (0.65 mm) with toes lengthened to ±3.9 mm for hand soldering.
  The chip is turned 90° between the DIP rows: on the 16-pin board TSSOP pin 1 is at the upper right, on the
  20-pin board at the lower left (seen from the top with the DIP pin-1 chamfer at the top left). Each has its own
  pin-1 triangle. On the 20-pin board the four corner pads (TSSOP pins 1, 10, 11, 20) are shortened and sit
  0.185 mm from the DIP pads (custom rule, above JLCPCB's 0.15 mm).
* Routing: 0.2 mm tracks (0.3 mm on the 16-pin GND/VCC), ≥ 0.2 mm clearance, copper ≥ 0.3 mm from the edge, tented
  0.6/0.3 mm vias, no pour (every net is one pin-to-pin link).
* Silkscreen on both sides: pin-1 triangle and the DIP corner numbers; top "U3/U4" + "v1.3.2" or "U9"; bottom
  "SN74CB3T3257 TSSOP16" / "PCM5100A TSSOP20" and "PicoGUS v1.3.2".

### Optional DAC charge-pump pads (20-pin board only)

The TSSOP20 board has six 0603 pads for using it as a stand-alone PCM510x breakout. They are marked DNP in its
schematic, left out of its JLCPCB BOM/CPL, and **stay empty on PicoGUS**, whose main board already has these
parts right at the DIP pins:

| Pad | Value | Between | Side | On PicoGUS this is |
|---|---|---|---|---|
| C1 | 0.1 µF | CPVDD (1) – CPGND (3) | back | C36 |
| C2 | 2.2 µF | CAPP (2) – CAPM (4) | back | C16 |
| C3 | 2.2 µF | VNEG (5) – AGND (9) | back | C17 |
| C4 | 0.1 µF | LDOO (18) – DGND (19) | front | C18 |
| C5 | 0.1 µF | AVDD (8) – AGND (9) | front | C13 (+ C15) |
| C6 | 0.1 µF | DVDD (20) – DGND (19) | front | C12 |

They are not right at the pins (the DIP pin field leaves no room; they sit 4–17 mm away along the tracks), so they would add
nothing on PicoGUS. Parts if needed elsewhere: Samsung CL10B104KB8NNNC (LCSC C1591) and CL10A225KO8NNNC (LCSC C23630).

### Ordering

* **PCBs (JLCPCB)**: upload `jlcpcb/<name>-gerbers.zip`. 2 layers, FR-4, 1.6 mm, 1 oz, any colour. HASL lead-free
  works; ENIG gives flatter pads for hand soldering the 0.65 mm pitch. Leave the vias tented as in the files. Pick
  "Remove order number" (or "Specify a location"): the bottom is full of text and pins. The minimum order is 5 of
  each design; one PicoGUS needs 2 × TSSOP16 and 1 × TSSOP20.
* **Optional JLCPCB assembly** (the chip only): `<name>-bom-jlc.csv` and `<name>-cpl-jlc.csv` (SN74CB3T3257PWR =
  LCSC C544573, PCM5100APWR = C131154; J1 and C1–C6 are not placed). The boards are barely above JLCPCB's 10 mm
  minimum for assembly and may need a panel or edge rails. Check the TSSOP rotation in the placement preview (CPL
  −90° on the 16-pin, +90° on the 20-pin board): pin 1 must be at the TSSOP silk triangle.
* **Pins: round machined pin strips, insulator under the adapter.** Checked at DigiKey on 2026-10-24
  (`adapters/adapter-pins-digikey-upload.csv` has the first choice of each):

  | For | Pin strip | DigiKey | Stock / price |
  |---|---|---|---|
  | TSSOP16 board, 2 × 1x8 each | Mill-Max 350-10-108-00-006000 | ED6864-08-ND | 2 274, USD 1.88 (a DigiKey cut strip, "discontinued at DigiKey" while stock lasts) |
  | | Preci-Dip 350-10-108-00-001101 | 350-10-108-00-001101-ND | 68, USD 2.75 (30-week lead time) |
  | TSSOP20 board, 2 × 1x10 each | Preci-Dip 350-10-110-00-001101 | 350-10-110-00-001101-ND | 453, USD 3.37 (30-week lead time) |
  | either, cut to length | Mill-Max 350-10-164-00-006000 (1x64) | ED6864-64-ND | 8 908, USD 14.43 (six 1x10 or eight 1x8 pieces) |

  The Mill-Max 1x10 (350-10-110-00-006000, ED6864-10-ND) is no longer carried by DigiKey. The Mill-Max strip has
  2.79 mm of insulator, a 2.79 mm post into the adapter and 3.61 mm of round pin below it (0.46 mm both ends, so it
  goes either way round). **The Preci-Dip 350 strip is not symmetric**: its 0.64 mm solder tail (4.17 mm) goes **up**
  through the adapter and is cut flush; the 0.47 mm connecting pin (3.96 mm) goes **down** into the socket or the
  card. Both fit the 20-pin board's 0.85 mm holes (finished hole after JLCPCB's ±0.08 mm tolerance ≥ 0.77 mm). The 16-pin board's
  1.0 mm holes also take 0.64 mm square header pins (e.g. from the 1x40 strip in the main BOM), but only for
  soldering the adapter straight into the card, **never into a machined socket**.

### Assembly

1. Solder the TSSOP first, pin 1 at the TSSOP silk triangle (not at the DIP pin-1 corner): upper right on the
   16-pin board, lower left on the 20-pin board, seen from the top with the DIP chamfer at the top left.
2. Insert the two pin strips from the **bottom** (insulators under the board; Preci-Dip: solder-tail end up) and
   solder them on the top. A spare DIP socket or the main board's footprint holds them straight at 7.62 mm while you
   solder. Cut the posts flush: a Preci-Dip post stands about 2.6 mm above the board (the chip is 1.2 mm) and would
   add ~1.4 mm to the height. Next to the chip (pins 5/6/15/16 on the 20-pin board) cut along the pin row, or fit
   and cut those pins before the chip goes on.
3. On the 20-pin board, solder DIP pins 5, 6, 15 and 16 carefully after the chip: the TSSOP-20 body overhangs their
   reduced pads (about 0.09 mm from the hole edges at maximum body size). Watch the four 0.185 mm corner gaps for
   bridges.
4. Check every pin for shorts to its neighbours, then fit the adapter with its chamfered corner / square pad (DIP
   pin 1) at the main-board footprint's pin-1 triangle and notch: soldered, or in a machined socket.

### Bought alternatives

* **U3/U4**: Chip Quik PA0034C (BOM line; round pins fitted). SparkFun BOB-13994 (DigiKey 1568-13994-ND, 66 in stock
  on 2026-10-23, USD 4.25 for 2 boards): no pins, pads on a 0.635 mm pitch (a 0.65 mm TSSOP-16 fits if centred
  carefully). After snapping the pair apart a board is about 10.41 mm wide, which the DIP-16 courtyard allows for.
  Solder it on square header pins straight into the card; in a socket it needs round machined pins and reaches
  about 10.4 mm. MikroElektronika MIKROE-305 (300 mil, no headers, no published outline: not checked against the
  courtyard).
* **U9**: Chip Quik PA0035C (BOM line). Its datasheet could not be read (chipquik.com returned 403): the
  1.000 × 0.400 in outline is DigiKey's, and the fitted round pins and 1:1 mapping follow the PA0034C datasheet and
  the Compact Series listings. Check the pin-1 mark and the mapping with a meter on arrival (see above). MIKROE-307 is the only other 300-mil TSSOP-20
  adapter found, and it is sold out.
* **Never**: the plain PA0034/PA0035, Aries LCQT-TSSOP16/20, Adafruit 1206/1207, SparkFun BOB-00499 or the generic
  0.5/0.6 in boards (wrong row spacing).

### Heights

Top of the TSSOP above the card, against the ~11 mm underside of a Waveblaster daughterboard (U3, U4 and U9 are
all under a full-size one):

| Adapter | Soldered into the card | In an On Shore SA machined socket |
|---|---|---|
| PA0034C / PA0035C (pins fitted, 0.45 mm collar) | ~3.3 mm | ~7.7–7.9 mm |
| Own adapter PCB on Mill-Max/Preci-Dip 350 strips (2.8 mm insulator, posts cut flush) | ~5.6 mm | ~10.0–10.2 mm: 0.8–1.0 mm to spare; solder directly if the daughterboard has parts underneath |
| BOB-13994 | on square header pins (height depends on the header) | ~10.4 mm on round machined pins: solder it directly instead |

## Socket build

`BOM-THT-sockets.csv` (with `BOM-THT-sockets-digikey-upload.csv` and `-10x`) lists everything needed to build the
card with every IC socketed. The same parts are the optional socket lines of `BOM-THT.csv`.

| Socket for | Part (DigiKey) | Height above the board | Top of the part |
|---|---|---|---|
| U2 (DIP-20) | On Shore SA203000 (ED3020-ND) | 4.6 mm | ≤ 9.8 mm, **under the daughterboard** |
| U9 on its adapter (DIP-20) | On Shore SA203000 (ED3020-ND) | 4.4–4.6 mm | ~7.7–7.9 mm (PA0035C) / ~10.0–10.2 mm (own PCB), **under the daughterboard** |
| U3, U4 on their adapters (DIP-16 ×2) | On Shore SA163000 (ED3016-ND) | 4.4–4.6 mm | ~7.7–7.9 mm (PA0034C) / ~10.0–10.2 mm (own PCB), **under the daughterboard** |
| U5, U6, U7, U10 (DIP-14 ×4) | Adam Tech ICM-314-1-GT-HT (2057-ICM-314-1-GT-HT-ND) | 4.6 mm | ≤ 9.8 mm, **under the daughterboard** |
| U11 (DIP-8) | On Shore SA083000 (ED3038-ND) | 4.6 mm | ≤ 9.8 mm, outside |
| U1 Pico | Adafruit 5585 short sockets (1528-5585-ND) + 5584 short plug headers (1528-5584-ND) | 5.0 mm | Pico board at ~7.5 mm |

* All are machined (round-pin) sockets with 0.5 mm tails that fit the 0.8 mm DIP holes and the adapter footprints'
  1.0 mm holes. LCSC equivalents (CONNFLY DS1001-01, XKB) are in the CSV.
* **Adapters in sockets need round pins**: the PA0034C/PA0035C (round 0.46 mm pins fitted) or the own adapter
  PCBs on round machined pin strips. Never push square header pins into a machined socket; it ruins the contacts.
* **Daughterboard clearance**: a Waveblaster daughterboard sits ~11 mm above the card. U2, U3, U4, U5, U6, U7, U9
  and U10 are under it, and a socketed DIP reaches ~9.8 mm (about 1.2 mm worst-case margin, 2 mm typical). That is
  fine for a daughterboard with a flat underside; if yours has parts underneath, solder those ICs and adapters
  directly, or use the low-profile Mill-Max 115-43-3xx-41-003000 sockets (2.41 mm, they need 1.0 mm holes: the
  adapter footprints have them, the other DIPs would need a footprint change).
* **A socketed Pico has no USB host port** (J11) unless three short wires connect its TP1–TP3 to the holes under
  it. The Adafruit short sockets keep the Pico at ~7.5 mm; the standard 8.5 mm Sullins sockets at ~11–12 mm.

## Layout notes from v1.3.1

Kept on the v1.3.2 blank (the parts and board items are where v1.3.1 had them):

* **RV1 / J8**: J8 on top, RV1 below it (at the top, RV1's M7 bushing would have been 0.4 mm above the PC's slot
  opening). **J11 at x 208.64** (at x 210 its flange stood 1.26 mm into the bracket plate); C34, F1 and R4 sit
  0.45–0.7 mm left to make room. **C32** (47 µF, 11 mm tall) out from under the daughterboard at (180.4, 39.4);
  C1/C2 clear of the daughterboard corner (C1 0.4 mm even at maximum can size).
* **U10 (with C21, R18) at y 66.6**, U7 and U6 spread evenly between U10 and U5 (y 80.9 and 95.2, with C6/C9):
  about 4.5 mm between the DIPs for routing. R2 sits on the R5/R6 line (y 99.0).
* No copper inside the gold-finger area: a no-fill rule area covers the strip above the fingers, and **J3's finger
  mask opening starts 0.63 mm lower (y 129.6)**.
* **Don't run "Update footprint from library"** on J3 (the mask edit), J11 (the shield padstack below) or RV1 (the
  library footprint has a stray Edge.Cuts line that breaks the outline). DRC lists these three as
  `lib_footprint_mismatch` warnings; that is expected. The two Pico micro-USB shell relief slots (1.3 × 2.0 mm) are
  drawn on Edge.Cuts as locked board items (JLCPCB wants non-plated slots on the outline layer): move them with U1
  if U1 ever moves.
* J11's shield pad next to Pico pin 4 (SPI_SCK) has a 2.6 mm pad on the front (3.0 mm on the back) for a 0.48 mm
  gap. **Solder J11 before the Pico and check that gap for a bridge.** H2's mask opening is 0.3 mm smaller (it sits
  0.38 mm from Pico pin 39).
* Rule areas keep copper tracks out of the front exposed-copper logo, and vias and the front GND pour out from under
  the flat Pico's test pads (TP4–TP6, Pico 2 TP7) and its TP1–TP3 USB area.
* A `JLCJLCJLCJLC` order-number placeholder sits on B.Fab next to the Pico (not plotted).

What worked when routing v1.3.1 (to repeat on v1.3.2; none of it is on the blank):

* Hand pre-routes, locked, then Freerouting (100 passes): IRQ2 along the finger corridor (J1 is at the far left, its
  finger B4 at the far right); USB D+/D− flow-through U12 (see below), then on B.Cu between J11's lower shield pad
  and the Pico's pin row and through the gaps between the micro-USB shell holes to TP3/TP2 (≥ 0.3 mm from the holes
  and from J11's shield, no stubs); the U9 GND, VNEG and XSMT stubs; the U3/U4 decoupling; the −12V trunk from
  finger B7 up to the analog section.
* Design rules (unchanged in the `.kicad_pro`): 0.2 mm clearance, 0.25 mm signal tracks, 0.7/0.35 mm vias; power
  tracks (+5V, +3V3, ±12V, VBUS, AVDD) widened after routing up to 0.8 mm where they fit. The audio nets (DAC outputs
  to the mixer, WT_L/WT_R, the volume-pot wipers and the op-amp inputs) are in the `audio` net class with **0.6 mm
  clearance**. GND poured on both layers with stitching vias.
* ±12V never on F.Cu next to the Pico's castellations (a bridge there would put −12 V on a GPIO). Where ±12V crosses
  the Pico's pin rows on B.Cu it was necked to 0.2 mm for 2.2 mm, centred between the pads (≥ 0.32 mm), and kept
  ≥ 0.38 mm from C11.1 (RUN).
* **ISA GND finger B10** needs a wide link into the pour (v1.3.1: a 1.5–3 mm back-side strap between the AEN and D4
  tracks into the U3/U4 pour), and the U3/U4 pour needs a good connection to the Pico-side pour (on v1.3.1 D4 and the
  +3V3 feed to C1 separated them on the back).
* U12's GND (pin 2) had a via right at the pin and a 1 mm track to C34's GND pad, tied to J11 pin 4.

## Unchanged from v1.3.1

### USB host port (J11, sheet 4 "USB host port")

A USB-A host port at the bracket for a mouse, joystick, keyboard, flash drive (CD-ROM images) or a hub, as on the
PicoGUS NE2000 card and the PicoMEM 2.

* **Data**: straight from the Pico's own USB D+/D− test points **TP3 / TP2** (behind the Pico's 27 Ω series
  resistors). U1 uses `RPi_Pico_SMD_TH_USB`: the v1.3.0 Pico footprint plus three plated holes under TP1 (GND),
  TP2 (D−) and TP3 (D+). The Pico must be **soldered flat** (castellated edge or through-hole pins, no header) so
  the three pads can be soldered from the back of the card. The positions are the same on Pico, Pico 2, Pico W and
  Pico 2 W. On headers or in a socket, solder three short wires (26–30 AWG, under 3 cm) from TP1–TP3 into the holes,
  or use the micro-USB OTG route of v1.3.0. The Pico's micro-USB stays in parallel with J11: **use one or the other.**
  TP2 = D− and TP3 = D+ were checked against Raspberry Pi's Pico R3 schematic and KiCad's own Pico footprint.
* **J11**: Molex 67643 (0676430910, DigiKey WM4993-ND / LCSC C2798029), stock footprint
  `Connector_USB:USB_A_Molex_67643_Horizontal`. Its front lip (14.4 × 7.16 mm, the frontmost part, 12.99 mm in
  front of the pin row) sits 0.1 mm behind the inside of the bracket plate. It is larger than the 13.6 × 6.2 mm
  cut-out, so it is the panel stop: nothing of J11 enters the cut-out, only the USB plug comes in through it. The
  receptacle face is 0.9 mm behind the outside of the bracket (PicoGUS 2.0: 1.3 mm), see [Bracket](#bracket).
* **VBUS**: ISA +5 V through **F1** (Bourns MF-R050, 0.5 A hold) with C34 (220 µF low-impedance; USB 2.0 asks for
  ≥ 120 µF) and C35 (0.1 µF), always on, like the PicoMEM 2's USB-A port. The Pico's VBUS pin (J7) is not involved.
* **U12** (USBLC6-2SC6, SOT-23-6): optional ESD protection on D+/D−. It sits right behind J11's pins, placed for
  flow-through routing as on v1.3.1: D+ J11 → pin 6 → under the body → pin 1, D− pin 4 → pin 3, on to the Pico, so
  there are no stubs and the port works without U12. C35 (0.1 µF) sits just below U12. Pin 1 is marked by a silk dot
  west of the chip. **If you fit U12, solder it before C34, C35 and J11**, which box it in.
* USB-C was considered and left out: the PicoMEM 2 also uses USB-A for its host port and USB-C only for
  programming (5.1k Rd, never sourcing VBUS). A dual USB-A/USB-C footprint is in the shared library, unused.

### Waveblaster header without standoff holes

J9 uses `PiGUS library:Wavetable_NoMountingHoles`: the v1.3.0 `Wavetable` footprint without the two 3.2 mm
daughterboard standoff holes (the far one landed in U4 in the rev B placement). The daughterboard is held by the
2x13 header only, so there is no M3 standoff and no screws in the BOM. A full-size daughterboard overhangs the
card's far end; support it by hand when plugging/unplugging cables.

### Bracket

The bracket parts are where v1.3.1 had them, not where v1.3.0 (and v1.2) had them, so a v1.2/v1.3.0 bracket does
not fit. Hole positions on a Keystone 9202 style bracket (registered on the card's two tab holes H1/H2; distances
measured down from the bracket's top surface, height = centre above the card's component side):

| Part | Hole | Down from the bracket top | Height |
|---|---|---|---|
| J8 line out | Ø6.3–6.5 mm | 15.0 mm | 7.0 mm |
| RV1 WT volume (M7 bushing) | Ø7.2–7.5 mm | 23.9 mm | 6.5 mm |
| J5 MIDI out | Ø6.3–6.5 mm | 41.4 mm | 7.0 mm |
| J11 USB-A | 13.6 × 6.2 mm rectangle, corners R ≤ 0.5 | 54.0 mm (centre; the cut-out runs from 47.2 to 60.8 mm) | 1.0–7.2 mm (centre 4.1) |

* All four are inside the PC/AT slot opening (it starts 10.95 mm below the bracket top). J8 is on top because its
  Ø6 nose fits there; RV1's M7 nut needs the room further down.
* No hole for the Pico's micro-USB: its plug can't enter the slot opening. Flash the Pico before soldering it,
  or with `pgusinit`.
* The webs between the holes are about 2.0 mm (J8–RV1), 10.6 mm (RV1–J5) and 2.5 mm (J5–J11).
* J11's x position was checked against the PicoGUS 2.0, whose USB-A port is proven in production. Its bracket is a
  Keystone 9202 derivative with 1.0 mm longer tabs (its tab holes sit 0.94 mm closer to the edge connector), so
  both brackets' plates end up within 0.06 mm of each other in a case. The 2.0's USB-A face is 1.31 mm behind the
  outside of its plate; J11's is 0.89 mm behind a Keystone 9202's, so J11 (x 208.64) is right. Moving it out would
  put its front lip into the plate. A PicoGUS 2.0 bracket does not fit this card: its cut-outs are elsewhere.
* J8 and RV1 are 8.9 mm apart (v1.3.0: 9.7 mm between J5 and RV1): with a Ø8 mm plug in J8, use a knob of
  Ø9.5 mm or less on RV1.


# v1.3.0 reference (unchanged in rev B and v1.3.2 unless noted above)

## What changed in v1.3.0 compared to v1.2

### Through-hole parts

| Part | v1.2 | THT board |
|---|---|---|
| Resistors | 0805 | 1/4 W axial, 10.16 mm hole pitch |
| Ceramic caps | 0805 / 1206 | radial MLCC / disc, **5.0 mm** lead pitch (v1.3.2: except C8/C10, 0805 on the back) |
| Electrolytics | 1206 / SMD | 5 mm radial electrolytics |
| D1 | B5819W SOD-123 | 1N5819 DO-41 |
| U2 | 74LVC244 TSSOP-20 | **74AHC244 DIP-20** |
| U5 | 74AHC14 SOIC-14 | **74AHC14 DIP-14** |
| U6 | 74LVC00 SOIC-14 | **74AHC00 DIP-14** |
| U7 | 74LVC1G00 SOT-23-5 | **one gate of a 74AHC00 DIP-14**, 3 spare gates tied off |
| U10 | 74LVC2G06 SOT-23-6 (open drain, 3V3) | **74AHCT126 DIP-14 at +5 V**, used as a "pseudo open-drain" |
| FB1/FB2 | 0805 beads | axial ferrite beads (bead on leads), 12.7 mm hole pitch |

Only the parts that exist in SMD packages alone are not plain through-hole parts:

* **U3, U4** SN74CB3T3257: FET bus-switch mux with 5 V→3.3 V clamping. It only comes in TSSOP-16/TVSOP. No DIP
  part does this job without redesigning the ISA bus interface (and the PIO timing). **v1.3.2: on 300-mil
  TSSOP-to-DIP adapters** (v1.3.0/v1.3.1: soldered to the card).
* **U9** PCM5100A/PCM5101A/PCM5102A DAC: TSSOP-20. **v1.3.2: on a 300-mil TSSOP-to-DIP adapter.**
* **U8** APS6404L PSRAM: SOIC-8 (1.27 mm pitch), soldered to the card. v1.3.2: its decoupling caps C8 (0.1 µF) and
  C10 (1 µF) are 0805s on the back, right under it.
* **U12** (rev B, optional) USBLC6-2SC6 ESD array: SOT-23-6, 0.95 mm pitch.

All of them can be hand-soldered. (Rev B also had C36 as an 0603; v1.3.2 makes it a THT disc.)

The 3.3 V logic needs 5 V-tolerant inputs, so it is 74**AHC** in DIP (TI still makes these and DigiKey
stocks them). **LCSC has almost none of these DIPs in stock** (0–74 pcs on 2026-09-28): order the logic
ICs from DigiKey.

### U10: 74AHCT126 instead of 74LVC2G06

The open-drain 74LVC2G06 has no DIP equivalent that takes a 3.3 V input and has a 5 V
open-drain output. HC/LS open-drain parts either need 5 V input levels or float "high"
while the Pico boots, which would hang the ISA bus through IOCHRDY. The AHCT126 runs from +5 V and
accepts the 3.3 V signals (TTL thresholds). It is wired as a pseudo open-drain driver:
input **A is tied to GND**, and the signal that drove the old inverter input now drives **OE**. OE=1 pulls the line low;
OE=0 releases it. The polarity is the same as on v1.2, so **no firmware change** is needed.

* Gate A: IOCHRDY (as v1.2). **R18** (100 k pull-down on `~RI/OCHRDY`) keeps IOCHRDY released
  while +5 V is up but the Pico's 3.3 V rail isn't yet, or while the Pico is in reset.
* Gate B: MIDI-out current loop (as v1.2).
* Gate C: Waveblaster `/RESET`. Gate D: Waveblaster MIDI (see below).
* **Only use 74AHCT126 here** (SN74AHCT126N). A 74**HCT**126 has
  input clamp diodes to VCC: with the Pico powered from USB and the PC off, the Pico's
  outputs would back-power the ISA +5 V rail through it. A 74**HC**126 at 5 V doesn't recognise 3.3 V
  highs. An **AHC**126 would work electrically, but its 5 V thresholds leave no margin.

**R17** (10 k pull-up on `UART_TX`) holds the MIDI and Waveblaster MIDI lines idle while the Pico boots
(the RP2040 pads default to pull-down, which would otherwise send a MIDI "break").

### Waveblaster header (new)

`J9` is the standard 26-pin Waveblaster header (2x13 male, 2.54 mm; the daughterboard has the
female socket). The pinout is as on PicoGUS 2.0: DGND 1,3,5,7,9,11 · AGND 15–25 odd · +5 V 6,10,14 ·
+12 V 18 · −12 V 22 · MIDI to DB 4 · audio R 20 · audio L 24 · /RESET 26 · 2,8,12,13,16 n.c.

* **MIDI** (pin 4): the MPU-401 UART (`UART_TX`, the same data as the MIDI-out jack) through U10D,
  a pseudo open-drain driver with a **10 k pull-up to +3.3 V** (R16). The line swings 0/3.3 V: that is
  a valid TTL high for 5 V daughterboards, and it is safe for modern 3.3 V daughterboards
  (Dreamblaster etc.) that are not 5 V tolerant.
* **/RESET** (pin 26): ISA RESET through U10C, also pseudo open-drain with a 10 k pull-up to +3.3 V (R9).
* **±12 V** come straight from the ISA bus. They are now connected on this board, with 0.1 µF decoupling at J9.
* v1.3.0: the footprint includes the standard support hole at the far end of the daughterboard (M3 × 11 mm
  standoff). **Rev B and v1.3.2 have no standoff holes** (see above).

### Analogue WT volume + op-amp mixer (new)

```
                    v1.2 DAC filter
PCM510x OUTL --- 470R ---+--- 20k (R10) ---+-------+---- 47pF (C30) ----+
                         |                 |       +---- 20k (R19) -----+
                       2.2nF               |       |                    |
                         |                 |       +--|- \              |
                        GND                |          |NE5532 >---------+--- 100R (R21) --> LINE OUT L (J8)
WT L (J9.24) --+-- C22 ---- RV1A ---- R12 -+       +--|+ /
               |                                   |
          100k (R14)                              GND
               |
              GND                    (right channel: R11, R13, R20, C31, R22, RV1B, C23, R15)
```
C22 = 10 µF bipolar · RV1A = 10k log (CW end from C22, CCW end to GND, wiper to R12) · R12 = 10k · R14 = 100k

* The two sources are summed by a **NE5532 inverting summer** (U11, DIP-8, powered from ISA ±12 V).
  **PCM has (almost) unity gain** (R19/(R7+R10) = 20k/20.47k, −0.2 dB), so the PCM line out has the same
  level as on v1.2 (2.1 Vrms full scale). **The WT input has a gain of 2 (+6 dB) at RV1 max** (R19/R12 = 20k/10k):
  Waveblaster daughterboards put out only about 1–1.2 Vrms full scale (a DreamBlaster X2 uses an AKM AK4388
  DAC, 1.13 Vrms), so at unity gain MIDI ended up 5–6 dB quieter than PCM. RV1 trims it down. C30/C31
  (47 pF) put the feedback corner at ~170 kHz (10k ∥ 100 pF was ~160 kHz). The output is phase-inverted, which you can't hear.
* **Changed 2026-10-09** (BOM only, no board change): R10, R11, R19, R20 were 10k and C30/C31 were 100 pF
  (about unity gain for both sources; PCM was −0.4 dB). On a board built that way, swap R10/R11/R19/R20 to
  20k (and C30/C31 to 47 pF; leaving 100 pF costs only ~0.2 dB more at 20 kHz) to get the extra WT gain. Raising the feedback instead
  of lowering R12/R13 keeps the daughterboard's load as before: about 4.8 kΩ at RV1 max (RV1 ∥ R12 ∥ R14).
* **RV1** (dual-gang 10 k audio taper, right angle: Bourns PTD902-2020K-A103, or the Alps RK097 the footprint was drawn for) sets the Waveblaster
  level. Its shaft goes through the ISA bracket between the MIDI and line-out jacks. The signal is on the
  CW-end terminals (3/6) and GND is on the CCW end (1/4), so the level rises clockwise. The 10 k summing
  resistor loads the wiper only a little, because a log pot's wiper is near the GND end for most of its travel.
* C22/C23 (10 µF **bipolar** electrolytics, Panasonic SU) block any DC offset from the daughterboard.
  The DC across them depends on the daughterboard (some outputs are AC-coupled, some carry an offset), so a
  polarised cap could end up reverse-biased; a bipolar one doesn't care. R14/R15 (100 k) give the
  capacitors a defined DC level when no daughterboard is fitted. Without a daughterboard, RV1's setting doesn't matter.
* The PCM510x sees a ~20.5 kΩ load (R7 + R10; its datasheet minimum is 1 kΩ). The output is DC-coupled, which is fine because the
  DAC's outputs are ground-centred; expect a few mV of offset at the jack.
* R21/R22 (100 Ω) isolate the op-amp from cable capacitance. The NE5532 drives 600 Ω headphones
  easily, but this is still a line output.
* **Op-amp supply filter.** The ISA ±12 V rails carry PSU ripple and noise from other cards. They reach U11
  through R23/R24 (10 Ω) and C32/C33 (47 µF), an RC low-pass at about 340 Hz, with C28/C29 (0.1 µF) at the
  op-amp pins. The NE5532's supply rejection falls at high frequencies, which is where this filter works.
  The daughterboard still gets the plain ±12 V (decoupled at J9); it has its own regulators and filtering.
* **DAC analogue supply.** On v1.2 the PCM510x AVDD (pin 8) shares the Pico's switch-mode 3.3 V with all
  the logic. Here AVDD has its own rail behind ferrite bead FB3 (the same part as FB1/FB2), with C13
  (0.1 µF) and C15 (10 µF) at the pin. DVDD and CPVDD stay on the plain 3.3 V, as on v1.2/v1.3.0. Since rev B the
  charge-pump supply CPVDD (pin 1) has its own 0.1 µF, C36 (v1.3.2: a THT disc right beside DIP-20 pins 1 and 3),
  so its current loop pin 1 → C36 → pin 3 (CPGND) is short (before rev B it was about 24 mm of track to C4 and back
  through the pour). TI's layout guide and PicoGUS 2.0 put this cap right at the pin. CPVDD stays on +3V3, as on
  v1.2/v1.3.0, and TI's 10 µF bulk cap at CPVDD is not fitted locally (the nearest bulk is C1). The bead drops only
  millivolts at the DAC's ~10 mA.
* **Capacitor types in the signal path:** C19/C20 (DAC output filter) and C30/C31 (op-amp feedback) are
  **C0G/NP0**, which has no voltage or temperature coefficient and so adds no distortion. X7R is used only for
  supply decoupling.

v1.3.0 and v1.3.1 also had a footprint for a GY-PCM5102 DAC module (J10) as an alternative to the on-board DAC;
**v1.3.2 has no module option** (see above).

### Back silkscreen

The back carries the princess artwork (hw-common/art/mila-alisa.svg: a big and a small princess, both crowned,
with sparkles and a castle, over the PicoGUS logo), 72.5 × 37 mm, its lower edge 3 mm above the Pico's pin holes,
6 mm off the card's centre toward the bracket. **On v1.3.2 this is still v1.3.1's placement**, only re-cut around
the new pads (several of which now land on the faces, see [Silkscreen and back artwork](#silkscreen-and-back-artwork));
it will be re-placed after the layout. The back is full of through-hole pads, so the artwork is cut out 0.3 mm
around every pad and hole (silkscreen never lands on copper).

The version and licence block sits between the Pico's pin rows, one text per line, ending with the thanks and the
dedication: *Thanks to polpo and all the PicoGUS contributors. This revision is dedicated to my wonderful
family, Mila and Alisa.* On v1.3.1 each line was nudged (at most 0.9 mm) so that no via hole lands in a letter;
check that again after routing v1.3.2. "< REAR" points at the bracket, its arrow tip in line with the bracket tab
holes H1/H2. All are mirrored on B.SilkS so they read correctly from the back.

### Other changes

* The board is **103.7 × 99.9 mm**. It has the same length, ISA edge connector and bracket tab holes (H1/H2) as
  v1.2 (on v1.3.0 the MIDI and line-out jacks are also in the v1.2 places; rev B and v1.3.2 move them, see
  [Bracket](#bracket)), but it is taller, like PicoGUS 2.0, to fit the Waveblaster header along the top edge.
  1.6 mm FR4 (the v1.2 and v1.3.0 board files' stackup says 0.57 mm; rev B's and v1.3.2's say 1.6 mm, so their 3D
  and STEP exports have the right thickness).
* Design rules are relaxed for THT/DIY: 0.2 mm clearance, 0.25 mm signal / 0.4 mm power tracks
  (+5 V, +3.3 V, VSYS), 0.7/0.35 mm vias (rev B and v1.3.2: the same, plus a 0.6 mm-clearance `audio` class).
  GND is poured on both layers.
* **J2 (the 2x31 ISA debug/probe header of v1.2, never fitted) is removed.** On a through-hole board
  its pads formed a wall right above the gold fingers that every ISA signal had to cross.
* The `+3.3V` power symbols are renamed `+3V3`. KiCad 8+ names power nets after the symbol value, so the
  upgraded v1.2 sheets would otherwise have split the 3.3 V rail into two nets.
* The symbol fields of U3/U4 now name the CB3T3257 that is actually used (the v1.2 fields still said 74FST3257DR2G).
* **Layout is grouped by circuit block.** Each chip and its decoupling caps and resistors form a KiCad group,
  so a block can be selected and moved as one: DAC (U9), Mixer + line out (U11, J8), WT volume (RV1),
  Waveblaster header (J9), MIDI out (J5), Pico + power (U1), ISA bus switches (U3, U4), ISA buffer (U2),
  74AHC14 (U5), 74AHC00 (U6, U7), 74AHCT126 (U10), PSRAM (U8, with C8/C10 on the back), IRQ/DMA jumpers (J1).
* **Layout for audio quality:** every decoupling cap sits next to its chip's supply pin. FB3 sits next to C13/C15.
  The ±12 V filter (R23/R24, C32/C33) sits above U11. The DAC is turned so that its outputs, the output RC filter
  (R7/R8, C19/C20) and the mixer op-amp form one short path to the line-out jack, away from the Pico and the ISA
  logic (v1.3.2: the DIP-20 at 180°, pins 1–10 toward the mixer).

## Card dimensions (ISA 8-bit)

The card is **103.7 mm long × 99.9 mm tall** (bottom of the fingers to the top edge). The IBM PC/XT
full-height limit is 4.2 in (106.7 mm), so the card is 6.8 mm under it; the AT limit is 4.8 in (121.9 mm).
The edge connector (62 contacts, 0.1 in pitch), its position and the bracket holes are unchanged from
v1.2, and the board is 1.6 mm, the standard ISA card thickness. A Waveblaster daughterboard sits about
11 mm above the card, inside the 0.8 in (20.3 mm) AT slot pitch. Clip the THT leads short
(≤ 2 mm) on the solder side, where the next card sits. C8/C10 on the back are 0805s (about 1.3 mm high at most).

The **SMD** version of this board is in [`hw-smd/`](../hw-smd/README.md).

## Ordering

`BOM-THT.csv` lists, for every line, the DigiKey part (with DigiKey stock) and the LCSC part (with LCSC stock),
plus alternatives: v1.3.0 lines as of 2026-09-28, rev B lines 2026-10-18, v1.3.2 lines 2026-10-23. DigiKey numbers
were checked against findchips distributor data or DigiKey itself, and LCSC numbers against the LCSC product API.

* **Logic ICs (U2, U5, U6, U7, U10): order from DigiKey.** LCSC lists the DIPs, but has 0–74 in stock.
* **Adapters: required.** Two TSSOP-16 → DIP-16 for U3/U4 and one TSSOP-20 → DIP-20 for U9. The BOM lines are the
  Chip Quik PA0034C (28 at DigiKey on 2026-10-23) and **PA0035C (only 1 at DigiKey on 2026-10-23**, 4-week lead time;
  the Proto-Advantage store lists it in stock). The 10-card upload asks for 10 PA0035C, so expect a back-order,
  or use the own adapter PCBs (`adapters/`, with `adapters/adapter-pins-digikey-upload.csv` for the pins). Neither
  Chip Quik adapter is sold by LCSC.
* **SMD parts on the card**: U8 (SOIC-8, front), C8/C10 (0805, back) and the optional U12 (SOT-23-6, front). Solder
  them before the through-hole parts.
* **DigiKey BOM cost**: the v1.3.0/v1.3.1 estimate (2026-09-28, parts only, no PCB, no shipping, from DigiKey's
  price breaks) was ~474 kr excl. MVA (~$50) for one board and ~356 kr (~$37) per board in a 10-board order (NOK
  at 9.53 NOK/USD; digikey.no can differ by a few percent). **v1.3.2 adds the three Chip Quik adapters, USD 22.17
  (~211 kr excl. MVA)** per card at DigiKey; with the own adapter PCBs it is about USD 14 of pin strips per card plus
  the PCBs. The 0805 C8/C10 and the disc C36 change the total by less than 10 kr. The optional machined socket set
  was ~80 kr on rev B, plus ~21 kr for the Pico sockets; v1.3.2 adds two DIP-16 sockets and a second DIP-20.
* **Ceramic capacitors must have 5.0 mm (or 5.08 mm) lead pitch.** 2.5 mm parts don't fit (e.g. TDK FG1x, Vishay K…L2).

Mistakes found in the v1.2 BOM/JLCPCB data (don't copy these from the v1.2 files):
C139966 is a Vilsion part, not the APS6404L. C2910528 is a 3.5 mm jack with a different pinout.
C2905423 is a female header, not the Pico. C2935925 is DEALON, not Amphenol. C6942 is the SOIC SN74AHC14DR.

## JLCPCB files

```
hw-common/tools/jlcpcb.sh hw-tht-v1.3.2/PicoGUS-THT.kicad_pcb   # -> hw-tht-v1.3.2/build/jlcpcb/
```

The script stops if DRC (including the schematic-parity check) finds errors. **On the unrouted blank it stops on
the 309 unconnected items**; `FORCE=1` exports anyway, but those gerbers have no tracks and are only good for
checking the placement, never for ordering. After routing it writes:

* `PicoGUS-THT-gerbers.zip`: upload this to jlcpcb.com. It holds the Gerbers (Protel extensions, no X2)
  and the Excellon drill files (PTH and NPTH separate, mm).
* `PicoGUS-THT-bom-jlc.csv` and `PicoGUS-THT-cpl-jlc.csv`: the BOM and placement files for JLCPCB assembly. The
  placement file has the SMD parts only: **U8 and U12 on Top, C8 and C10 on Bottom**. U3/U4/U9 are DIP footprints
  now, so they are not in it (the TSSOPs go on the adapters; see [Adapter boards](#adapter-boards) for their own
  JLCPCB files). With SMD parts on both sides, JLCPCB needs two-sided assembly for all four (its economic service
  places one side only; check at order time); hand-soldering the 0805s is easy.
* `PicoGUS-THT-drc.rpt`: the DRC report.

The **Hardware (JLCPCB files)** GitHub Action runs the same script for `hw-tht` (v1.3.0) and `hw-smd`. v1.3.1 and
v1.3.2 are not in it.

Order options: 2 layers, **1.6 mm**, **Surface finish: ENIG** and **Gold fingers: Yes** with the **45° finger
chamfer** (the ISA edge connector). ENIG is required: JLCPCB only gold-plates the fingers on ENIG boards; with HASL
the fingers are tinned, and tin wears and oxidises in an ISA slot. Check the rotations in JLCPCB's placement preview
before paying for assembly (especially the bottom-side C8/C10). JLCPCB no longer prints an order number by default.
The `JLCJLCJLCJLC` placeholder sits on B.Fab next to the Pico (not plotted): if an order ever needs "Specify a
location", move it to B.Silkscreen first, or JLCPCB prints the placeholder text itself.

## Caveats

1. **Unrouted and untested.** v1.3.2 is a placement-only blank. The circuit is the proven v1.2 one with the
   v1.3.x additions, but no v1.3.x THT board has been fabricated or run in a PC yet. Route it, check the routing
   and DRC, and re-place the back artwork before ordering.
2. **Audio needs −12 V.** The mixer op-amp runs from ISA ±12 V, so without −12 V there is
   **no line output at all** (not even PCM). Every AT/ATX PC power supply provides −12 V. Some unusual
   backplanes or small DC-DC "pico" supplies may not, so check yours.
3. **Slower logic in the ISA path.** 74AHC00 (U6/U7) is a few ns slower than the v1.2 74LVC00,
   and IOCHRDY now goes through an AHCT126 instead of an LVC2G06. At ISA speeds this should be
   harmless, but it hasn't been tested on fast (10–12 MHz) buses.
4. **AHC input thresholds.** At 3.3 V, 74AHC needs ~2.3 V for a logic high (74LVC: 2.0 V). The ISA
   signals that reach U5/U7 directly (AEN, RESET, DACK) are fine from any normal chipset or
   74LS/F/ACT driver (VOH ≈ 3.4 V or more). A marginal old bus that only gives the TTL minimum of 2.4 V
   has little margin.
5. **Daughterboard clearance.** The Waveblaster daughterboard sits ~11 mm above the card, and a
   full-size (Creative-spec) board covers most of the card's left half. Only low parts are placed under it (DIP ICs
   and the adapters, axial resistors, radial MLCCs, the J1 jumper block as on PicoGUS 2.0). Electrolytics and the
   pot are outside that area; the Pico's far end is 1.3 mm under it, so with a daughterboard fitted solder the Pico
   flat or use the short Adafruit sockets. IC sockets add 4.6–5.1 mm, and a socketed own adapter PCB reaches ~10 mm:
   solder those directly or check the clearance if the daughterboard has parts on its underside (see
   [Socket build](#socket-build) and [Heights](#heights)).
6. **Adapters.** PA0035C stock is thin and its datasheet was not readable; check its pin-1 mark on arrival. Never
   put square header pins into machined sockets. On the own TSSOP-20 adapter, solder DIP pins 5, 6, 15 and 16 with
   care (the chip body overhangs their pads).
7. **RV1.** The BOM uses the Bourns PTD902-2020K-A103 (DigiKey 426, 24 kr). It has the same 2.5 mm
   pin grid and M7 bushing as the Alps RK097 the footprint was drawn for; test-fit one in the bracket
   before a large order. LCSC doesn't list it, so the LCSC column keeps the Alps RK09712200HA
   (C470545). The Alps parts (RK09712200HA: DigiKey 210 / LCSC 175, 45 kr; the switched RK0971221Z0X
   also fits) have low stock, and Alps North America has published a discontinuation notice that
   covers parts of the range.
8. **ISA bracket.** No v1.2/v1.3.0 bracket fits; cut the four openings in the [Bracket](#bracket) table.
9. **Mixed LCSC stand-ins.** LCSC's 3.5 mm jacks are XKB PJ-325C5 equivalents. Their unused
   TN pin sits ~0.4 mm further out, so clip it if it doesn't fit. Don't use C2910528
   (different pinout; it is in the v1.2 JLCPCB project). The Pico is not sold in the LCSC shop
   (only in the JLCPCB assembly library). DigiKey doesn't stock the APS6404L PSRAM; Adafruit 4677 (the bare chip) is
   the DigiKey source.
10. **Ferrite beads.** FB1/FB2 are axial beads on leads (DigiKey Fair-Rite 2743002112, LCSC FH
    RH3.5x0.8x9P52E). A wire link also works (MIDI out works without the beads, with slightly worse EMI).
11. The v1.2 symbols are cached in the schematic, so KiCad reports "symbol doesn't match library"
    warnings. That is expected; don't "update symbols from library" blindly.
12. Waveblaster pin 8 (MIDI out from the daughterboard) and the daughterboard audio inputs (pins 12/16)
    are not used. PicoGUS has no MIDI in.
