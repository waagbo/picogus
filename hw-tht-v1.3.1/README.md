# PicoGUS THT (through-hole) v1.3.1 (rev B, work in progress) – based on PicoGUS v1.2

Board revision **v1.3.1 (rev B)**: the [v1.3.0 THT board](../hw-tht/README.md) with a **new placement**, a
**USB-A host port** at the bracket and the Waveblaster header **without the daughterboard standoff holes**. The rest of the circuit is v1.3.0's.

> **Status: routed, DRC clean; not built or tested yet.**
> The schematic passes ERC; the board passes DRC with schematic parity (0 errors, 0 unconnected). See
> [Layout status](#layout-status). This folder is not built by the JLCPCB GitHub Action yet.

Files:

| File | What |
|---|---|
| `PicoGUS-THT.kicad_pro/.kicad_sch/.kicad_pcb` | KiCad **10** project (the root sheet is the v1.2 logic). The board is routed |
| `audio.kicad_sch` | PCM510x DAC sheet (as v1.3.0) |
| `wavetable.kicad_sch` | Waveblaster header, WT volume, op-amp mixer, line-out jack (as v1.3.0, with the +6 dB WT gain) |
| `usb.kicad_sch` | New: USB host port (J11 USB-A, F1, C34/C35, U12) |
| `PicoGUS-THT-schematic.pdf` | Schematic PDF |
| `BOM-THT.csv` | BOM with DigiKey **and** LCSC part numbers and a digikey.no link per line (v1.3.0 lines checked 2026-09-28, socket lines 2026-10-18, C36 2026-10-19) |
| `BOM-THT-digikey-upload.csv` (+ `-optional`, `-10x`) | DigiKey cart/BOM upload with the reference designators as Customer Reference. Optional: sockets, U12 |
| `BOM-THT-sockets.csv` (+ `-digikey-upload`, `-10x`) | Socket build: every IC socketed, with heights (see [Socket build](#socket-build)) |

The shared `PiGUS library` symbols and footprints are linked from `hw-common/`, like the other boards.
New in the shared library for rev B: `RPi_Pico_SMD_TH_USB` and `Wavetable_NoMountingHoles` (plus the unused
`USB_A_C_Dual_Molex_67643_GCT_USB4085_Horizontal` and `DIP-16_W7.62mm_TSSOP-16_Nested`).

## What changed compared to v1.3.0

### New placement

The Raspberry Pi Pico moved down to the lower right, the bracket parts moved up (from the top: J8 line out,
RV1 WT volume, J5 MIDI out, J11 USB-A), and the mixer sits next to them. Part positions are in the board file; see
[Layout status](#layout-status). The bracket holes therefore differ from v1.3.0: see [Bracket](#bracket).

### USB host port (J11, sheet 4 "USB host port")

A USB-A host port at the bracket for a mouse, joystick, keyboard, flash drive (CD-ROM images) or a hub, as on the
PicoGUS NE2000 card and the PicoMEM 2.

* **Data**: straight from the Pico's own USB D+/D− test points **TP3 / TP2** (behind the Pico's 27 Ω series
  resistors). U1 uses `RPi_Pico_SMD_TH_USB`: the v1.3.0 Pico footprint plus three plated holes under TP1 (GND),
  TP2 (D−) and TP3 (D+). The Pico must be **soldered flat** (castellated edge or through-hole pins, no header) so
  the three pads can be soldered from the back of the card. The positions are the same on Pico, Pico 2, Pico W and
  Pico 2 W. On headers or in a socket, solder three short wires (26–30 AWG, under 3 cm) from TP1–TP3 into the holes,
  or use the micro-USB OTG route of v1.3.0. The Pico's micro-USB stays in parallel with J11: **use one or the other.**
* **J11**: Molex 67643 (0676430910, DigiKey WM4993-ND / LCSC C2798029), stock footprint
  `Connector_USB:USB_A_Molex_67643_Horizontal`. Its front lip (14.4 × 7.16 mm, the frontmost part, 12.99 mm in
  front of the pin row) sits 0.1 mm behind the inside of the bracket plate. It is larger than the 13.6 × 6.2 mm
  cut-out, so it is the panel stop: nothing of J11 enters the cut-out, only the USB plug comes in through it. The
  receptacle face is 0.9 mm behind the outside of the bracket (PicoGUS 2.0: 1.3 mm), see [Bracket](#bracket).
* **VBUS**: ISA +5 V through **F1** (Bourns MF-R050, 0.5 A hold) with C34 (220 µF low-impedance; USB 2.0 asks for
  ≥ 120 µF) and C35 (0.1 µF), always on, like the PicoMEM 2's USB-A port. The Pico's VBUS pin (J7) is not involved.
* **U12** (USBLC6-2SC6, SOT-23-6): optional ESD protection on D+/D−. It sits right behind J11's pins and is wired
  flow-through: D+ and D− run 4.5 mm from J11 into U12 pins 6/4, straight under the body to pins 1/3, and on to
  the Pico, so there are no stubs and the port works without U12. GND (pin 2) has a via right at the pin and a
  1 mm track to C34's GND pad, which ties to J11 pin 4 with a 1 mm track. VBUS (pin 5) goes through a via to J11
  pin 1. C35 (0.1 µF) sits just below U12. Pin 1 is marked by a silk dot west of the chip (the footprint's own
  pin-1 triangle lands on a via). **If you fit U12, solder it before C34, C35 and J11**, which box it in.
* USB-C was considered and left out: the PicoMEM 2 also uses USB-A for its host port and USB-C only for
  programming (5.1k Rd, never sourcing VBUS). A dual USB-A/USB-C footprint is in the shared library, unused.

### U3/U4: TSSOP-16, as on v1.3.0

An earlier rev B draft gave U3/U4 (SN74CB3T3257, TSSOP-16 only) a nested DIP-16 + TSSOP-16 footprint for
300-mil adapter boards. It was dropped: its DIP pads and the TSSOP-to-DIP links took most of the routing room
and ground around the bus switches. U3/U4 are back on v1.3.0's `Package_SO:TSSOP-16_4.4x5mm_P0.65mm`, SMD only.

### Waveblaster header without standoff holes

J9 uses `PiGUS library:Wavetable_NoMountingHoles`: the v1.3.0 `Wavetable` footprint without the two 3.2 mm
daughterboard standoff holes (the far one landed in U4 in the new placement). The daughterboard is held by the
2x13 header only, so the M3 standoff and screws are gone from the BOM. A full-size daughterboard overhangs the
card's far end; support it by hand when plugging/unplugging cables.

### Bracket

The bracket parts are not where v1.3.0 (and v1.2) had them, so a v1.2/v1.3.0 bracket does not fit. Hole
positions on a Keystone 9202 style bracket (registered on the card's two tab holes H1/H2; distances measured down
from the bracket's top surface, height = centre above the card's component side):

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

### Other

* Title blocks, silkscreen and back silkscreen say v1.3.1.
* No v1.3.0 reference designator changed. The design rules are v1.3.0's.
* New references: J11, F1, C34, C35, U12 (USB port); C36 (U9 CPVDD decoupling). VBUS_USB is in the pwr net class.

## Layout status

The board holds the rev B placement "take 2" with the fixes from its review:

* **RV1 / J8 swapped**: at the top, RV1's M7 bushing was 0.4 mm above the PC's slot opening (it could not pass a
  bracket in a case). J8 is now on top, RV1 below it.
* **J11 1.36 mm further in** (x 208.64): at x 210 its flange stood 1.26 mm into the bracket plate. C34, F1 and R4
  moved 0.45–0.7 mm left to make room.
* **C32** (47 µF, 11 mm tall) out from under the daughterboard, to (180.4, 39.4) next to C31.
* R17 / U5 and J6 / J7 courtyard overlaps fixed; R19 / R20 turned so their inverting-input ends sit at U11;
  C8 next to U8's VCC pin; R2 horizontal above U3/U4 (frees the corridor between them); C1/C2 out of the
  daughterboard corner.

After U3/U4 went back to TSSOP-16:

* **U3/U4 at 90°** (pin 1 bottom left; positions unchanged). Of the 16 rotation combinations this one has the
  shortest, least crossed bus ratsnest, and it routed best: with the same autorouter set-up the ISA data/address
  and Pico AD buses took 1 284 mm and 56 vias, against 1 525 mm and 85 vias at the old -90°/-90°. The finished
  board (with the decoupling links and audio clearance below) has 1 401 mm and 61 vias; the board with the nested
  footprint had 1 954 mm and 108.
* **C3/C5 at the pin-16 end** of their chip: +3V3 from the cap to pin 16 is a 2.3–2.8 mm track, and GND runs from
  pin 8 under the chip body back to the cap (about 7 mm, no via). v1.3.0: about 2.7 mm and 8 mm with a via.
* **U10 (with C21, R18) up to y 66.6**, U7 and U6 spread evenly between U10 and U5 (y 80.9 and 95.2, with C6/C9):
  about 4.5 mm between the DIPs for routing instead of 0.4 mm between U7 and U6.

Routing: Freerouting (100 passes) after hand pre-routes, which are locked in the board:

* IRQ2 along the finger corridor (J1 is at the far left, its finger B4 at the far right);
* USB D+/D− from J11 through U12 (J11 → pin 6/4 → under the body → pin 1/3), then on B.Cu down the west side of
  U12, between J11's lower shield pad and the Pico's pin row, and through the gaps between the Pico's micro-USB
  shell holes to TP3/TP2 (≥ 0.3 mm from the holes and from J11's shield, 42.2 / 42.5 mm long, no stubs). The
  B.Cu under J11 is all GND pour, tying J11's shield pads to its GND pin;
* the U9 (PCM510x) GND, VNEG and XSMT stubs, as on v1.3.0;
* the U3/U4 decoupling links above, and the −12V trunk from finger B7 up to the analog section (it crosses the
  Pico's pin rows on B.Cu).

Design rules are v1.3.0's again: signal tracks 0.25 mm, vias 0.7/0.35 mm (the nested footprint had needed
0.2 mm and 0.6/0.3 mm). Power tracks (+5V, +3V3, ±12V, VBUS, AVDD) were widened after routing wherever they
fit, up to 0.8 mm; about half of the ±12V length is still 0.25 mm in the tight spots, as on v1.3.0 (they
carry tens of mA; −12V from the finger to J9 is 0.21 Ω, v1.3.0 0.29 Ω), and where ±12V crosses the Pico's pin
rows it is necked to 0.2 mm for 2.2 mm, centred between the pads (≥ 0.32 mm). The audio nets (DAC outputs to the mixer, WT_L/WT_R, the volume-pot wipers and the op-amp
inputs) are in an `audio` net class with **0.6 mm clearance**, so no other track runs closer than 0.6 mm
alongside them; none comes within 0.5 mm. WT_L was finished by a grid router at that clearance and then
straightened (no vias). GND is poured on both layers with stitching vias. Rule areas keep copper tracks out of the
front exposed-copper logo, and vias and the front GND pour out from under the flat Pico's test pads (TP4–TP6,
Pico 2 TP7) and its TP1–TP3 USB area.

From the routed-board review (kept on the re-routed board):

* No copper inside the gold-finger area and no vias right above the fingers. **J3's finger mask opening on this
  board starts 0.63 mm lower (y 129.6)**.
* **Don't run "Update footprint from library"** on J3 (the mask edit above), J11 (the shield padstack below) or
  RV1 (the library footprint has a stray Edge.Cuts line that breaks the outline). DRC lists these three as
  `lib_footprint_mismatch` warnings; that is expected. The two micro-USB shell slots under the Pico are also drawn
  on Edge.Cuts (locked board items): move them with U1 if U1 ever moves.
* J11's shield pad next to Pico pin 4 (SPI_SCK) has a 2.6 mm pad on the front (3.0 mm on the back) for a 0.48 mm
  gap. **Solder J11 before the Pico and check that gap for a bridge.** The USB D+ track also passes 0.31 mm
  from the back of that shield pad (under the mask). H2's mask opening is 0.3 mm smaller (it
  sits 0.38 mm from Pico pin 39).
* C1/C2 0.5 mm right (C1 now 0.4 mm clear of the daughterboard corner even at maximum can size).

From the review of the re-routed board:

* ±12V never runs on F.Cu next to the Pico's castellations (a bridge there would put −12 V on a GPIO); the B.Cu
  crossings of the pin rows are necked as above, and ±12V keeps ≥ 0.38 mm from C11.1 (RUN) and FB3.1.
* No front GND pour under the flat Pico's exposed test pads or around TP1–TP3 (the back pour stays).
* The Pico's two 1.3 × 2.0 mm micro-USB shell relief slots are drawn on Edge.Cuts as well as in the NPTH drill
  file: JLCPCB wants non-plated slots on the outline layer.
* U9 (PCM510x) CPGND (pin 3, through C36's GND pad since the revision review) and AGND (pin 9) have GND vias
  into the back pour.
* TP2 = D− and TP3 = D+ were checked against Raspberry Pi's Pico R3 schematic and KiCad's own Pico footprint.
* Silkscreen: all text at least 0.8 mm / 0.15 mm stroke; labels and references off pads; "LINE OUT" (was
  "AUDIO OUT") and "WT VOL" next to J8/RV1; "USB" by J11.

From the rev B revision review (U9 charge-pump decoupling, U12 at J11, ISA GND finger B10):

* **U9 CPVDD decoupling:** C36 (0.1 µF X7R 0603) sits under U9 pins 1–3, pad 1 on CPVDD (pin 1), pad 2 on
  CPGND (pin 3), with 0.6 mm stubs. The charge-pump supply loop pin 1 → C36 → pin 3 is about 6.6 mm around
  (2.7 mm²); before, pin 1's nearest +3V3 cap was C4, 23.6 mm of track away, with the return through the pour
  (TI's layout guide and PicoGUS 2.0, C15 at its DAC pin 1, put this cap right at the pin). The CPGND via into
  the back pour moved to (150.55, 72.80), just below C36, and two GND stitching vias (131.6, 77.4) and
  (139.0, 76.4) keep the front pour strip west of it tied to the back. To make room, C16 moved 0.5 mm left and
  R7 0.4 mm down: CAPM now runs straight down from pin 4 and then at 45° between C36 and R7, and the
  flying-cap loop (pin 2 → C16 → pin 4) grows from 16.5 to 20.6 mm². CPVDD stays on +3V3, as on v1.2/v1.3.0:
  tying it to AVDD as on PicoGUS 2.0 would need the charge-pump routing redone, and TI's 10 µF bulk cap at
  CPVDD is not fitted locally (the nearest bulk is C1). The MIDI TX track (U10 → R3) still runs under U9's
  body on the front: there is no short alternative path, and it is not part of the decoupling loop.
* **U12 at J11** (see the USB host port section): it used to hang on 13 / 26 mm stubs off the D+/D− pair, with
  21.6 mm of track from its GND pin to J11. Now the pair runs through it, and U12's GND reaches J11.4 through
  12.9 mm of track (DC copper model 2.6 mOhm, was 2.7); J11's two shield pads are joined to each other and to
  J11.4 by the back pour (0.3 mOhm, was 1.9). C35 moved below U12 (horizontal) and C34 0.2 mm up; ~RIOR,
  ~SPI_CS and SPI_RX were re-routed locally (at most +2.1 mm). Side effects, all small at 12 Mbit/s: the pair
  is 12.5 mm longer (matched to 0.28 mm), it has less GND under it on the front, the lower J11 shield pad's path
  to the ISA GND fingers is a little longer, and C35's GND return to J11 is longer (C35 only bypasses VBUS; the
  USBLC6's own clamp does the ESD work). U12's courtyard is 0.05 mm from J11's.
* **ISA GND finger B10:** joined to the GND pour by a wide back-side strap (1.5–3 mm) along the only free
  corridor: 1.9 mm wide under the IRQ2 via, then 3 mm wide diagonally between the AEN and D4 tracks into the
  U3/U4 GND pour. The old 0.5 mm track stays underneath where the strap covers it (its piece next to the
  unconnected fingers B11/B12 is gone: GND keeps 0.24 mm from them). Nothing new in the gold-finger area; the
  IRQ2 corridor is unchanged. DC copper model, 1 A: the B10 link itself falls from about 12.6 to 3.5 mOhm, so
  every B10 path is about 10 mOhm lower (to B1 about 12, because the U12 change also lowers B1's own path into the pour).
  B10 to U4 GND about 18 → 8 mOhm (v1.3.0: 18), to U3 GND
  21 → 11 (v1.3.0: 9), to B1 / B31 41 / 35 → 29 / 25 (v1.3.0: 18 / 16).
  The rest of the gap to v1.3.0 is the U3/U4 pour's own connection to the Pico-side pour: D4 and the +3V3 feed
  to C1 separate them on the back, so closing it means re-routing those (a follow-up, not a local change).

## Socket build

`BOM-THT-sockets.csv` (with `BOM-THT-sockets-digikey-upload.csv` and `-10x`) lists everything needed to
build the card with every IC socketed. The same parts are the optional socket lines of `BOM-THT.csv`.

| Socket for | Part (DigiKey) | Height above the board | Top of the part |
|---|---|---|---|
| U2 (DIP-20) | On Shore SA203000 (ED3020-ND) | 4.6 mm | ≤ 9.8 mm, **under the daughterboard** |
| U5, U6, U7, U10 (DIP-14 ×4) | Adam Tech ICM-314-1-GT-HT (2057-ICM-314-1-GT-HT-ND) | 4.6 mm | ≤ 9.8 mm, **under the daughterboard** |
| U11 (DIP-8) | On Shore SA083000 (ED3038-ND) | 4.6 mm | ≤ 9.8 mm, outside |
| U1 Pico | Adafruit 5585 short sockets (1528-5585-ND) + 5584 short plug headers (1528-5584-ND) | 5.0 mm | Pico board at ~7.5 mm |

* All are machined (round-pin) sockets with 0.5 mm tails that fit the existing 0.8 mm DIP holes. LCSC
  equivalents (CONNFLY DS1001-01, XKB) are in the CSV.
* **Daughterboard clearance**: a Waveblaster daughterboard sits ~11 mm above the card. U2, U5, U6, U7 and U10 are
  under it, and a socketed DIP reaches ~9.8 mm (about 1.2 mm worst-case margin, 2 mm typical). That is fine for a
  daughterboard with a flat underside; if yours has parts underneath, solder those five ICs directly or use the
  low-profile Mill-Max 115-43-3xx-41-003000 sockets (2.41 mm), which need 1.0 mm holes (a footprint change). The TE stamped sockets of the v1.3.0 BOM are 5.1 mm high (not ~4 mm as
  the v1.3.0 README says).
* **A socketed Pico has no USB host port** (J11) unless three short wires connect its TP1–TP3 to the holes under
  it. The Adafruit short sockets keep the Pico at ~7.5 mm; the standard 8.5 mm Sullins sockets at ~11–12 mm.
* J10 (GY-PCM5102 module) stays soldered flat, as on v1.3.0.


# v1.3.0 reference (unchanged in rev B unless noted above)

## What changed in v1.3.0 compared to v1.2

### Through-hole parts only

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

Only the parts that exist in SMD packages alone stay SMD (rev B: plus C36, below). All of them have a pitch of 0.65 mm or more and can be hand-soldered:

* **U3, U4** SN74CB3T3257: FET bus-switch mux with 5 V→3.3 V clamping. It only comes in
  TSSOP-16/TVSOP. No DIP part does this job without redesigning the ISA bus interface (and the PIO timing).
* **U8** APS6404L PSRAM: SOIC-8.
* **U9** PCM5100A/PCM5101A/PCM5102A DAC: TSSOP-20.
* **C36** (rev B) 0.1 µF X7R 0603 (Yageo CC0603KRX7R9BB104, LCSC C14663, a JLCPCB basic part): the DAC's
  charge-pump (CPVDD) decoupling cap. It only works right at U9's pins, where no through-hole cap fits.
  Solder it after U9.

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
  standoff). **Rev B has no standoff holes** (see above).

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
  (0.1 µF) and C15 (10 µF) at the pin. DVDD and CPVDD stay on the plain 3.3 V, as on v1.2/v1.3.0. Rev B: the
  charge-pump supply CPVDD (pin 1) has its own 0.1 µF, C36 (0603), directly under pins 1–3; its current loop
  pin 1 → C36 → pin 3 (CPGND) is about 6.6 mm around (before: about 24 mm of track to C4 and back through the
  pour). The bead drops only millivolts at the DAC's ~10 mA.
* **Capacitor types in the signal path:** C19/C20 (DAC output filter) and C30/C31 (op-amp feedback) are
  **C0G/NP0**, which has no voltage or temperature coefficient and so adds no distortion. X7R is used only for
  supply decoupling.

### DAC option: on-board PCM5100A **or** a GY-PCM5102 module (J10)

The PCM output can come from **either** the on-board DAC **or** a plug-in GY-PCM5102 module. Fit one of
the two, **never both** (two DAC outputs would drive the same line).

| Build | Fit | Leave out |
|---|---|---|
| On-board DAC (default) | U9, C12–C20, C36, FB3, R7, R8 | J10 (the module) |
| GY-PCM5102 module | the module, soldered almost flat in J10 | **U9, C12–C20, C36, FB3, R7, R8** |

* The module lies **over the DAC area**: the parts to leave out for it are exactly the ones inside the
  module's **dashed J10 outline** on the silkscreen, headed "DAC -OR- GY-PCM5102". That includes the
  DAC's output filter (R7/R8 470 Ω, C19/C20 2.2 nF): the module has the same 470 Ω / 2.2 nF filter on
  board, so its outputs join the audio path after it (AUDIO_L/AUDIO_R, straight into the mixer).
* Solder the module **almost flat, directly on its 2.54 mm male pins** (no sockets): its 6-pin I2S header
  (SCK BCK DIN LCK GND VIN) and a 4-pin piece of the 1x40 strip on the **AGND ROUT AGND LROUT end** of
  its 9-pin row, both pointing down. It then sits a few mm above the card, well under a Waveblaster
  daughterboard (~11 mm); on 8.5 mm sockets it would hit one.
* Set the module's solder jumpers **H1=L, H2=L, H3=H, H4=L**. It gets +5 V (it has its own 3.3 V
  regulator); SCK is grounded on the card. Its own 3.5 mm jack is not used: the audio goes through the
  mixer to J8.
* Either way the mixer, the Waveblaster mix and the firmware are the same. With the module the board is
  fully through-hole apart from U3/U4 and the SOIC-8 PSRAM.
* The module is not sold by DigiKey (AliExpress, eBay and the like).

### Back silkscreen

The back carries the PicoGUS artwork (hw-common/art/picogus-artwork.svg: three game heroes over the
PicoGUS logo), 76 × 27 mm (rev B: in the upper half of the card, behind the mixer and DAC), with the dedication
(rev B: along the card's far edge, read from the back): *This version is
dedicated to Mila and Alisa, my two wonderful princesses, who fill my world with joy.* The licence block sits
between the Pico's pin rows (rev B). All are mirrored
on B.SilkS so they read correctly from the back. The back is full of through-hole pads, so the artwork is
cut out 0.3 mm around every pad and hole (silkscreen never lands on copper).

### Other changes

* The board is **103.7 × 99.9 mm**. It has the same length, ISA edge connector and bracket tab holes (H1/H2) as
  v1.2 (on v1.3.0 the MIDI and line-out jacks are also in the v1.2 places; rev B moves them, see
  [Bracket](#bracket)), but it is taller, like PicoGUS 2.0, to fit the
  Waveblaster header along the top edge. 1.6 mm FR4 (the v1.2 and v1.3.0 board files' stackup says 0.57 mm; rev B's
  says 1.6 mm, so its 3D and STEP exports have the right thickness).
* Design rules are relaxed for THT/DIY: 0.2 mm clearance, 0.25 mm signal / 0.4 mm power tracks
  (+5 V, +3.3 V, VSYS), 0.7/0.35 mm vias (rev B: the same, plus a 0.6 mm-clearance `audio` class and power
  tracks widened to up to 0.8 mm where they fit). GND is routed
  and poured on both layers, with stitching vias.
* **J2 (the 2x31 ISA debug/probe header of v1.2, never fitted) is removed.** On a through-hole board
  its pads formed a wall right above the gold fingers that every ISA signal had to cross.
* The `+3.3V` power symbols are renamed `+3V3`. KiCad 8+ names power nets after the symbol value, so the
  upgraded v1.2 sheets would otherwise have split the 3.3 V rail into two nets.
* The symbol fields of U3/U4 now name the CB3T3257 that is actually used (the v1.2 fields still said 74FST3257DR2G).
* **Layout is grouped by circuit block.** Each chip and its decoupling caps and resistors form a KiCad group,
  so a block can be selected and moved as one: DAC (U9), Mixer + line out (U11, J8), WT volume (RV1),
  Waveblaster header (J9), MIDI out (J5), Pico + power (U1), ISA bus switches (U3, U4), ISA buffer (U2),
  74AHC14 (U5), 74AHC00 (U6, U7), 74AHCT126 (U10), PSRAM (U8), IRQ/DMA jumpers (J1).
* **Layout for audio quality:** every decoupling cap sits next to its chip's supply pin, with a short
  return to GND. FB3 sits next to C13/C15. The ±12 V filter (R23/R24, C32/C33) sits above U11. The PCM5102 is rotated so that its outputs, output RC filter (R7/R8, C19/C20) and the
  mixer op-amp form one short, straight path to the line-out jack, away from the Pico and the ISA logic.

## Card dimensions (ISA 8-bit)

The card is **103.7 mm long × 99.9 mm tall** (bottom of the fingers to the top edge). The IBM PC/XT
full-height limit is 4.2 in (106.7 mm), so the card is 6.8 mm under it; the AT limit is 4.8 in (121.9 mm).
The edge connector (62 contacts, 0.1 in pitch), its position and the bracket holes are unchanged from
v1.2, and the board is 1.6 mm, the standard ISA card thickness. A Waveblaster daughterboard sits about
11 mm above the card, inside the 0.8 in (20.3 mm) AT slot pitch. Clip the THT leads short
(≤ 2 mm) on the solder side, where the next card sits.

The **SMD** version of this board is in [`hw-smd/`](../hw-smd/README.md).

## Ordering

`BOM-THT.csv` lists, for every line, the DigiKey part (with DigiKey stock) and the LCSC part
(with LCSC stock) as of 2026-09-28, plus alternatives. DigiKey numbers were checked
against findchips distributor data, and LCSC numbers against the LCSC product API.

* **Logic ICs (U2, U5, U6, U7, U10): order from DigiKey.** LCSC lists the DIPs, but has 0–74 in stock.
  Everything else is available from both.
* **DigiKey BOM cost** (2026-09-28, parts only, no PCB, no shipping), from DigiKey's price breaks:

  | | 1 board | per board, 10-board order |
  |---|---|---|
  | NOK excl. MVA (digikey.no) | ~474 kr | ~356 kr |
  | NOK incl. 25 % MVA | ~592 kr | ~444 kr |
  | USD | ~$50 | ~$37 |

  The optional sockets and Pico headers add ~39 kr (~$4) (v1.3.0 figures; rev B drops the ~10 kr daughterboard standoff
  and screws, and the machined socket set is ~80 kr plus ~21 kr for the Pico sockets). NOK figures are DigiKey's USD prices converted
  at 9.53 NOK/USD, so digikey.no can differ by a few percent. The Pico (44 kr), U3/U4 (40 kr)
  and the 18 × 0.1 µF (59 kr) are the big items.
* **Ceramic capacitors must have 5.0 mm (or 5.08 mm) lead pitch.** 2.5 mm parts don't fit (e.g. TDK FG1x, Vishay K…L2).

Mistakes found in the v1.2 BOM/JLCPCB data (don't copy these from the v1.2 files):
C139966 is a Vilsion part, not the APS6404L. C2910528 is a 3.5 mm jack with a different pinout.
C2905423 is a female header, not the Pico. C2935925 is DEALON, not Amphenol. C6942 is the SOIC SN74AHC14DR.

## JLCPCB files

```
hw-common/tools/jlcpcb.sh hw-tht-v1.3.1/PicoGUS-THT.kicad_pcb   # -> hw-tht-v1.3.1/build/jlcpcb/
```

The script stops if DRC (including the schematic-parity check) finds errors. `FORCE=1` exports anyway. It writes:

* `PicoGUS-THT-gerbers.zip`: upload this to jlcpcb.com. It holds the Gerbers (Protel extensions, no X2)
  and the Excellon drill files (PTH and NPTH separate, mm).
* `PicoGUS-THT-bom-jlc.csv` and `PicoGUS-THT-cpl-jlc.csv`: the BOM and placement files for JLCPCB assembly (SMD parts only).
  Only U3/U4, U8/U9, C36 and the optional U12 are SMD on this board; the LCSC column comes from the schematic fields.
* `PicoGUS-THT-drc.rpt`: the DRC report.

The **Hardware (JLCPCB files)** GitHub Action runs the same script for `hw-tht` (v1.3.0) and `hw-smd`. Rev B is
not in it yet; run the script locally (rev B passes its DRC gate: 0 errors, 0 unconnected).

Order options: 2 layers, **1.6 mm**, **Surface finish: ENIG** and **Gold fingers: Yes** with the **45° finger
chamfer** (the ISA edge connector). ENIG is required: JLCPCB only gold-plates the fingers on ENIG boards; with HASL
the fingers are tinned, and tin wears and oxidises in an ISA slot. Check the rotations in JLCPCB's
placement preview before paying for assembly. JLCPCB no longer prints an order number by default. A
`JLCJLCJLCJLC` placeholder sits on B.Fab next to the Pico (not plotted): if an order ever needs "Specify a
location", move it to B.Silkscreen first, or JLCPCB prints the placeholder text itself.

## Caveats

1. **Untested.** This board is built from the proven v1.2 circuit, but it hasn't been fabricated or
   run in a PC. It was autorouted with Freerouting, so check the routing before ordering.
2. **Audio now needs −12 V.** The mixer op-amp runs from ISA ±12 V, so without −12 V there is
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
   full-size (Creative-spec) board covers most of the card's left half. Only low parts are placed
   under it (DIP ICs, axial resistors, radial MLCCs, the J1 jumper block as on PicoGUS 2.0). Electrolytics
   and the pot are outside that area (rev B: also U3/U4 are under it, and the Pico's far end is 1.3 mm under
   it, so with a daughterboard fitted solder the Pico flat or use the short Adafruit sockets). IC sockets add 4.6–5.1 mm and may touch a daughterboard
   that has parts on its underside, so solder the DIPs directly or check the clearance (see [Socket build](#socket-build)).
6. **RV1.** The BOM uses the Bourns PTD902-2020K-A103 (DigiKey 426, 24 kr). It has the same 2.5 mm
   pin grid and M7 bushing as the Alps RK097 the footprint was drawn for; test-fit one in the bracket
   before a large order. LCSC doesn't list it, so the LCSC column keeps the Alps RK09712200HA
   (C470545). The Alps parts (RK09712200HA: DigiKey 210 / LCSC 175, 45 kr; the switched RK0971221Z0X
   also fits) have low stock, and Alps North America has published a discontinuation notice that
   covers parts of the range.
7. **ISA bracket.** v1.3.0: drill one more hole (about 7 mm, for RV1's bushing) between the MIDI and
   line-out jack holes, which are where they are on v1.2. **Rev B: no v1.2/v1.3.0 bracket fits; cut the four
   openings in the [Bracket](#bracket) table.**
8. **Mixed LCSC stand-ins.** LCSC's 3.5 mm jacks are XKB PJ-325C5 equivalents. Their unused
   TN pin sits ~0.4 mm further out, so clip it if it doesn't fit. Don't use C2910528
   (different pinout; it is in the v1.2 JLCPCB project). The Pico is not sold in the LCSC shop
   (only in the JLCPCB assembly library). DigiKey doesn't stock the APS6404L PSRAM; Adafruit 4677 (the bare chip) is the DigiKey source.
9. **Ferrite beads.** FB1/FB2 are axial beads on leads (DigiKey Fair-Rite 2743002112, LCSC FH
   RH3.5x0.8x9P52E). A wire link also works (MIDI out works without the beads, with slightly worse EMI).
10. The v1.2 symbols are cached in the schematic, so KiCad reports "symbol doesn't match library"
    warnings. That is expected; don't "update symbols from library" blindly.
11. Waveblaster pin 8 (MIDI out from the daughterboard) and the daughterboard audio inputs (pins 12/16)
    are not used. PicoGUS has no MIDI in.
