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
* **C8/C10** (U8 PSRAM decoupling) are now **0805s on the back**, right under U8.

The main board is through-hole apart from **U8** (APS6404L PSRAM, SOIC-8, 1.27 mm pitch) with its two 0805 caps
C8/C10 on the back, and the optional **U12** (USBLC6-2SC6, SOT-23-6). The TSSOPs are soldered to the adapters, not
to the card. The rest of the circuit is v1.3.1's: USB-A host port, Waveblaster header, op-amp mixer.

> **Status: routed, DRC clean; not built or tested yet.** The schematic's ERC is the same as v1.3.1's (280
> warnings, 0 errors). The board passes DRC with schematic parity (0 errors, 0 unconnected) and
> `hw-common/tools/jlcpcb.sh` exports it. The routed board was reviewed against v1.3.1 from four sides (analog
> and power, digital/ISA/USB, manufacturing, silkscreen); every finding was checked by a second reviewer, and
> the confirmed ones were fixed unless they need parts moved or were accepted (listed under Layout status). A
> final check reproduced the numbers and the gerbers. It is better than v1.3.1 on audio isolation, DAC ground and
> ±12V, and worse on the ISA data bus and the USB-host +5V feed: see [Layout status](#layout-status).

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

The shared `PiGUS library` symbols and footprints are linked from `hw-common/`, like the other boards. **New in the
shared library for v1.3.2:** `DIP-16_W7.62mm_TSSOP-16_Adapter` and `DIP-20_W7.62mm_TSSOP-20_Adapter`, in
`hw-common/PiGUS library.pretty`; DRC's library check finds the board's U3/U4/U9 identical to them. The older
`DIP-16_W7.62mm_TSSOP-16_Nested` stays unused.

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

### C36: THT disc below U9 pin 1

C36 (U9 CPVDD decoupling) is now the board's standard 0.1 µF 50 V X7R radial disc (Murata RDER71H104K0M1H03A,
the 0.1 µF line of the BOM) instead of an 0603: an SMD part under an adapter board makes no sense. It stands
vertically just below DIP-20 pin 1 (CPVDD), its courtyard against the DIP-20's. It cannot go under the adapter: a
soldered adapter sits about 0.45 mm above the board, and a socket body covers the area. Pin 1 → C36 pad 1 is
2.7 mm of track; C36 pad 2 → CPGND (pin 3) is 8.2 mm through the GND pour (v1.3.1, with the 0603 under the TSSOP:
1.9 / 0.6 mm). C16 sits on the straight line to pin 3, so the route is close to the shortest this placement
allows; turning C36 horizontal right under pins 1–3 would cut the ground leg to about 2.7 mm (see
[Placement suggestions](#placement-suggestions)).

### C8/C10: 0805 on the back under U8

| Ref | Part | DigiKey | LCSC |
|---|---|---|---|
| C8 | 0.1 µF 50 V X7R 0805, Yageo CC0805KRX7R9BB104 | 311-1140-1-ND | C49678 (JLCPCB basic) |
| C10 | 1 µF 50 V X7R 0805, Samsung CL21B105KBFNNNE | 478-KGM21AR71H105KUCT-ND (KYOCERA AVX 08055C105KAT2A: the Samsung part, 1276-1029-1-ND, had no DigiKey stock on 2026-10-23) | C28323 (JLCPCB basic) |

Both sit on the back (B.Cu), under U8 (SOIC-8, front, at 0° on v1.3.2: pin 1 top left, pin 8 (+3V3) top right,
pins 3/4/7 GND). They lie horizontally between U8's two pin columns, C8 (0.1 µF) at (189.00, 92.90) between the
pin 1/8 and 2/7 rows, C10 (1 µF) at (189.00, 95.00) between the 3/6 and 4/5 rows. Pad 1 (+3V3) of each is on the
pin-8 side (x 189.95), pad 2 (GND) on the pin-1 side (x 188.05). The +3V3 path from pin 8 to C8 is 3.2 mm and GND
3.3 mm; C10 5.3 / 1.2 mm (v1.3.1, discs on top: C8 1.9 / 5.7 mm, C10 36.2 / 4.4 mm). U8 and FB3 now sit on their
old top-side spots. The JLCPCB placement file lists them on the **Bottom** layer, rotation 180. Their references
are on the back silkscreen beside each cap, clear of the artwork. (LCSC C49678 is the Yageo part, a JLCPCB basic
part; the Samsung CL21B104KBCNNNC is C1711, an extended part, listed as the C8 alternative.)

### Placement

The board holds the final hand arrangement of the unrouted v1.3.2 blank (2026-10-25). Compared with v1.3.1:

* **Logic column at the left edge.** U10, U7, U6 and U5 (DIP-14, 90°, pin 1 at the left as on v1.3.1) are stacked
  at x 128.16, 11.43 mm apart (pin 1 at y 65.77 / 77.19 / 88.62 / 100.05). Their decoupling caps C21, C6, C9 and C7
  stand in one column at x 125.00, in line with C27, and R18, R5, R6 and R17 in one column at x 146.70 at the
  chips' right-hand ends. The column keeps clear of the front exposed-copper PicoGUS logo at the left edge.
* **U2** (DIP-20, 90°) at the lower left, pin 1 (120.07, 113.81), with C4 above it at (120.00, 103.00). **J1** (the
  IRQ/DMA jumper block) at x 120.00, pin 1 (120.00, 123.70), with its labels.
* **U3/U4 swapped**: U3 (D0–D3, A0–A3) is now the left DIP-16, pin 1 (146.88, 106.42), and U4 (D4–D7, A4–A7) the
  right one, pin 1 (158.19, 106.44), both at 0°. C3/C5 sit above each chip's pin 16 (+3V3), R2 at (161.50, 100.00).
  The swap shortens the D-bus pad-to-pad span to 253 mm (v1.3.1: 267 mm). Pins 8/9 (y 124.2) are inside the no-fill
  strip above the fingers.
* **DAC**: U9's DIP-20 lies horizontally (90°), pin 1 (161.00, 78.30) at the bottom left. Pins 1–10 (charge pump,
  outputs, AVDD, AGND) are the bottom row (y 78.30), pins 11–20 (I2S, LDOO, DGND, DVDD) the top row (y 70.68).
  Below the bottom row, from the left: C36 (vertical under pin 1), C16 (CAPP/CAPM) and C17 (VNEG), C19/C20 (x 164.00,
  y 87.50 / 90.50), the output filter R7/R8 (vertical, x 172.40 / 175.60), C13/C15 (AVDD) under pins 8–10, and FB3
  (horizontal, y 88.50) with its +3V3 pad at U8. Above the top row: C14 (+3V3), C12 (DVDD) and C18 (LDOO) at
  x 161 / 164 / 167.
* **USB host corner**: U12 moved left next to U8, to (197.14, 91.45); F1 is vertical at (204.00, 90.00); C35 under
  U12 at (200.00, 95.00); C34 0.8 mm up. U8 is turned to 0° at (189.00, 93.90), with C8/C10 under it on the back.
* **Pico (U1) 0.27 mm left** (pin 1 at (218.36, 100.00)), together with C1, C2, C11, D1, R1, J6 and J7. The Pico's
  test-pad rule areas and its two micro-USB relief slots on Edge.Cuts moved with it.
* Smaller moves: R16 vertical at (156.50, 55.50); C24, C25 and C26 in a row at y 45.50 (x 148.60 / 143.60 / 139.00);
  FB1, FB2, R3, R11, R13 and R20–R22 1.5 mm right; R4 0.36 mm left.

| Part | v1.3.1 | v1.3.2 |
|---|---|---|
| U10 / U7 / U6 / U5 | (126.98, 66.60) / (127.08, 80.90) / (127.08, 95.20) / (126.88, 109.50), 90° | x 128.16, y 65.77 / 77.19 / 88.62 / 100.05, 90° |
| U2 | (151.07, 95.31), 90° | (120.07, 113.81), 90° |
| J1 | (121.53, 122.91) | (120.00, 123.70) |
| U3 / U4 | TSSOP-16 centre (162.81, 114.11) / (149.19, 114.28), 90° | DIP-16 pin 1 (146.88, 106.42) / (158.19, 106.44), 0° |
| U9 | TSSOP-20 centre (153.02, 66.14), 90° | DIP-20 pin 1 (161.00, 78.30), 90° |
| C36 | 0603 (150.40, 70.80) | disc (161.00, 81.00), −90° |
| U8 | (187.00, 91.50), −90° | (189.00, 93.90), 0° |
| C8 / C10 | discs (191.30, 94.50) / (179.54, 94.03) | 0805 on the back (189.00, 92.90) / (189.00, 95.00), 180° |
| U12 | (203.78, 90.88) | (197.14, 91.45) |
| F1 | (192.00, 85.90), 0° | (204.00, 90.00), −90° |
| U1 (Pico) | (194.50, 108.89) | (194.23, 108.89) |
| C4 / R2 | (147.84, 88.81) / (158.86, 99.00) | (120.00, 103.00) / (161.50, 100.00) |

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
* **Princess artwork re-placed for the routed board**: 52.8 × 27.0 mm (v1.3.1: 71.6 × 37.0 mm), on the back above the
  logic and DAC area, its lower edge about 3 mm above the Pico module's edge and 4.1 mm above its pin holes (front
  coordinates x 125.9–178.7, y 68.4–95.4). It is cut 0.3 mm around every pad, hole and via. Faces, eyes and mouths are
  not cut and no via hole lands on the ink; the castle loses 29 % to U6/U7's pad rows and the "PicoGUS" wordmark 14 %
  to U5's. This is the best clean spot of that size in the band above the Pico. A spot in the top band
  (`ART_PLACE=134.40,38.80,53`, above the DAC) keeps 86 % of the ink instead of 79 %, but gives up the position above
  the Pico and needs two stitching vias re-placed.
* The back text block (version, copyright, licence, URL, thanks, dedication) is re-spaced around the vias: no via
  hole within 0.10 mm of a letter.
* C8/C10's references sit on the back beside each cap.
* Front: via holes moved out of the `v1.3.2`, `LINE OUT`, J1, C1 and C2 labels; one front text still has a hole
  in it (U4's reference, under its adapter). Five GND stitching vias line the left edge of the front copper logo
  (x ≈ 122), 0.4–1.1 mm from the letters. v1.3.1's U12 pin-1 dot, left behind at (201.5, 89.7) next to F1 when
  U12 moved, is removed.
* Silkscreen DRC (with silk_over_copper/silk_overlap enabled): 0 silk_over_copper, 2 silk_overlap (the IRQ/DMA
  divider line over J1, as on v1.3.1), 8 silk_edge_clearance (the bracket parts), as on the blank.

### Other

* Title blocks say v1.3.2 (2026-10-23). The schematic notes about the GY module and the SMD parts are updated.
* No reference designator changed and none is new; J10 is the only one removed. The design rules and net classes
  are v1.3.1's (the `.kicad_pro` is unchanged).
* 32 footprints (the wavetable sheet set, FB3, RV1, U11) still carried symbol links (KIID paths) from before the
  v1.3.0 schematic rebuild; DRC parity had only matched them by reference. They are re-linked to their symbols, so
  "Update PCB from schematic" keeps them.

## Layout status

### Routing

* Freerouting could not finish this placement: the adapters' 1.7 mm pads close both layers in the bus corridor
  between U3/U4 and the fingers. The board was routed with a grid router (negotiated-congestion rip-up and
  re-route); of three net orders tried, analog-first gave the cleanest audio and DAC ground and was kept. Then
  the power tracks were widened where they fit (up to 0.8 mm), GND poured on both layers with stitching vias, the
  back art and text placed, and the review fixes below applied.
* Kept from v1.3.1: the USB D+/D− path through U12 (J11 → U12 pins 6→1 / 4→3 → B.Cu between J11's shield and the
  Pico's top pin row → TP3/TP2; no stubs, the same 0.39–0.52 mm to the shell holes and 0.31 mm to J11's shield);
  the `audio` net class (0.6 mm clearance); ±12V necked to 0.2 mm and centred (≥ 0.32 mm) where it crosses the
  Pico's pin rows on B.Cu.
* Design rules unchanged (the `.kicad_pro` is v1.3.1's): 0.2 mm clearance, 0.25 mm net-class track width,
  0.7/0.35 mm vias. The grid router laid most signal tracks at 0.2 mm: 2.7 of the 4.4 m of signal track, on the ISA
  and the Pico side alike (v1.3.1: about 10 mm).
* 5925 mm of track and 420 vias, 145 of them GND (v1.3.1: 6293 mm, 201 vias).

### Review fixes

The routed board was reviewed against v1.3.1 from four sides (analog/power, digital/ISA/USB, manufacturing,
silkscreen). A second reviewer reproduced every finding (none was rejected). Most were fixed without moving any
footprint; the rest need parts moved or were accepted (see below). A final independent check reproduced the
measurements, the DRC and the JLCPCB package.

* **+5V out of the Waveblaster header rows.** It ran on the solder side between J9.17 (GND) and J9.18 (+12V), 0.22 mm
  from each, and 0.23 mm from C27.1 (−12V). It now runs along the top edge and enters J9.14 between NC/GND pins:
  no +5V within 0.30 mm of a ±12V pad. The J9.19/J9.25 starved thermals went with it.
* **+12V through U2's pin rows** (both layers): centred, locked 0.2 mm necks, 0.37 mm from the pads (was
  0.22–0.27 mm from U2.6 (A8), U2.7 and U2.18 (RTC)).
* **−12V between U9's GND pins 9/10 and 11/12**: 0.2 mm necks, 0.32 mm from the pads (was 0.295).
* **USB-host +5V feed** (D1 → F1, up to 500 mA) re-routed. It crossed both Pico castellation rows on F.Cu
  0.22–0.25 mm from GPIO pads, ran over a Pico bottom test pad and passed between J11 pins 1/2 at 0.225 mm. It now
  crosses the bottom row on B.Cu between U1.37 (3V3_EN, not connected) and U1.38 (GND) and the top row on F.Cu
  between U1.2 (~SPI_CS) and U1.3 (GND), each a locked 0.2 mm neck centred at 0.32 mm, and reaches F1 north of J11
  (≥ 0.65 mm from J11's pins). No +5V or ±12V copper is within 0.30 mm of any Pico pad.
* **Pico test pads and USB relief slots**: the five test-pad rule areas and the two relief slots on Edge.Cuts had
  stayed behind when U1 moved 0.27 mm. They follow U1 again (slot centre x 218.23 in the outline and in the NPTH
  drill file), with no pour or +5V under the test pads.
* **J9 ground**: four GND vias; the back pour under the J9 group is 51 % (was 34 %).
* **Vias**: 29 nudged off THT pads, castellation ends and finger tops; the U8 pin-7 GND via is 0.26 mm from the
  SOIC pad (was 0.10). Router debris removed: 25 duplicate segments, 28 zero-length pieces, 3 stubs inside pads and
  the overlap-only joints; track ends re-centred on their vias (3 still join off-centre, see below).

### Compared with v1.3.1

| | v1.3.1 | v1.3.2 |
|---|---|---|
| DRC errors / unconnected (schematic parity) | 0 / 0 | 0 / 0 |
| starved_thermal warnings | 12 | 2 (U1.28, joined through the other layer) |
| Track / vias / signal vias | 6293 mm / 201 / 96 | 5925 mm / 420 / 226 |
| ISA D0–D7 length / vias | 377 mm / 14 | 442 mm / 52 |
| Signal same-net via pairs < 2.5 mm apart | 2 | 36 |
| I2S length / vias / crossings with audio (BCK, DIN, LRCK) | 274 mm / 0 / 2, 2, 4 (LRCK 20.9 mm in the mixer area) | 235 mm / 1 / 0 |
| Other tracks within 1 mm of AUDIO_L/R and DAC_OUTL/R | +5V 4.4 mm; DAC_OUTL/R 3.2/3.6 mm | none (nearest pads and vias 0.68–0.91 mm) |
| GND resistance to the fingers: U11.3 / U9 AGND / J8 sleeve / J9.1 | 2.76 / 3.23 / 2.91 / 2.98 mΩ | 3.02 / 3.07 / 2.95 / 3.26 mΩ |
| DAC-to-mixer ground offset U9.9 − U11.3 (Pico 100 mA / USB 500 mA / WT 150 mA) | −17 / −188 / −16 µV | +4 / −20 / −35 µV |
| Waveblaster ground J9.25 − U11.3 (WT 150 mA) | +75 µV | +126 µV |
| Pour more than 5 mm from a GND via | 387 mm² | 58 mm² |
| DAC decoupling, supply track / ground path: C13 (AVDD), C16 (CAPP/CAPM), C17 (VNEG) | 7.3/15.8, 5.0/8.9, 7.0/12.1 mm | 3.2/2.4, 3.2/3.2, 6.1/5.2 mm |
| C36 (CPVDD): pin 1 → C36 / C36 → CPGND | 1.9 / 0.6 mm | 2.7 / 8.2 mm |
| Logic decoupling ground path U2 / U5 / U6 / U7 / U10 | 61.7 / 75.3 / 45.3 / 17.2 / 20.1 mm | 23.0 / 19.1 / 19.0 / 17.8 / 18.9 mm |
| U3 / U4 decoupling ground path | 7.3 / 7.4 mm | 24.4 / 47.3 mm |
| +5V resistance, fingers to F1 (USB host) / to J9's +5V pins | 40.0 / 69–77 mΩ | 80.5 / 101–112 mΩ |
| ±12V length at ≤ 0.25 mm | 65.5 / 72.9 mm | 5.2 / 10.6 mm (centred necks) |
| ±12V within 0.30 mm of a non-GND THT pad, solder side | none | none |
| +5V / ±12V within 0.30 mm of a Pico pad | 5 | 0 |
| +5V / +3V3 within 0.30 mm of an other-net THT or Pico pad, solder side | 18 | 7 |
| Vias < 0.30 / < 0.25 mm from an other-net pad | 7 / 3 | 22 / 5 |
| Vias < 0.5 / < 1.0 mm from a gold finger | 0 / 5 | 5 / 16 |
| Silkscreen DRC over copper / overlap / edge | 5 / 14 / 8 | 0 / 2 / 8 |
| Back art | 71.6 × 37.0 mm, 757 mm² of ink | 52.8 × 27.0 mm, 385 mm² |

Resistances and offsets are from a DC copper model of the filled pours; the offsets are the ground-voltage
difference between the two pins for the named load current.

**Better** than v1.3.1: audio isolation, I2S exposure, DAC ground offsets, the DAC and logic decoupling, stitching,
±12V widths, starved thermals, silkscreen. **Worse**: the ISA data bus (length, vias, hop pairs, vias near the
fingers), the USB-host +5V resistance, the U3/U4 and C36 ground paths (placement), the Waveblaster ground offset,
and the art size.

### Known weaker points (accepted)

* **ISA data bus**: D0–D7 442 mm with 52 vias (v1.3.1: 377 mm, 14). D2 is 110 mm with 15 vias, D0 65 mm with 9,
  D4 63 mm with 11; ~RI/OCHRDY has 14 vias and ~BUSOE 7. 36 same-net via pairs closer than 2.5 mm. This works at
  ISA speeds; it is a quality and repairability loss. A routing pass alone could not clean it up: rip-up runs of
  the corridor between U3/U4 and the fingers stayed congested (29–38 nets in conflict), and the D0/D2/D4/D5 finger
  entries are boxed in by vias at the castellation ends, the +3V3 loop around fingers A6–A8 and the locked ±12V,
  IRQ2 and power hops.
* **Vias near the fingers and pads**: 5 within 0.5 mm of a finger top (AEN and ~IOR ×2 at 0.319 mm, IRQ4, D5), all
  tented and at least 0.95 mm from the finger mask opening; 22 within 0.30 mm of an other-net pad, 5 of them within
  0.25 mm (A1, AD2, ~IOW and two D4/D5 vias 0.22 mm from castellation ends 27/24). None had a clean spot within
  0.8 mm.
* **−12V on F.Cu under the Pico** (x 181.53, y 102.4–115.6): under the mask and the module body, its vias tented,
  at least 1.0 mm from castellation pads 25/26. This breaks the letter of the v1.3.1 rule (±12V never on F.Cu next
  to the castellations); moving it to B.Cu would mean re-routing 16 bus crossings. Five tented signal vias sit
  0.22 mm beside it.
* **~SPI_CS between USB D+ and D−** on B.Cu along the Pico's top row (0.27–0.40 mm from each), with the SPI_RX and
  SPI_SCK vias 0.225 mm from the pair. At 12 Mbit/s the coupling is mostly common-mode and USB retries on CRC
  errors; a fix needs SPI_RX, ~SPI_CS and the +5V feed in that corner re-planned together.
* **USB-host +5V feed**: 91 mm with 6 vias from D1 to F1 (through R4.2), with the two locked 0.2 mm necks (7.9 mm in
  all); 80.5 mΩ from the fingers to F1, 40 mV at 500 mA (v1.3.1: 40 mΩ). Its 0.5 mm F.Cu run at y 127.6 is 0.42 mm
  from the lower board edge (x 199–201.5), and it passes 0.25 mm from D1.1 (VSYS) on F.Cu (a bridge there would bypass
  the Schottky). F1's position sets this (see [Placement suggestions](#placement-suggestions)).
* **AUDIO_L over AUDIO_R**: stacked on opposite layers for 22 mm at x 185.85 (v1.3.1 crossed once). Estimated
  L/R crosstalk about −87 dB at 20 kHz and −113 dB at 1 kHz (C19/C20 shunt the node), below what the mixer adds.
  BCK also runs over ISA D2 for about 13 mm (y 95.7–95.8) and over the −12V back trunk for 21 mm; the PCM5100A's
  PLL filters the BCK edges and the measured DAC ground offsets stay ≤ 35 µV.
* **+12V on the component side** 0.22–0.28 mm from 6 THT pads (C21.1, C1.1, R9.1, U3.9, J1.18, U4.9). These are on
  the side opposite the solder joints, so the bridge risk is low.
* **FB3 (AVDD filter) is fed from U8's +3V3 node**: 1.3 mV per 25 mA of PSRAM current (v1.3.1: 0.6 mV), but the
  logic current no longer reaches it, so the total at FB3 is lower than on v1.3.1 (1.4 mV against 3.1 mV for the
  modelled loads), ahead of the ferrite and 10 µF filter.
* **U3/U4 decoupling ground path** 24 / 47 mm: the adapter pinout puts pin 8 (GND) and pin 16 (VCC) at opposite
  corners, and U4.8 sits in the finger no-fill strip. The SN74CB3T3257 is a FET bus switch that draws µA from VCC,
  so this has no practical effect.
* KiCad's (ignored) `track_not_centered_on_via` check lists 3 track ends that join a via off-centre (inside it).

### Placement suggestions

Not done: each one needs parts moved.

* **F1 back west of U12**, as on v1.3.1: one straight, centred B.Cu +5V run could then cross both Pico rows,
  bringing F1 back toward 40 mΩ (each row crossing still needs a centred 0.2 mm neck).
* **U12 back next to J11**: about 9–11 mm less USB D+/D− (53.5 / 51.8 mm now, 42.2 / 42.5 on v1.3.1; irrelevant at
  12 Mbit/s).
* **C36 horizontal right under U9 pins 1–3** (pads at (161.0, 81.0) / (166.0, 81.0)), with C16/C17 one row down: the
  CPVDD loop down to about 2.7 / 2.7 mm (now 2.7 / 8.2, v1.3.1 1.9 / 0.6).
* **C1 0.27 mm right** (back to x 173.5): 0.68 mm instead of about 0.4 mm between its can (nominal size) and a
  full-size Waveblaster daughterboard's maximum outline.
* C19/C20 nearer U9 pins 6–9 would shorten their ground return to AGND pin 9 (19.3 / 20.6 mm of copper, v1.3.1
  18.1 / 9.1 mm; DGND pin 19 to AGND pin 9, set by the pinout, is 22 mm, v1.3.1 9 mm). The ground model shows no
  penalty, so this is not worth a revision on its own.

### Before ordering and building

* **Order the adapters first**: PA0035C stock is thin (1 at DigiKey on 2026-10-23). The own adapter PCBs are the
  fallback (see [Adapter boards](#adapter-boards)).
* In JLCPCB's placement preview, check U8's pin 1 (U8 is at 0° now, v1.3.1 had −90°), U12's pin 1, and the
  bottom-side C8/C10 if JLCPCB places them.
* **Solder J11 before the Pico** and check the gap at its shield pad for a bridge (see
  [Kept from v1.3.1](#kept-from-v131)).

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

## Kept from v1.3.1

* **RV1 / J8**: J8 on top, RV1 below it (at the top, RV1's M7 bushing would have been 0.4 mm above the PC's slot
  opening). **J11 at x 208.64** (at x 210 its flange stood 1.26 mm into the bracket plate). **C32** (47 µF, 11 mm
  tall) out from under the daughterboard at (180.4, 39.4). C1's can is about 0.4 mm from a full-size
  daughterboard's maximum outline at nominal size, 0.16 mm at maximum can size (v1.3.1: 0.68 / 0.43 mm; C1 moved
  0.27 mm with the Pico row).
* No copper inside the gold-finger area: a no-fill rule area covers the strip above the fingers, and **J3's finger
  mask opening starts 0.63 mm lower (y 129.6)**. The ISA GND fingers B1, B10 and B31 each have a 1.0 mm back-side
  track into the pour.
* **Don't run "Update footprint from library"** on J3 (the mask edit), J11 (the shield padstack below) or RV1 (the
  library footprint has a stray Edge.Cuts line that breaks the outline). DRC lists these three as
  `lib_footprint_mismatch` warnings; that is expected. The two Pico micro-USB shell relief slots (1.3 × 2.0 mm) are
  drawn on Edge.Cuts as locked board items (JLCPCB wants non-plated slots on the outline layer), and five rule
  areas keep vias and the front pour away from the Pico's test pads. On v1.3.2 they moved with U1 (0.27 mm left);
  **move them with U1 again if U1 ever moves**.
* J11's shield pad next to Pico pin 4 (SPI_SCK) has a 2.6 mm pad on the front (3.0 mm on the back) for a 0.48 mm
  gap (on the back the pin's THT pad is 2.1 mm away, and USB D+ passes the shield pad at 0.31 mm under the mask).
  **Solder J11 before the Pico and check that gap for a bridge.** H2's mask opening is 0.3 mm smaller (it sits
  0.38 mm from Pico pin 39).
* Rule areas keep copper tracks out of the front exposed-copper logo, and vias and the front GND pour out from under
  the flat Pico's test pads (TP4–TP6, Pico 2 TP7) and its TP1–TP3 USB area.
* U12's GND (pin 2) has a GND via 1.25 mm from the pin.
* A `JLCJLCJLCJLC` order-number placeholder sits on B.Fab next to the Pico (not plotted). Three via holes touch the
  bottom edge of its box: if an order ever uses "Specify a location", raise it 0.4 mm first.

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
* **DAC analogue supply.** On v1.2 the PCM510x AVDD (pin 8) shares the Pico's switch-mode 3.3 V with all the logic.
  Here AVDD has its own rail behind ferrite bead FB3 (the same part as FB1/FB2), with C13 (0.1 µF) and C15 (10 µF) at
  the pin. DVDD and CPVDD stay on the plain 3.3 V, as on v1.2/v1.3.0. Since rev B the charge-pump supply CPVDD (pin 1)
  has its own 0.1 µF, C36, so its current loop pin 1 → C36 → pin 3 (CPGND) is short: 1.9 + 0.6 mm on rev B; on v1.3.2,
  a THT disc just below DIP-20 pin 1, 2.7 mm of track from pin 1 and 8.2 mm of pour back to pin 3 (before rev B it was
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
holes, 6 mm off the card's centre toward the bracket. **On v1.3.2 it is re-placed for the routed board**: 52.8 ×
27.0 mm, its lower edge about 4.1 mm above the Pico's pin holes, 16 mm off the card's centre away from the bracket,
the faces not cut (see [Silkscreen and back artwork](#silkscreen-and-back-artwork)). The back is full of through-hole
pads, so the artwork is cut out 0.3 mm around every pad and hole (silkscreen never lands on copper).

The version and licence block sits between the Pico's pin rows, one text per line, ending with the thanks and the
dedication: *Thanks to polpo and all the PicoGUS contributors. This revision is dedicated to my wonderful family, Mila
and Alisa.* On v1.3.1 each line was nudged (at most 0.9 mm) so that no via hole lands in a letter; on v1.3.2 the lines
are re-spaced so that no via hole is within 0.10 mm of a letter. "< REAR" points at the bracket, its arrow tip in line
with the bracket tab holes H1/H2. All are mirrored on B.SilkS so they read correctly from the back.

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
  On v1.3.2, U2, U6 and U7 (with C4, C6, C9) are not grouped.
* **Layout for audio quality:** every decoupling cap sits next to its chip's supply pin. FB3 sits next to C13/C15.
  The ±12 V filter (R23/R24, C32/C33) sits above U11. The DAC is turned so that its outputs, the output RC filter
  (R7/R8, C19/C20) and the mixer op-amp form one short path to the line-out jack, away from the Pico and the ISA
  logic (v1.3.2: the DIP-20 at 90°, pins 1–10 in its bottom row with R7/R8 and C19/C20 below it).

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

The script stops if DRC (including the schematic-parity check) finds errors; on v1.3.2 it finds 0 errors and 0
unconnected items. It writes:

* `PicoGUS-THT-gerbers.zip`: upload this to jlcpcb.com. It holds the Gerbers (Protel extensions, no X2)
  and the Excellon drill files (PTH and NPTH separate, mm).
* `PicoGUS-THT-bom-jlc.csv` and `PicoGUS-THT-cpl-jlc.csv`: the BOM and placement files for JLCPCB assembly. The
  placement file has the SMD parts only: **U8 and U12 on Top (rotation 0), C8 and C10 on Bottom (rotation 180)**.
  The rotations are KiCad's, with no JLCPCB offset table; U8 was at −90 on v1.3.1, so a correction worked out for
  v1.3.1's U8 does not carry over. U3/U4/U9 are DIP footprints
  now, so they are not in it (the TSSOPs go on the adapters; see [Adapter boards](#adapter-boards) for their own
  JLCPCB files). With SMD parts on both sides, JLCPCB needs two-sided assembly for all four (its economic service
  places one side only; check at order time); hand-soldering the 0805s is easy.
* `PicoGUS-THT-drc.rpt`: the DRC report.

The **Hardware (JLCPCB files)** GitHub Action runs the same script for `hw-tht` (v1.3.0) and `hw-smd`. v1.3.1 and
v1.3.2 are not in it.

Order options: 2 layers, **1.6 mm**, **Surface finish: ENIG** and **Gold fingers: Yes** with the **45° finger
chamfer** (the ISA edge connector). ENIG is required: JLCPCB only gold-plates the fingers on ENIG boards; with HASL
the fingers are tinned, and tin wears and oxidises in an ISA slot. The fingers start 0.58–0.60 mm above the card
edge, as on v1.2, so JLCPCB's bevel (about 0.74 mm) takes about 0.15 mm off their tips. Check the rotations in
JLCPCB's placement preview before paying for assembly (U8's and U12's pin 1, and the bottom-side C8/C10). JLCPCB no
longer prints an order number by default.
The `JLCJLCJLCJLC` placeholder sits on B.Fab next to the Pico (not plotted): if an order ever needs "Specify a
location", move it to B.Silkscreen first, or JLCPCB prints the placeholder text itself.

## Caveats

1. **Not built or tested.** v1.3.2 is routed, DRC clean and reviewed. The circuit is the proven v1.2 one with the
   v1.3.x additions, but no v1.3.x THT board has been fabricated or run in a PC yet. See
   [Known weaker points](#known-weaker-points-accepted) for what the layout gives up against v1.3.1.
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
   pot are outside that area; the Pico's far end is about 1.6 mm under it (1.3 mm on v1.3.1), so with a
   daughterboard fitted solder the Pico flat or use the short Adafruit sockets. IC sockets add 4.6–5.1 mm, and a
   socketed own adapter PCB reaches ~10 mm: solder those directly or check the clearance if the daughterboard has
   parts on its underside (see
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
