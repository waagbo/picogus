# PicoGUS THT (through-hole) v1.3.2 (work in progress) – based on PicoGUS v1.2

Board revision **v1.3.2**: the [v1.3.1 (rev B) THT board](../hw-tht-v1.3.1/README.md) made buildable without
fine-pitch soldering on the card itself.

* **U3/U4** (SN74CB3T3257) and **U9** (PCM5100A) only exist as TSSOPs. On v1.3.2 each one sits on a **300-mil
  TSSOP-to-DIP adapter board** that goes into a plain DIP-16/DIP-20 footprint, soldered or socketed. The
  adapters can be bought (Chip Quik PA0034C/PA0035C) or made from the two adapter PCBs in this folder
  (see [Adapter boards](#adapter-boards)).
* **The GY-PCM5102 module option (J10) is gone.** The on-board DAC is always fitted.
* **C36** (U9 charge-pump decoupling) is now a through-hole 0.1 µF disc just below the DIP-20's pin 1 (it was an
  0603 under the TSSOP).
* **C8/C10** (U8 PSRAM decoupling) are the v1.3.1 through-hole discs, in a row just above U8.
* **1.84 mm shorter**: 102.0 mm long over the outline (v1.3.1: 103.8 mm), trimmed at the bracket end. The bracket tab
  holes H1/H2 are 1.70 mm further from the bracket plate, so the card needs a bracket with longer tabs (see
  [Bracket](#bracket)).
* **No artwork on the back silkscreen**: the princess artwork is to be UV printed instead. The back texts stay.

The main board is through-hole apart from **U8** (APS6404L PSRAM, SOIC-8, 1.27 mm pitch) and the optional **U12**
(USBLC6-2SC6, SOT-23-6), both on the front. The TSSOPs are soldered to the adapters, not to the card. The rest of the
circuit is v1.3.1's: USB-A host port, Waveblaster header, op-amp mixer.

> **Status: routed; not built or tested yet.** The 102 mm placement (2026-10-28) is routed on two layers
> (2026-10-30). The schematic's ERC is the same as v1.3.1's (280 warnings, 0 errors). DRC with schematic parity finds
> 0 unconnected items and, as errors, only the 12 accepted courtyard overlaps in the logic column, so
> `hw-common/tools/jlcpcb.sh` exports with `FORCE=1`; `build/jlcpcb/` holds that export (not in git: the repository
> ignores `build/`). The routing was reviewed twice from an analog/power and a digital/ISA/USB/manufacturing side,
> and the findings were fixed or are listed as accepted. Compared with the routed 103.7 mm layout before it (commit
> 8dbb6c6) it is better on the ISA data bus, digital-to-audio and DAC isolation, the line-out's ground error from a
> ground loop to the fingers or from USB load, ±12V and power clearances, vias near the fingers and the USB-host +5V
> feed, and worse on I2S length and vias, the line-out's ground error from a ground loop to the bracket, the B10
> ground-finger track and the ground resistances if the Pico's GND pins are not tied inside the module: see
> [Layout status](#layout-status).

Files:

| File | What |
|---|---|
| `PicoGUS-THT.kicad_pro/.kicad_sch/.kicad_pcb` | KiCad **10** project (the root sheet is the v1.2 logic). The board is routed |
| `audio.kicad_sch` | PCM510x DAC sheet: U9 on its DIP-20 adapter, C36 a THT disc, no J10 |
| `wavetable.kicad_sch` | Waveblaster header, WT volume, op-amp mixer, line-out jack (as v1.3.1) |
| `usb.kicad_sch` | USB host port (J11 USB-A, F1, C34/C35, U12) (as v1.3.1) |
| `PicoGUS-THT-schematic.pdf` | Schematic PDF |
| `BOM-THT.csv` | BOM with DigiKey **and** LCSC part numbers and a digikey.no link per line (v1.3.0 lines checked 2026-09-28, rev B lines 2026-10-18, v1.3.2 lines 2026-10-23) |
| `BOM-THT-digikey-upload.csv` (+ `-optional`, `-10x`) | DigiKey cart/BOM upload with the reference designators as Customer Reference. The bought adapters (PA0034C ×2, PA0035C) are in the main file; optional: sockets, U12 |
| `BOM-THT-sockets.csv` (+ `-digikey-upload`, `-10x`) | Socket build: every IC (and adapter) socketed, with heights (see [Socket build](#socket-build)) |
| `adapters/TSSOP16-DIP16-300mil/`, `adapters/TSSOP20-DIP20-300mil/` | The two adapter PCBs: KiCad 10 projects with ready JLCPCB files (see [Adapter boards](#adapter-boards)) |
| `adapters/adapter-pins-digikey-upload.csv` (+ `-10x`) | DigiKey upload: the round machined pin strips for the own adapter PCBs (one card: 4 × 1x8, 2 × 1x10) |
| `docs/renders/` | 3D renders of the routed card from the front and the back, in red, purple, black and blue solder mask (ENIG, white silkscreen; the adapter boards on their DIP sockets in the same colour) |
| `docs/figures/` | Figures for the README's exceptions and design choices (e.g. the accepted courtyard overlaps), with the scripts that make them in `docs/figures/src/` |

The shared `PiGUS library` symbols and footprints are linked from `hw-common/`, like the other boards. **New in the
shared library for v1.3.2:** `DIP-16_W7.62mm_TSSOP-16_Adapter`, `DIP-20_W7.62mm_TSSOP-20_Adapter` and
`RPi_Pico_SMD_TH_USB_NoSWD` (U1, see [102 mm board](#102-mm-board)), in `hw-common/PiGUS library.pretty`; DRC's
library check finds the board's U1, U3, U4 and U9 identical to them. The older `DIP-16_W7.62mm_TSSOP-16_Nested`
stays unused.

Every part has a 3D model. The ones KiCad 10's library does not have are in `hw-common/parts`, reached through the
project folder's `parts` link: the CUI SJ1-3535NG jacks (J8 green; J5 a black copy of the same CUI model), the Alps
RK09712 pot (RV1), a simple generated Bourns MF-R050 (F1), and the U3/U4/U9 adapter stacks: KiCad's DIP socket
model stretched to the On Shore SA socket's 4.5 mm, with the own adapter PCB from `adapters/` on round machined pin
strips above it (`TSSOP16-DIP16-300mil-adapter.step`, `TSSOP20-DIP20-300mil-adapter.step`, exported from those
boards). U1 uses KiCad's own `RaspberryPi_Pico.step`, and J9's header model path is fixed. The library footprints
carry the same models as the board.

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
  adapter PCBs.
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

### C36: THT disc below U9 pin 1

C36 (U9 CPVDD decoupling) is now the board's standard 0.1 µF 50 V X7R radial disc (Murata RDER71H104K0M1H03A,
the 0.1 µF line of the BOM) instead of an 0603: an SMD part under an adapter board makes no sense. It stands
vertically just below DIP-20 pin 1 (CPVDD), its courtyard against the DIP-20's. It cannot go under the adapter: a
soldered adapter sits about 0.45 mm above the board, and a socket body covers the area. C36 pad 1 is 2.70 mm
straight below pin 1; pad 2 (GND) is 9.2 mm in a straight line from CPGND (pin 3), and C16 sits on that line, so
the ground leg will be longer than that (v1.3.1, with the 0603 under the TSSOP: 1.9 / 0.6 mm of copper; the routed
v1.3.2 layout, commit 8dbb6c6: 2.7 mm of track / 8.2 mm through the pour by its README; the script of
[Layout status](#layout-status) gives 2.7 / 10.0 mm for 8dbb6c6 and the same for this layout). Turning C36
horizontal right under
pins 1–3 would bring both legs to 2.7 mm (see [Placement suggestions](#placement-suggestions)).

### C8/C10: through-hole discs above U8

| Ref | Part | DigiKey | LCSC |
|---|---|---|---|
| C8 | 0.1 µF 50 V X7R radial disc, 5.0 mm pitch, Murata RDER71H104K0M1H03A (the BOM's 0.1 µF line) | 490-9144-1-ND | C527620 |
| C10 | 1 µF 50 V X7R radial disc, 5.0 mm pitch, Murata RDER71H105K2M1H03A | 490-9146-1-ND | C1620132 |

These are the v1.3.1 parts again; the routed v1.3.2 layout (commit 8dbb6c6) had 0805s on the back under U8. Both
discs lie horizontally (180°) in one row just above U8 (SOIC-8, front, at 0°: pin 1 top left, pin 8 (+3V3) top
right, pins 3/4/7 GND), at y 88.40, where FB3 was on the routed layout: C8 (0.1 µF) with pad 1 (+3V3) at x 191.50
and pad 2 (GND) at x 186.50, C10 (1 µF) with pad 1 at x 184.00 and pad 2 at x 179.00. C8's +3V3 pad is 3.6 mm
straight above U8 pin 8. (v1.3.1: both vertical, origins C8 (191.30, 94.50), C10 (179.54, 94.03).) Their references
sit in the middle of each outline, as on the other discs. With them, every SMD part (U8, U12) is on the front.

### 102 mm board

The bracket end of the outline is trimmed 1.84 mm (x 220.13 → 218.29). The board is 101.90 × 99.85 mm at the
outline's line centres, 102.00 × 99.95 mm over its 0.1 mm lines (v1.3.1 and the routed v1.3.2 layout: 103.74 ×
99.85 / 103.84 × 99.95 mm). JLCPCB quoted USD 24.50 for 10 black ENIG 2-layer boards at 102.0 mm (and 101.9 mm), USD 33 at
102.7 mm and 33.53 at 103.74 mm (2026-10-28). The ISA fingers, the other three edges and the bracket plate's place in the case do not change.
To keep copper at least 0.35 mm from the new edge, these parts moved left, away from the bracket:

| Part | Moved | Now | Nearest copper to the new edge |
|---|---|---|---|
| H1 / H2 (bracket tab holes) | 1.70 mm | x 214.70 | 0.39 mm (their 6.4 mm pads) |
| J8 (line out) | 1.10 mm | x 217.00 | 0.39 mm (sleeve pad) |
| J5 (MIDI out) | 0.90 mm | x 217.00 | 0.39 mm (sleeve pad) |
| U1 (Pico), with its five test-pad rule areas and its two micro-USB relief slots | 1.35 mm | centre x 192.88, pin 1 (217.01, 100.00) | 0.36 mm (TP2/TP3); the slots (centre x 216.88) end 0.41 mm from the edge |
| U4 / U3 | 0.45 / 0.35 mm | pin 1 x 157.74 / 146.53 | – (they make room for the Pico: U4's courtyard is 0.05 mm from the Pico's, U3's 0.04 mm from U4's) |

J11 and RV1 stay where they were. The distances are to the outline's line centre. Between these parts and the edge
there is no room for a track or a via: 0.3 mm copper-to-edge, a 0.2 mm track and 0.2 mm clearance need 0.7 mm. What
the trim means for the bracket is under [Bracket](#bracket): the tab holes need a bracket with 1.70 mm longer tabs.

* **U1 uses the new footprint `PiGUS library:RPi_Pico_SMD_TH_USB_NoSWD`**: `RPi_Pico_SMD_TH_USB` without the
  three SMD castellation pads at the SWD end (pads 41–43: SWCLK and SWDIO, both not connected, and GND; their
  through-holes stay) and with the courtyard 0.2 mm past the module there instead of 0.5 mm. With the old
  courtyard, U4's would overlap the Pico's by 0.25 mm.
* The Pico's USB end (x 218.38) is now flush with the edge: 0.09 mm past the outline's line centre (0.40 mm inside
  it before).
* The Pico's far end is at x 167.38, 2.9 mm under a full-size Waveblaster daughterboard's outline (x 170.32; routed
  v1.3.2 layout 1.6 mm, v1.3.1 1.3 mm). That matters for a socketed Pico (see [Socket build](#socket-build)).

### Placement

The board holds a new hand arrangement on the 102 mm outline (2026-10-28), routed afterwards (see
[Layout status](#layout-status)). Compared with v1.3.1:

* **Logic column at the left edge.** U10, U7, U6 and U5 (DIP-14, 90°, pin 1 at the left as on v1.3.1) are stacked
  in line with U2 at x 120.07, 12.05 mm apart (pin 1 at y 65.62 / 77.67 / 89.71 / 101.76; U2: 113.81). Each chip's
  decoupling cap lies horizontally (0°) in the gap above it, pad 1 2.21 mm straight above the supply pin (pin 14;
  U2: pin 20): C21 (U10, +5V), C6 (U7), C9 (U6), C7 (U5) and C4 (U2), pads at x 120.07 / 125.07, y 55.79 / 67.83 /
  79.88 / 91.93 / 103.97. R18, R5, R6 and R17 stand vertically (−90°) in one column at x 138.60, at the chips'
  right-hand ends (pad 1 at y 56.73 / 68.78 / 80.82 / 92.87). The column is packed tighter than the courtyards
  allow: 12 overlaps, accepted (see [Layout status](#layout-status)).
* **U2** (DIP-20, 90°) at the lower left, pin 1 (120.07, 113.81), with C4 above it at (120.07, 103.97). **J1** (the
  IRQ/DMA jumper block) at x 120.00, pin 1 (120.00, 123.70), with its labels.
* **U3/U4 swapped**: U3 (D0–D3, A0–A3) is now the left DIP-16, pin 1 (146.53, 106.42), and U4 (D4–D7, A4–A7) the
  right one, pin 1 (157.74, 106.44), both at 0°. C3/C5 sit above each chip's pin 16 (+3V3), R2 at (161.50, 100.00).
  The swap shortens the D-bus pad-to-pad span (finger to chip pin, D0–D7) to 256 mm (v1.3.1: 267 mm). Pins 8/9
  (y 124.2) are inside the no-fill strip above the fingers.
* **DAC**: U9's DIP-20 lies horizontally (90°), pin 1 (146.00, 78.30) at the bottom left; the whole DAC group is
  15.00 mm further left than on the routed v1.3.2 layout (C13/C15 11.60 mm). Pins 1–10 (charge pump, outputs, AVDD,
  AGND) are the bottom row (y 78.30), pins 11–20 (I2S, LDOO, DGND, DVDD) the top row (y 70.68). Below the bottom
  row, from the left: C36 (vertical under pin 1), C16 (CAPP/CAPM) and C17 (VNEG), C19/C20 (x 149.00, y 87.50 /
  90.50), the output filter R7/R8 (vertical, x 157.40 / 160.60), FB3 (now vertical at x 164.10, its AVDD pad just
  below pin 8 (AVDD), its +3V3 pad at y 93.73) and C13/C15 (AVDD) under pins 9–10, their AVDD pads at x 167.18.
  Above the top row: C14 (+3V3), C12 (DVDD) and C18 (LDOO) at x 146 / 149 / 152, and the `DAC` label. R7/R8, FB3,
  C13 and C15 are now under a full-size daughterboard too (all low parts).
* **USB host corner**: U12 moved left next to U8, to (197.14, 91.45); F1 is vertical at (204.00, 90.00); C35 under
  U12 at (200.00, 95.00); C34 0.8 mm up. U8 is turned to 0° at (189.00, 93.90), with C8/C10 in a row just above it.
* **Pico (U1) 1.62 mm left** (pin 1 at (217.01, 100.00)): 0.27 mm on the routed layout and 1.35 mm more for the trim.
  Its test-pad rule areas and its two micro-USB relief slots on Edge.Cuts moved with it. C1, C2, C11, D1, R1, J6 and
  J7 are 0.27 mm left of v1.3.1 (they did not move again).
* **Bracket end**: H1/H2 1.70 mm, J8 1.10 mm and J5 0.90 mm left (see [102 mm board](#102-mm-board)); J11 and RV1
  stay.
* Smaller moves: R16 vertical at (156.50, 55.50); C24, C25 and C26 in a row at y 45.50 (x 148.60 / 143.60 / 139.00);
  FB1, FB2, R3, R11, R13 and R20–R22 1.5 mm right; R4 0.36 mm left.

| Part | v1.3.1 | v1.3.2 |
|---|---|---|
| U10 / U7 / U6 / U5 | (126.98, 66.60) / (127.08, 80.90) / (127.08, 95.20) / (126.88, 109.50), 90° | x 120.07, y 65.62 / 77.67 / 89.71 / 101.76, 90° |
| C21 / C6 / C9 / C7 | x 123.92 (C7 123.72), y 59.98 / 74.71 / 88.81 / 103.50, −90° | x 120.07, y 55.79 / 67.83 / 79.88 / 91.93, 0° |
| R18 / R5 / R6 / R17 | (126.92, 69.17) / (135.48, 99.00) / (122.64, 99.00) / (122.64, 112.35), 0° | x 138.60, y 56.73 / 68.78 / 80.82 / 92.87, −90° |
| U2 | (151.07, 95.31), 90° | (120.07, 113.81), 90° |
| J1 | (121.53, 122.91) | (120.00, 123.70) |
| U3 / U4 | TSSOP-16 centre (162.81, 114.11) / (149.19, 114.28), 90° | DIP-16 pin 1 (146.53, 106.42) / (157.74, 106.44), 0° |
| U9 | TSSOP-20 centre (153.02, 66.14), 90° | DIP-20 pin 1 (146.00, 78.30), 90° |
| C36 | 0603 (150.40, 70.80) | disc (146.00, 81.00), −90° |
| FB3 | (162.00, 56.50), −90° | (164.10, 93.73), 90° |
| U8 | (187.00, 91.50), −90° | (189.00, 93.90), 0° |
| C8 / C10 | (191.30, 94.50) / (179.54, 94.03), 90° | (191.50, 88.40) / (184.00, 88.40), 180° |
| U12 | (203.78, 90.88) | (197.14, 91.45) |
| F1 | (192.00, 85.90), 0° | (204.00, 90.00), −90° |
| U1 (Pico) | (194.50, 108.89) | (192.88, 108.89) |
| C4 / R2 | (147.84, 88.81) / (158.86, 99.00) | (120.07, 103.97) / (161.50, 100.00) |
| H1 / H2 | x 216.40 | x 214.70 |
| J8 / J5 | x 218.10 / 217.90 | x 217.00 / 217.00 |

Positions are footprint origins (pin 1 for the DIPs), in mm.

### Silkscreen and back artwork

* Front `v1.3.2`, back `PicoGUS v1.3.2`.
* **Label font: Jost, embedded in the board file.** The 21 board labels (`WAVEBLASTER`, `MIXER`, `DAC`, `v1.3.2`,
  `LINE OUT`, `WT VOL`, `MIDI OUT`, `UART`, `USB PWR`, `IRQ`/`DMA` and their numbers, `< REAR`) use Jost Medium and
  Bold ([indestructible-type/Jost](https://github.com/indestructible-type/Jost), SIL Open Font License 1.1; the
  copyright and licence are in the font files' metadata), embedded with the board's embedded-fonts option. KiCad
  therefore shows and plots them the same on every machine, with or without the font installed. They were in
  Futura, inherited from v1.2, which was not embedded: each machine plotted its own substitute (DejaVu Sans or Inter
  in the cloud sessions that made the earlier gerbers). Futura cannot be embedded: Futura PT is marked
  "Preview & Print" only, so KiCad refuses it, and its licence forbids sharing the files. Jost is a free
  Futura-style face and looks almost the same at this size. Embedding adds about 120 KB to the board file. The
  v1.3.1 and v1.3.0 boards still use un-embedded Futura.
* **No princess artwork on the board.** It is removed from the back silkscreen and is to be UV printed on the back
  instead (source: `hw-common/art/mila-alisa.svg`; the routed v1.3.2 layout had it at 52.8 × 27.0 mm, v1.3.1 at
  71.6 × 37.0 mm). The back text block (version, copyright, licence, URL, thanks, dedication) and `< REAR` stay.
* **Front copper logo moved.** The exposed-copper PicoGUS logo (F.Mask openings over the front GND pour, with a rule
  area that keeps tracks and vias out) is turned horizontal and sits along the top edge above `LINE OUT`, x
  185.9–211.6, y 37.8–40.6, with `v1.3.2` right of it at (214.1, 40.0). Before, the logo stood vertically at the left
  edge (x 118.3–121.1, y 43.5–69.3) and `v1.3.2` at (120.5, 40.5). The pour under it is filled, so the logo shows
  copper.
* Labels moved: `DAC` horizontal above U9 at (158.7, 66.0) (was vertical at (186.3, 74.5)), `MIXER` to
  (168.5, 39.0) (was (165.9, 41.0)), `MIDI OUT` 1.05 mm, `UART` 0.30 mm and `USB PWR` 0.10 mm left, and `< REAR`
  1.70 mm left with H1/H2, its arrow tip still in line with them. The references of C22, C23, C31, C32, C33 and RV1
  moved.
* v1.3.1's U12 pin-1 dot, left behind at (201.5, 89.7) next to F1 when U12 moved, is removed.
* Silkscreen DRC (with silk_over_copper/silk_overlap enabled): 45 silk_over_copper, all from the outlines of the
  decoupling discs in the logic column crossing the neighbouring DIP pads (`jlcpcb.sh` plots the silkscreen with the
  mask openings subtracted, so none of it prints on a pad); 3 silk_overlap (the IRQ/DMA divider line over J1 twice,
  as on v1.3.1, and `UART` over J6's outline); 19 silk_edge_clearance (J8, J5, RV1, J11 and the Pico's outline at
  the bracket end).

### Other

* Title blocks say v1.3.2 (root sheet and board 2026-10-28, the other sheets 2026-10-23). The schematic notes about
  the GY module and the SMD parts are updated, and `PicoGUS-THT-schematic.pdf` is plotted from the current schematic.
* No reference designator changed and none is new; J10 is the only one removed. The design rules and net classes
  are v1.3.1's (the `.kicad_pro` is unchanged).
* The schematic fields of C8/C10 name the Murata discs again (as on v1.3.1), and U1's footprint field is
  `PiGUS library:RPi_Pico_SMD_TH_USB_NoSWD`.
* 32 footprints (the wavetable sheet set, FB3, RV1, U11) still carried symbol links (KIID paths) from before the
  v1.3.0 schematic rebuild; DRC parity had only matched them by reference. They are re-linked to their symbols, so
  "Update PCB from schematic" keeps them.

## Layout status

The 102 mm placement is routed on two layers (2026-10-30) and was reviewed twice, each time from an analog/power
and a digital/ISA/USB/manufacturing side, against the routed 103.7 mm layout of commit 8dbb6c6 with the same
scripts. The comparison below is with that layout; its README (`git show 8dbb6c6:hw-tht-v1.3.2/README.md`) has
the comparison of that layout with v1.3.1.

### Routing

* Routed with a grid router (negotiated-congestion rip-up and re-route, as for 8dbb6c6), then
  polished: digital, I2S and USB nets kept 1 mm away from the audio-class and DAC-output copper, a second rip-up
  pass for the address and control nets, USB D+/D− re-routed as flow-through chains through U12, power widened up
  to 0.8 mm where it fits, GND poured on both layers with stitching vias. The review fixes are listed below.
* Design rules unchanged (the `.kicad_pro` is v1.3.1's): 0.2 mm clearance, 0.3 mm copper to edge, 0.25 mm default
  track and 0.7/0.35 mm vias, `pwr` 0.4 mm and `GND` 0.5 mm with 0.8/0.4 mm vias, `audio` with 0.6 mm clearance.
  19 segments (13.5 mm) are narrower than their class, each where the class width does not fit: the centred 0.2 mm
  necks of +5V, +3V3 and ±12V between Pico and U2 pins (0.32 mm from the Pico pads) and a few short +5V, VSYS and
  −12V jogs at pads (8dbb6c6: 2.9 m of track under class width).
* 5503 mm of track and 445 vias: 194 GND, 32 power, 219 signal (8dbb6c6: 5925 mm, 420 vias, 145 GND).
* DRC with schematic parity: 0 unconnected items, and as errors only the 12 accepted courtyard overlaps below, so
  `jlcpcb.sh` needs `FORCE=1`. Warnings: 3 `lib_footprint_mismatch` (J3, J11, RV1; expected, see
  [Kept from v1.3.1](#kept-from-v131)), 19 `silk_edge_clearance`, 6 `starved_thermal` (one spoke instead of two
  at the Pico's GND pin 8 on both layers and on its castellation pad, at the castellation pads of pins 3 and 28, and
  at C5.2, whose one F.Cu spoke joins only an isolated piece of the front pour; pin 8 also has a B.Cu track, the
  others are joined through their holes on B.Cu) and 83
  schematic-parity field warnings (footprints without the symbols' Manufacturer/MPN fields, J3's BOM flag).
* **Courtyard overlaps (accepted), all in the logic column**: each decoupling disc's courtyard overlaps those of the
  DIPs above and below it by 0.33–0.34 mm (C21/U10, C6/U10, C6/U7, C9/U7, C9/U6, C7/U6, C7/U5, C4/U5, C4/U2), and
  the vertical resistors' overlap end to end by 0.20 mm (R18/R5, R5/R6, R6/R17). The disc bodies (F.Fab) stay about
  1.5 mm from the DIP bodies and 0.11 mm (seen from above) from the DIP pads; the discs' pads are 0.61 mm from the
  DIP pads. The resistors' pads are 0.29 mm apart end to end (GND/RESET, GND/+5V, DACK/+3V3): solder those with
  care.

  ![The 12 courtyard overlaps: 3D render, courtyard map with each overlap's depth, and close-ups of a disc between two
  DIPs and of two resistors end to end](docs/figures/courtyard-overlaps.png)

### Review fixes

First review round:

* **DAC analog half kept clear.** I2S, ~RI/OCHRDY, ~RIOR and I/OCHRDY ran through U9's charge-pump and output-filter
  block and took the front ground away under it. They are re-routed around it: no digital track or via is within 1 mm
  of U9 pins 1–10, the CAPP/CAPM/VNEG/AVDD copper or any audio net (8dbb6c6: LRCK 3.6 mm next to U9's analog pins),
  and the pour covers 63 % (front) / 70 % (back) of the DAC's analog half again (8dbb6c6 61 / 73 %). Digital pads are
  nearer, as set by the placement: the MIDI-out parts R3/FB1 sit 0.62–0.90 mm from the AUDIO_R and R12-Pad1 tracks,
  and R16's WT_MIDI pad 0.84 mm from WT_R (8dbb6c6: R3 0.68 mm from AUDIO_R, R16 0.94 mm from WT_L).
* **FB3 (AVDD filter) has its own +3V3 feed** from the Pico's 3V3 pin: no 74xx logic or DAC-digital current
  shares its path, but U8 (PSRAM) and its decoupling still share part of it (8dbb6c6: 6 mΩ shared with logic, 49 mΩ
  with U8; now 0 and 35 mΩ of FB3's 54.5 mΩ).
* **±12V at least 0.30 mm from every non-GND THT pad** on the solder side (it passed D1.2, R9.2, FB1/FB2, R4 and
  C24 at 0.22–0.27 mm), and GND vias tie the back pour under the Pico and the front pour inside U11.
* USB D+ no longer makes a stacked U-turn at U12, and no via sits within 0.10 mm of an SMD pad; VBUS_USB enters
  U12 pin 5 on a 0.4 mm neck; redundant vias on THT pads and dead-end stubs inside pads removed; segments widened
  to their class width wherever it fits; narrow GND pockets and most vias in silk text removed.

Final review round:

* **Gold fingers.** Each finger has its own mask opening the size of the finger (from y 128.97, see
  [Kept from v1.3.1](#kept-from-v131)), so a track beside a finger top is covered only by the mask between them.
  +12V passed 0.22 / 0.23 mm from J3.8 (B8, 0WS on AT boards) and J3.7 (−12V), −12V 0.24 mm from J3.6 (DRQ2) and
  +5V 0.21 mm from J3.30 (OSC). They are now 0.36 / 0.42 / 0.34 / 0.31 mm away (+12V moved 0.2 mm up, the last
  millimetres of −12V/+12V/+5V necked to 0.6 mm). The wide finger entries no longer stick out of the finger
  outline into the opening between the fingers (J3.10's GND entry keeps its 1.0 mm width, bent inside the finger).
* **Mixer ground to the line-out sleeve.** The output of U11's B half (U11.7 → R22) ran on the solder side between
  U11 and J8 and cut the back pour between the mixer's reference pins and J8's sleeve. It now runs on the component
  side above WT_L, and the B half's inverting input (U11.6 → R20) passes just east of R22/R21's pad 1 (x 192, under
  their bodies) as on 8dbb6c6. The A half's output run (C30 → R21) on the solder side moved from y 55.4 (between U11
  and the resistor row) to y 58.6, across the R19–R24 resistor row, as on 8dbb6c6. Two GND vias were added and one
  moved. The resistance between U11.3 and J8's sleeve dropped by 35–38 % (to 1.24 mΩ from 2.01 in the DC model of the
  table; three different ground models agree on the drop, and put it within ±5 % of 8dbb6c6's, which is as close as
  the models agree with each other). At the line-out jack, the ground error from a loop to the fingers fell from
  1054 / 1229 to 285 / 465 µV/A (worst channel, without a Waveblaster / with one at full volume; see the table).
* **USB D+/D− at least 0.30 mm from C35.1** (VBUS_USB) on the solder side (0.21 mm at two corners before).
* The ~RI/OCHRDY via beside C20 moved 0.25 mm: no via is closer than 0.30 mm to an other-net pad.
* **USB D+/D− at least 0.33 mm from the Pico's NPTH peg hole** at (213.85, 106.47) on the solder side: D− passed it
  at 0.26 mm and D+ at 0.27 mm, just above the 0.254 mm hole-clearance rule. D− now runs 0.1 mm lower and D+'s loop
  over the hole 0.1 mm wider; all copper is at least 0.30 mm from every NPTH hole, slot and the outline.
* **The front pour ends 0.30 mm from the Pico's TP1 (GND) pad**: the TP1–TP3 rule area (`revb-pico-4`) now starts
  0.2 mm further west (x 212.83), so a pour stub no longer stops 0.10 mm from the pad. (The DC model of the table had
  counted that gap as a connection; with the Pico's GND pins split, its resistances to the fingers now read 0.03 mΩ
  higher; its other results did not change.)
* A 1 nm gap between two overlapping −12V segments at J9.22 is closed (it was harmless).
* The JLCPCB package in `build/jlcpcb/` (not in git: the repository ignores `build/`) and this README are made from
  the routed board (the package left in `build/jlcpcb/` before was the export of the 103.7 mm layout of 8dbb6c6).

### Compared with the 103.7 mm layout (8dbb6c6)

| | 8dbb6c6 (103.7 mm) | v1.3.2 (102 mm) |
|---|---|---|
| DRC errors / unconnected (schematic parity) | 0 / 0 | 12 (the accepted courtyard overlaps) / 0 |
| starved_thermal warnings | 2 | 6 |
| Track / vias (GND / power / signal) | 5925 mm / 420 (145 / 49 / 226) | 5503 mm / 445 (194 / 32 / 219) |
| ISA D0–D7 length / vias | 442 mm / 52 | 283 mm / 21 |
| I2S length / vias | 236 mm / 1 | 261 mm / 6 |
| USB D+ / D− length | 53.5 / 51.8 mm | 51.3 / 43.8 mm |
| +12V / −12V length (vias) | 187 (9) / 185 (4) mm | 125 (0) / 159 (5) mm |
| Digital tracks and vias within 1 mm of audio copper / of U9 pins 1–10 | 0.05 / 3.6 mm (LRCK) | none / none |
| Other-net track within 1 mm of audio copper, same layer | 9.5 mm | 11.4 mm (all power) |
| AUDIO_L over AUDIO_R on opposite layers | 21.6 mm | 7.5 mm |
| GND resistance to the fingers: U11.3 / U9 AGND / J8 sleeve / J9.1, Pico GND pins tied | 2.62 / 2.59 / 2.46 / 2.95 mΩ | 2.37 / 2.07 / 2.29 / 2.49 mΩ |
| same, Pico GND pins split | 3.21 / 3.27 / 3.09 / 3.46 mΩ | 3.59 / 3.37 / 3.56 / 3.65 mΩ |
| DAC-to-mixer ground offset U9.9 − U11.3 (Pico 100 mA / USB 500 mA / WT 150 mA) | +3.9 / −21.1 / −39.4 µV | +0.7 / −21.6 / −29.2 µV |
| Waveblaster ground J9.25 − U11.3 (WT 150 mA) | +147 µV | +88 µV |
| Line-out ground error per ampere of loop current from J8's sleeve to the fingers, worst channel: no Waveblaster / Waveblaster at full volume | 410 / 715 µV/A | 285 / 465 µV/A |
| same, loop current to the bracket (H1/H2) | 185 / 166 µV/A | 313 / 222 µV/A |
| Line-out ground error at 500 mA of USB-host load, worst channel: no Waveblaster / at full volume | 226 / 314 µV | 128 / 133 µV |
| +5V resistance, fingers to F1 (USB host) / to J9's +5V pins | 80.6 / 96–107 mΩ | 47.3 / 96–101 mΩ |
| +3V3 path to FB3 (AVDD) shared with logic / with U8 | 6.1 / 48.8 mΩ | 0 / 35.0 mΩ |
| ±12V within 0.30 mm of an other-net non-GND pad (both sides, gold fingers included) | 6 (+12V, component side; 11 with GND pads, one on the solder side) | none |
| Power copper within 0.30 mm of a gold finger | 1 (+5V, 0.29 mm from J3.30) | none |
| +5V / +3V3 within 0.30 mm of an other-net THT pad (solder side) or Pico pad (either side) | 28 | 17 |
| Vias < 0.5 / < 1.0 mm from a gold finger | 5 / 16 | 0 / 0 |
| Vias < 0.30 / < 0.25 mm from an other-net pad | 21 / 5 | 0 / 0 |
| Pour more than 5 mm from a GND via | 2047 mm² | 853 mm² |
| DAC decoupling, supply track / ground path: C13 (AVDD), C36 (CPVDD) | 3.2 / 4.2, 2.7 / 10.0 mm | 4.7 / 7.2, 2.7 / 10.0 mm |
| Via holes < 0.10 mm from silk text (inside the ink) | 8 (1) | 6 (4) |
| Track joints inside a via but off its centre / tee joints | 11 / 0 | 46 / 12 |

Both columns come from the same scripts. Resistances, offsets and line-out ground errors are from a DC copper model
of the refilled pours (0.1 mm grid; "tied" treats the Pico's GND pins as one node through the module's ground plane,
"split" leaves them separate; the line-out rows are "tied", "split" ranks the layouts the same way). The offsets are
the ground-voltage difference between the two pins for the named load current. The line-out ground error is what the
ground drops add to the signal at the jack, measured against its sleeve; for the inverting mixer
`e = (Vref − Vsleeve) + 0.977·(Vref − V(U9.9)) + 2·(Vref − Vwt)`, where Vref is U11's + input (pin 3 left, pin 5
right), 0.977 the PCM gain (R19/(R7+R10)) and 2 the Waveblaster gain (R19/R12). With a Waveblaster at full volume Vwt
is J9's ground; without one, the worst case is RV1 turned down, where RV1's ground end reaches the summing node
through R12 alone. Loop current enters at J8's sleeve (from a cable shield or a grounded amplifier) and leaves at the
fingers or at the bracket. An independent 0.1 mm raster solver gives the same ranking (loop to the fingers 347 / 481
against 425 / 740 µV/A, to the bracket 311 / 228 against 191 / 175 µV/A, USB 142 / 140 against 230 / 323 µV). The
model differs from the one behind 8dbb6c6's README, which gave that layout 3.02 / 3.07 / 2.95 / 3.26 mΩ; compare
within this table only.

**Better** than 8dbb6c6: the ISA data bus, digital-to-audio and DAC isolation, AUDIO_L/R stacking, the line-out
ground error from a ground loop to the fingers and from USB load, ±12V and power clearances (pads and fingers), vias
near the fingers and pads, the USB-host +5V feed, the FB3 feed, the GND resistances with the Pico's GND pins tied,
the DAC and Waveblaster ground offsets under Waveblaster load, stitching, track widths. **Worse**: I2S length and
vias, the GND resistances if the Pico's GND pins are not tied inside the module, the line-out ground error from a
ground loop to the bracket, the B10 GND finger track (0.6 / 0.5 mm for about 10 mm, with a via, before it reaches a
pour; 8dbb6c6 1.0 mm), the power copper next to audio copper (11.4 against 9.5 mm), digital over audio on the other
layer, the C13 supply and ground paths (placement), the starved thermals, and the cosmetic points below.

### Known weaker points (accepted)

* **Line-out ground loop to the bracket**: per ampere of loop current from J8's sleeve to the bracket (H1/H2), the
  ground error at the jack is 313 / 222 µV (worst channel, without a Waveblaster / with one at full volume; 8dbb6c6
  185 / 166 µV); a loop to the fingers gives less than on 8dbb6c6 (285 / 465 against 410 / 715 µV/A). With 10 mA of
  loop current and no Waveblaster that is at most 3.1 µV of hum through the bracket and 2.8 / 1.1 µV (left / right)
  through the fingers (8dbb6c6 1.8 and 4.1 / 1.3 µV), below −100 dBV and under the DAC's noise floor (formula below
  the table).
* **USB load, right channel**: 500 mA of USB-host load gives −101 µV at the right channel's jack without a
  Waveblaster (−29 µV before the final review's mixer re-route, 8dbb6c6 −170 µV); the worst channel improved (128 µV
  against 214 before, 8dbb6c6 226).
* **Pico GND pins split**: if the Pico's GND pins are not tied through the module, the resistances to the fingers are
  0.10–0.47 mΩ above 8dbb6c6 (U11.3 3.59, J8 sleeve 3.56 mΩ).
* **I2S**: 261 mm with 6 vias (8dbb6c6 236 mm, 1 via): the nets go round the DAC's analog half instead of across
  it. None crosses an audio net.
* **DAC decoupling**: C13 (AVDD) has 4.7 mm of supply track and a 7.2 mm ground path to AGND (8dbb6c6 3.2 /
  4.2 mm), C36 2.7 / 10.0 mm; both are set by the placement (see [Placement suggestions](#placement-suggestions)).
* **Router traces**: at 46 points (44 vias, 94 track ends) tracks join a via off its centre (inside the via; 8dbb6c6
  11 points), 12 tracks join another track part-way (tees), and 30 finger-entry segments end inside the finger short
  of its centre. All are sound copper and pass DRC.
* **Silk**: 4 signal via holes are inside reference-designator ink (C11, C7, U4, C2) and 2 more are within 0.10 mm
  of text (U8's reference, the jumper label `3`); none had a free spot within reach.
* **−12V on F.Cu under the Pico** (x 190–197, y 112–116): under the mask and the module, at least 0.73 mm from the
  castellation pads.
* **Digital over audio on the other layer**: 5.0 mm, all perpendicular crossings (WT_MIDI twice, R3's net)
  (8dbb6c6 3.3 mm).

### Placement suggestions

Not done: each one needs parts moved.

* **F1 back west of U12**, as on v1.3.1: one straight, centred B.Cu +5V run could then cross both Pico rows (each
  row crossing still needs a centred 0.2 mm neck). The D1 → F1 feed is 47.3 mΩ from the fingers to F1 now
  (8dbb6c6 80.5 mΩ, v1.3.1 40 mΩ).
* **U12 back next to J11**: shorter USB D+/D− (51.3 / 43.8 mm now, 42.2 / 42.5 on v1.3.1; irrelevant at
  12 Mbit/s).
* **C36 horizontal right under U9 pins 1–3** (pads at (146.0, 81.0) / (151.0, 81.0)), with C16/C17 one row down: both
  CPVDD legs 2.7 mm (pin 1 → C36, C36 → CPGND pin 3; now 2.7 / 10.0 mm).
* **C1 0.27 mm right** (back to x 173.5): 0.68 mm instead of about 0.4 mm between its can (nominal size) and a
  full-size Waveblaster daughterboard's maximum outline.
* C19/C20 nearer U9 pins 6–9 would shorten their ground return to AGND (pin 9; 21.1 / 22.4 mm of copper now, as on
  8dbb6c6). The ground model shows no penalty, so this is not worth a revision on its own.

### Before ordering and building

* **Order the adapters first**: PA0035C stock is thin (1 at DigiKey on 2026-10-23). The own adapter PCBs are the
  fallback (see [Adapter boards](#adapter-boards)).
* **Bracket**: get or print a bracket whose tabs are 1.70 mm longer than a Keystone 9202's (see [Bracket](#bracket)).
* In JLCPCB's placement preview, check U8's pin 1 (U8 is at 0° now, v1.3.1 had −90°) and U12's pin 1.
* **Solder J11 before the Pico** and check the gap between its shield pad and Pico pin 4 (SPI_SCK, 0.80 mm) for a
  bridge (see [Kept from v1.3.1](#kept-from-v131)).

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
  it. The Adafruit short sockets keep the Pico at ~7.5 mm; the standard 8.5 mm Sullins sockets at ~11–12 mm. The
  Pico's far end (x 167.38) is 2.9 mm under a full-size daughterboard's outline (x 170.32), so with a daughterboard
  fitted use the short sockets.

## Kept from v1.3.1

* **RV1 / J8**: J8 on top, RV1 below it (at the top, RV1's M7 bushing would have been 0.4 mm above the PC's slot
  opening). **J11 at x 208.64** (at x 210 its flange stood 1.26 mm into the bracket plate). **C32** (47 µF, 11 mm
  tall) out from under the daughterboard at (180.4, 39.4). C1's can is about 0.4 mm from a full-size
  daughterboard's maximum outline at nominal size, 0.16 mm at maximum can size (v1.3.1: 0.68 / 0.43 mm; C1 moved
  0.27 mm with the Pico row).
* No copper inside the gold-finger area: a no-fill rule area covers the strip above the fingers, and **J3's mask
  opening between the fingers starts 0.63 mm lower (y 129.6) than the finger tops**. Each finger also has its own
  mask opening, the size of the finger (from y 128.97, no mask expansion), so a track beside a finger top is
  covered only by the mask between them. Keep power copper at least 0.30 mm from the finger copper (this layout:
  0.31 mm, +5V next to B30 (OSC); ±12V 0.34 mm or more). The ISA GND fingers B1, B10 and B31 each have a back-side
  track into the pour, but only B1's is 1.0 mm all the way. B10's and B31's are 1.0 mm only inside the finger: B31's
  leaves it at 0.5 mm next to a +5V via and reaches the back pour 0.5 mm further on; B10's leaves it at 0.6 mm next
  to the +12V finger, runs 4.7 mm to a via in the no-fill strip and then about 5 mm at 0.5 mm on the front before it
  reaches the front pour (8dbb6c6: 1.0 mm tracks from all three).
* **Don't run "Update footprint from library"** on J3 (the mask edit), J11 (the shield padstack below) or RV1 (the
  library footprint has a stray Edge.Cuts line that breaks the outline). DRC lists these three as
  `lib_footprint_mismatch` warnings; that is expected. J5, J8, RV1 and F1 also carry 3D models from `hw-common/parts`
  in place of stock model paths that KiCad 10's library does not have: updating them from the library puts the
  missing paths back. The two Pico micro-USB shell relief slots (1.3 × 2.0 mm) are
  drawn on Edge.Cuts as locked board items (JLCPCB wants non-plated slots on the outline layer), and five rule
  areas keep vias and the front pour away from the Pico's test pads. On v1.3.2 they moved with U1 (1.62 mm left in
  all: 0.27 mm, then 1.35 mm for the 102 mm trim); **move them with U1 again if U1 ever moves**.
* J11's shield pad next to the Pico's top pin row has a 2.6 mm pad on the front (3.0 mm on the back). With the Pico
  1.35 mm further in, it faces Pico pin 3 (GND): 0.48 mm on the front (2.08 mm to the pin's THT pad on the back), so
  a bridge there is harmless. Pin 4 (SPI_SCK), which faced it at 0.48 mm on v1.3.1 and the routed layout, is now
  0.80 mm away. **Solder J11 before the Pico and check the gap to pin 4 for a bridge.** H2's mask opening is 0.3 mm
  smaller (it sits 0.38 mm from Pico pin 39).
* Rule areas keep copper tracks out of the front exposed-copper logo, and vias and the front GND pour out from under
  the flat Pico's test pads (TP4–TP6, Pico 2 TP7) and its TP1–TP3 USB area.
* U12's GND (pin 2) has a GND via 1.26 mm from the pin (8dbb6c6: 1.25 mm); keep one there.
* A `JLCJLCJLCJLC` order-number placeholder sits on B.Fab next to the Pico (not plotted). On 8dbb6c6 three via holes
  touched the bottom edge of its box; on this layout none does (the nearest, a +5V via, is 0.15 mm below it). If an
  order ever uses "Specify a location", check the box for via holes again after any routing change.

## Unchanged from v1.3.1

### USB host port (J11, sheet 4 "USB host port")

A USB-A host port at the bracket for a mouse, joystick, keyboard, flash drive (CD-ROM images) or a hub, as on the
PicoGUS NE2000 card and the PicoMEM 2.

* **Data**: straight from the Pico's own USB D+/D− test points **TP3 / TP2** (behind the Pico's 27 Ω series
  resistors). U1 uses `RPi_Pico_SMD_TH_USB_NoSWD`: the v1.3.0 Pico footprint plus three plated holes under TP1
  (GND), TP2 (D−) and TP3 (D+), without the SMD pads at the SWD end (v1.3.1: `RPi_Pico_SMD_TH_USB`, with them). The
  Pico must be **soldered flat** (castellated edge or through-hole pins, no header) so
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
* **U12** (USBLC6-2SC6, SOT-23-6): optional ESD protection on D+/D−, routed flow-through as on v1.3.1: D+ J11 →
  pin 6 → under the body → pin 1, D− pin 4 → pin 3, on to the Pico, so there are no stubs and the port works without
  U12. On v1.3.2 it sits next to U8, about 10 mm west of J11's pins (F1 between), with C35 (0.1 µF) just below it.
  Pin 1 is marked by the footprint's silk triangle just above it (v1.3.1's hand-placed dot is removed). **If you fit
  U12, solder it before C35**, 0.36 mm below it.
* USB-C was considered and left out: the PicoMEM 2 also uses USB-A for its host port and USB-C only for
  programming (5.1k Rd, never sourcing VBUS). A dual USB-A/USB-C footprint is in the shared library, unused.

### Waveblaster header without standoff holes

J9 uses `PiGUS library:Wavetable_NoMountingHoles`: the v1.3.0 `Wavetable` footprint without the two 3.2 mm
daughterboard standoff holes (the far one landed in U4 in the rev B placement). The daughterboard is held by the
2x13 header only, so there is no M3 standoff and no screws in the BOM. A full-size daughterboard overhangs the
card's far end; support it by hand when plugging/unplugging cables.

### Bracket

The bracket parts are where v1.3.1 had them, not where v1.3.0 (and v1.2) had them. **The tab holes H1/H2 are
1.70 mm further from the plate than on a Keystone 9202**: their centres are 7.03 mm behind the plate's inner face
instead of 5.33 mm (measured on the board's bracket drawing on User.Drawings), because the card is 1.84 mm shorter
at the bracket end (see [102 mm board](#102-mm-board)). A stock Keystone 9202, a PicoGUS 2.0 bracket and the brackets
for v1.2, v1.3.0, v1.3.1 and the routed v1.3.2 layout therefore do not fit: the card needs a bracket with tabs
1.70 mm longer, e.g. a 3D-printed one (the bracket drawing on User.Drawings has them). The plate itself sits where
it always does in the case, so the openings do not change. Hole positions (distances measured down from the
bracket's top surface, height = centre above the card's component side):

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
  put its front lip into the plate. A PicoGUS 2.0 bracket does not fit this card: its cut-outs are elsewhere, and
  its tab holes are 0.94 mm, not 1.70 mm, further in.
* **J8 and J5 moved in** with the trim (1.10 / 0.90 mm), so their noses now end 0.31 mm behind the plate's outer
  face (on the bracket drawing's 0.78 mm plate, x 221.73–222.51; v1.3.1: 0.79 / 0.58 mm past it). A
  plug whose body is wider than the hole then stops on the plate about 0.3 mm short of the jack. A counterbore of
  about 0.4–0.5 mm around those two holes on the outside of the plate lets it seat fully; on a thicker (3D-printed)
  plate the noses end further behind its outer face, so size the counterbore for that plate.
* J8 and RV1 are 8.9 mm apart (v1.3.0: 9.7 mm between J5 and RV1): with a Ø8 mm plug in J8, use a knob of
  Ø9.5 mm or less on RV1.


# v1.3.0 reference (unchanged in rev B and v1.3.2 unless noted above)

## What changed in v1.3.0 compared to v1.2

### Through-hole parts

| Part | v1.2 | THT board |
|---|---|---|
| Resistors | 0805 | 1/4 W axial, 10.16 mm hole pitch |
| Ceramic caps | 0805 / 1206 | radial MLCC / disc, **5.0 mm** lead pitch |
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
* **U8** APS6404L PSRAM: SOIC-8 (1.27 mm pitch), soldered to the card.
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
* **DAC analogue supply.** On v1.2 the PCM510x AVDD (pin 8) shares the Pico's switch-mode 3.3 V with all the logic.
  Here AVDD has its own rail behind ferrite bead FB3 (the same part as FB1/FB2), with C13 (0.1 µF) and C15 (10 µF) at
  the pin. DVDD and CPVDD stay on the plain 3.3 V, as on v1.2/v1.3.0. Since rev B the charge-pump supply CPVDD (pin 1)
  has its own 0.1 µF, C36, so its current loop pin 1 → C36 → pin 3 (CPGND) is short: 1.9 + 0.6 mm on rev B; on v1.3.2,
  a THT disc just below DIP-20 pin 1 (pad 1 2.7 mm from the pin; the routed layout, commit 8dbb6c6, had 2.7 mm of
  track and 8.2 mm of pour back to pin 3; 10.0 mm by the script of [Layout status](#layout-status), as on this
  layout) (before rev B it was
  about 24 mm of track to C4 and back through the pour). TI's layout guide and PicoGUS 2.0 put this cap right at the
  pin. CPVDD stays on +3V3, as on v1.2/v1.3.0, and TI's 10 µF bulk cap at CPVDD is not fitted locally (the nearest
  bulk is C1). The bead drops only millivolts at the DAC's ~10 mA.
* **Capacitor types in the signal path:** C19/C20 (DAC output filter) and C30/C31 (op-amp feedback) are
  **C0G/NP0**, which has no voltage or temperature coefficient and so adds no distortion. X7R is used only for
  supply decoupling.

v1.3.0 and v1.3.1 also had a footprint for a GY-PCM5102 DAC module (J10) as an alternative to the on-board DAC;
**v1.3.2 has no module option** (see above).

### Back silkscreen

The back carries the princess artwork (hw-common/art/mila-alisa.svg: a big and a small princess, both crowned, with
sparkles and a castle, over the PicoGUS logo): on rev B 71.6 × 37.0 mm, its lower edge 3 mm above the Pico's pin
holes, 6 mm off the card's centre toward the bracket. The back is full of through-hole pads, so the artwork is cut
out 0.3 mm around every pad and hole (silkscreen never lands on copper). **v1.3.2 has no artwork on the board**: it
is to be UV printed on the back instead (the routed v1.3.2 layout, commit 8dbb6c6, had it re-placed at 52.8 ×
27.0 mm; see [Silkscreen and back artwork](#silkscreen-and-back-artwork)).

The version and licence block sits between the Pico's pin rows, one text per line, ending with the thanks and the
dedication: *Thanks to polpo and all the PicoGUS contributors. This revision is dedicated to my wonderful family, Mila
and Alisa.* On v1.3.1 each line was nudged (at most 0.9 mm) so that no via hole lands in a letter; on the routed
v1.3.2 layout the lines were re-spaced so that no via hole was within 0.10 mm of a letter. This board keeps those
positions, and its via holes are at least 0.13 mm from every back-side letter (0.18 mm from the text block).
"< REAR" points at the bracket, its arrow tip in line
with the bracket tab holes H1/H2. All are mirrored on B.SilkS so they read correctly from the back.

### Other changes

* The board is **103.7 × 99.9 mm** (v1.3.2: 101.9 mm long, see [102 mm board](#102-mm-board)). It has the same length, ISA edge connector and bracket tab holes (H1/H2) as
  v1.2 (on v1.3.0 the MIDI and line-out jacks are also in the v1.2 places; rev B and v1.3.2 move them, see
  [Bracket](#bracket); v1.3.2 is also 1.84 mm shorter, with H1/H2 1.70 mm further in, see
  [102 mm board](#102-mm-board)), but it is taller, like PicoGUS 2.0, to fit the Waveblaster header along the top edge.
  1.6 mm FR4 (the v1.2 and v1.3.0 board files' stackup says 0.57 mm; rev B's and v1.3.2's say 1.6 mm, so their 3D
  and STEP exports have the right thickness).
* Design rules are relaxed for THT/DIY: 0.2 mm clearance, 0.25 mm signal / 0.4 mm power tracks
  (+5 V, +3.3 V, VSYS), 0.7/0.35 mm vias (rev B and v1.3.2: the same, plus a 0.6 mm-clearance `audio` class).
  GND is poured on both layers (v1.3.2 too, with stitching vias; see [Layout status](#layout-status)).
* **J2 (the 2x31 ISA debug/probe header of v1.2, never fitted) is removed.** On a through-hole board
  its pads formed a wall right above the gold fingers that every ISA signal had to cross.
* The `+3.3V` power symbols are renamed `+3V3`. KiCad 8+ names power nets after the symbol value, so the
  upgraded v1.2 sheets would otherwise have split the 3.3 V rail into two nets.
* The symbol fields of U3/U4 now name the CB3T3257 that is actually used (the v1.2 fields still said 74FST3257DR2G).
* **Layout is grouped by circuit block.** Each chip and its decoupling caps and resistors form a KiCad group,
  so a block can be selected and moved as one: DAC (U9), Mixer + line out (U11, J8), WT volume (RV1),
  Waveblaster header (J9), MIDI out (J5), Pico + power (U1), ISA bus switches (U3, U4), ISA buffer (U2),
  74AHC14 (U5), 74AHC00 (U6, U7), 74AHCT126 (U10), PSRAM (U8, C8, C10), IRQ/DMA jumpers (J1).
  On v1.3.2, U2, U6 and U7 (with C4, C6, C9) are not grouped.
* **Layout for audio quality:** every decoupling cap sits next to its chip's supply pin. FB3 sits next to C13/C15.
  The ±12 V filter (R23/R24, C32/C33) sits above U11. The DAC is turned so that its outputs, the output RC filter
  (R7/R8, C19/C20) and the mixer op-amp form one short path to the line-out jack, away from the Pico and the ISA
  logic (v1.3.2: the DIP-20 at 90°, pins 1–10 in its bottom row with R7/R8 and C19/C20 below it).

## Card dimensions (ISA 8-bit)

The card is **101.9 mm long × 99.85 mm tall** (bottom of the fingers to the top edge, at the outline's line
centres; 102.0 × 99.95 mm over its 0.1 mm lines). v1.3.1 and the routed v1.3.2 layout were 103.7 mm long: v1.3.2 is
trimmed 1.84 mm at the bracket end (see [102 mm board](#102-mm-board)). The IBM PC/XT full-height limit is 4.2 in
(106.7 mm), so the card is 6.8 mm under it; the AT limit is 4.8 in (121.9 mm). The edge connector (62 contacts,
0.1 in pitch) and its position are unchanged from v1.2; the bracket tab holes H1/H2 are 1.70 mm further in than on
v1.2 (see [Bracket](#bracket)). The board is 1.6 mm, the standard ISA card thickness. A Waveblaster daughterboard
sits about 11 mm above the card, inside the 0.8 in (20.3 mm) AT slot pitch. Clip the THT leads short (≤ 2 mm) on the
solder side, where the next card sits. There are no SMD parts on the back.

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
* **SMD parts on the card**: U8 (SOIC-8) and the optional U12 (SOT-23-6), both on the front. Solder them before
  the through-hole parts.
* **DigiKey BOM cost**: the v1.3.0/v1.3.1 estimate (2026-09-28, parts only, no PCB, no shipping, from DigiKey's price
  breaks) was ~474 kr excl. MVA (~$50) for one board and ~356 kr (~$37) per board in a 10-board order (NOK at 9.53
  NOK/USD; digikey.no can differ by a few percent). **v1.3.2 adds the three Chip Quik adapters, USD 22.17 (~211 kr
  excl. MVA)** per card at DigiKey; with the own adapter PCBs it is about USD 14 of pin strips per card plus the
  PCBs. C8/C10 are the v1.3.1 discs again; the disc C36 (an 0603 on v1.3.1) changes the total by less than 10 kr. The
  optional machined socket set was ~80 kr on rev B, plus ~21 kr for the Pico sockets; v1.3.2 adds two DIP-16 sockets
  and a second DIP-20.
* **Ceramic capacitors must have 5.0 mm (or 5.08 mm) lead pitch.** 2.5 mm parts don't fit (e.g. TDK FG1x, Vishay K…L2).

Mistakes found in the v1.2 BOM/JLCPCB data (don't copy these from the v1.2 files):
C139966 is a Vilsion part, not the APS6404L. C2910528 is a 3.5 mm jack with a different pinout.
C2905423 is a female header, not the Pico. C2935925 is DEALON, not Amphenol. C6942 is the SOIC SN74AHC14DR.

## JLCPCB files

```
FORCE=1 hw-common/tools/jlcpcb.sh hw-tht-v1.3.2/PicoGUS-THT.kicad_pcb   # -> hw-tht-v1.3.2/build/jlcpcb/
```

The script stops if DRC (including the schematic-parity check) finds errors. On this board DRC finds only the 12
accepted courtyard overlaps (see [Layout status](#layout-status)), which are errors in this project's rules, so
export with `FORCE=1` after checking that the report lists nothing else. `build/jlcpcb/` holds the export of the
routed board made that way (2026-10-30; not in git: the repository ignores `build/` and `*.zip`, so a clone has to
run the script): outline to x 218.29 (102.0 mm), 445 vias, 831 plated holes. It writes:

* `PicoGUS-THT-gerbers.zip`: upload this to jlcpcb.com. It holds the Gerbers (Protel extensions, no X2)
  and the Excellon drill files (PTH and NPTH separate, mm).
* `PicoGUS-THT-bom-jlc.csv` and `PicoGUS-THT-cpl-jlc.csv`: the BOM and placement files for JLCPCB assembly. The
  placement file has the SMD parts only: **U8 and U12 on Top (rotation 0)**.
  The rotations are KiCad's, with no JLCPCB offset table; U8 was at −90 on v1.3.1, so a correction worked out for
  v1.3.1's U8 does not carry over. U3/U4/U9 are DIP footprints
  now, so they are not in it (the TSSOPs go on the adapters; see [Adapter boards](#adapter-boards) for their own
  JLCPCB files).
* `PicoGUS-THT-drc.rpt`: the DRC report.

The **Hardware (JLCPCB files)** GitHub Action runs the same script for `hw-tht` (v1.3.0) and `hw-smd`. v1.3.1 and
v1.3.2 are not in it.

Order options: 2 layers, **1.6 mm**, **Surface finish: ENIG** and **Gold fingers: Yes** with the **45° finger
chamfer** (the ISA edge connector). ENIG is required: JLCPCB only gold-plates the fingers on ENIG boards; with HASL
the fingers are tinned, and tin wears and oxidises in an ISA slot. The fingers start 0.58–0.60 mm above the card
edge, as on v1.2, so JLCPCB's bevel (about 0.74 mm) takes about 0.15 mm off their tips. Check the rotations in
JLCPCB's placement preview before paying for assembly (U8's and U12's pin 1). JLCPCB no
longer prints an order number by default.
The `JLCJLCJLCJLC` placeholder sits on B.Fab next to the Pico (not plotted): if an order ever needs "Specify a
location", move it to B.Silkscreen first, or JLCPCB prints the placeholder text itself.

## Caveats

1. **Not built or tested.** v1.3.2 is routed and reviewed (see [Layout status](#layout-status)). The circuit is the
   proven v1.2 one with the v1.3.x additions, but no v1.3.x THT board has been fabricated or run in a PC yet.
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
   and the adapters, axial resistors and FB3's axial bead, radial MLCCs, the J1 jumper block as on PicoGUS 2.0).
   Electrolytics and the pot are outside that area; the Pico's far end is about 2.9 mm under it (1.3 mm on v1.3.1),
   so with a daughterboard fitted solder the Pico flat or use the short Adafruit sockets. IC sockets add 4.6–5.1 mm,
   and a socketed own adapter PCB reaches ~10 mm: solder those directly or check the clearance if the daughterboard
   has parts on its underside (see [Socket build](#socket-build) and [Heights](#heights)).
6. **Adapters.** PA0035C stock is thin and its datasheet was not readable; check its pin-1 mark on arrival. Never
   put square header pins into machined sockets. On the own TSSOP-20 adapter, solder DIP pins 5, 6, 15 and 16 with
   care (the chip body overhangs their pads).
7. **RV1.** The BOM uses the Bourns PTD902-2020K-A103 (DigiKey 426, 24 kr). It has the same 2.5 mm
   pin grid and M7 bushing as the Alps RK097 the footprint was drawn for; test-fit one in the bracket
   before a large order. LCSC doesn't list it, so the LCSC column keeps the Alps RK09712200HA
   (C470545). The Alps parts (RK09712200HA: DigiKey 210 / LCSC 175, 45 kr; the switched RK0971221Z0X
   also fits) have low stock, and Alps North America has published a discontinuation notice that
   covers parts of the range.
8. **ISA bracket.** No stock bracket fits: the tabs must be 1.70 mm longer than a Keystone 9202's, and no bracket
   made for v1.2, v1.3.0, v1.3.1 or the routed v1.3.2 layout fits. Cut the four openings in the
   [Bracket](#bracket) table (and counterbore the J8/J5 holes).
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
