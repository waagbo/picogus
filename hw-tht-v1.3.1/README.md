# PicoGUS THT (through-hole) v1.3.1 (rev B, work in progress) – based on PicoGUS v1.2

Board revision **v1.3.1 (rev B)**: the [v1.3.0 THT board](../hw-tht/README.md) plus a **USB host port**
at the bracket and a **DIP-16 option for U3/U4**. The rest of the circuit is v1.3.0 unchanged.

> **Status: schematic done, board UNROUTED.**
> The schematic passes ERC. The board has every v1.3.0 part at its v1.3.0 position, U1/U3/U4 on their new
> footprints, the new parts parked below the card edge, and **no tracks, vias or zone fills**. It is the
> starting point for the rev B placement (moving the Pico down, placing the USB port), not a board to order.
> DRC on this file reports the unrouted nets and the U3/U4 overlaps listed under
> [Placement to do](#placement-to-do); everything else matches v1.3.0.
> This folder is not built by the JLCPCB GitHub Action yet (its DRC would fail until the board is routed).

Files:

| File | What |
|---|---|
| `PicoGUS-THT.kicad_pro/.kicad_sch/.kicad_pcb` | KiCad **10** project (the root sheet is the v1.2 logic). The board is **unrouted** |
| `PicoGUS-THT.kicad_dru` | Custom DRC rules: keep 0.50 mm hole-to-hole and 0.13 mm annular ring on everything except the USB-C pins |
| `audio.kicad_sch` | PCM510x DAC sheet (as v1.3.0) |
| `wavetable.kicad_sch` | Waveblaster header, WT volume, op-amp mixer, line-out jack (as v1.3.0, with the +6 dB WT gain) |
| `usb.kicad_sch` | New: USB host port (J11, F1, C34/C35, R25/R26, U12) |
| `PicoGUS-THT-schematic.pdf` | Schematic PDF |
| `BOM-THT.csv` | BOM with DigiKey **and** LCSC part numbers and a digikey.no link per line (v1.3.0 lines checked 2026-09-28, rev B lines 2026-10-18) |
| `BOM-THT-digikey-upload.csv` (+ `-optional`, `-10x`) | DigiKey cart/BOM upload with the reference designators as Customer Reference. Optional: sockets, U3/U4 adapters, the USB-C receptacle, U12 |
| `BOM-THT-sockets.csv` (+ `-digikey-upload`, `-10x`) | Socket build: every IC socketed, with heights (see [Socket build](#socket-build)) |

The shared `PiGUS library` symbols and footprints are linked from `hw-common/`, like the other boards.
New in the shared library for rev B: `RPi_Pico_SMD_TH_USB`, `DIP-16_W7.62mm_TSSOP-16_Nested` and
`USB_A_C_Dual_Molex_67643_GCT_USB4085_Horizontal`.

## What changed compared to v1.3.0

### USB host port (J11, sheet 4 "USB host port")

A USB host port for a mouse, joystick, keyboard or flash drive, as on the PicoGUS NE2000 card.

* **Data**: straight from the Pico's own USB D+/D− test points **TP3 / TP2** (behind the Pico's 27 Ω series
  resistors). U1 now uses `RPi_Pico_SMD_TH_USB`: the v1.3.0 Pico footprint plus three plated holes under TP1
  (GND), TP2 (D−) and TP3 (D+). The Pico must be **soldered flat** on the card (castellated edge or through-hole
  pins, no header) so the three pads can be soldered from the back of the card through the holes.
  The positions are the same on Pico, Pico 2, Pico W and Pico 2 W.
* **Pico on headers or in a socket**: the TP pads no longer touch the card. Either use the micro-USB OTG route
  of v1.3.0 (an OTG adapter on the Pico's micro-USB, jumper J7 for VBUS), or solder three short wires (26–30
  AWG, under 3 cm, twisted together) from the Pico's TP1/TP2/TP3 into the holes. The Pico's micro-USB stays in
  parallel with the port: **never use the micro-USB and J11 at the same time.**
* **One footprint, USB-A or USB-C** (`PiGUS library:USB_A_C_Dual_Molex_67643_GCT_USB4085_Horizontal`): fit one of the two sockets.
  * USB-A: Molex 67643 (0676430910 black, DigiKey WM4993-ND / LCSC C2798029; or 0676433910 white,
    WM14667-ND). The NE2000 card's 0676432910 has the same land pattern but is out of stock at DigiKey.
  * USB-C: GCT USB4085-GF-A (USB 2.0, all pins through-hole; DigiKey 2073-USB4085-GF-ACT-ND / LCSC C7095263).
    Its signal tails are only 1.6 mm long (flush with the back of the board): solder it from the back with a
    fine tip and let the solder wick up the holes. The USB-C signal pads have no top-side solder-mask opening,
    so a fitted USB-A shell can't short them.
  * Both front faces are on one line, 1.5 mm beyond the board edge (the Molex flange then sits just behind the
    bracket plate). This is estimated from a bracket STEP model: **test-fit a real bracket** before ordering.
  * The GCT pattern needs two project-rule changes, already made in this folder: Board Setup min hole-to-hole
    0.45 mm (was 0.50) and min annular width 0.12 mm (was 0.13), and `PicoGUS-THT.kicad_dru` puts 0.50/0.13 mm
    back on every other footprint. JLCPCB's limits are 0.45 mm pad hole-to-hole and 0.18 mm PTH annular ring,
    so the 0.125 mm ring of the USB4085 pins may get a DFM note.
* **VBUS**: ISA +5 V through **F1** (resettable fuse, 0.5 A hold) with C34 (220 µF low-impedance, USB 2.0 asks for ≥ 120 µF) + C35 (0.1 µF). The Pico's VBUS
  pin (J7) is not involved.
* **USB-C**: R25/R26 (56 kΩ Rp on CC1/CC2) tell a USB-C device that this is a 5 V source (default USB power).
  With a USB-A socket the CC pads are unused; fit R25/R26 anyway or leave them out. VBUS is always on, which a
  USB-C source strictly should not do (see [caveat 12](#caveats)): **USB-A is the safer choice**.
* **U12** (USBLC6-2SC6, SOT-23-6): optional ESD protection on D+/D−.
* **Bracket**: J11 needs a cut-out in the ISA bracket; see [Placement to do](#placement-to-do).

### U3/U4: TSSOP-16 or a 300-mil DIP-16 adapter

U3/U4 (SN74CB3T3257) only exist as TSSOP-16 (and smaller). Their new footprint
`PiGUS library:DIP-16_W7.62mm_TSSOP-16_Nested` is a 0.3" DIP-16 (1.0 mm drills) with the TSSOP-16 pads
between its rows; both pad sets carry the same pin numbers, so fit **one** of:

* the TSSOP-16 soldered directly (as v1.3.0, or by JLCPCB assembly), or
* a **300-mil** TSSOP-16-to-DIP-16 adapter board in the DIP pads, soldered or in a DIP-16 socket:
  Chip Quik PA0034C (round pins fitted) or SparkFun BOB-13994 (you add the pins; its pads are on a 0.635 mm
  pitch, so centre the 0.65 mm TSSOP carefully).
  The common 0.5"/0.6" "SOP16/TSSOP16 to DIP16" adapters **do not fit**.

The footprint's origin and rotation are the TSSOP's, so the JLCPCB placement file stays correct:
`hw-common/tools/jlcpcb.sh` now also exports parts on `*_Nested` footprints (it used to export SMD-typed
footprints only). Each nested footprint needs 16 short links and 4 vias under the TSSOP body when routed.

### Other

* Title blocks, silkscreen and back silkscreen say v1.3.1.
* New references: J11, F1, C34, C35, R25, R26, U12. No v1.3.0 reference changed.

## Placement to do

The board is the v1.3.0 placement with three changes. **U1** has the new footprint at the old place.
**U3/U4** sit on their v1.3.0 TSSOP centres, turned 90° so the two DIP outlines don't overlap; they still
overlap C3, C5, C29, C33, R2 and R24, which is what DRC reports (besides the unrouted nets).
**J11, F1, C34, C35, R25, R26, U12** are parked below the ISA edge, with a note on the User.Comments layer.

* **J11** needs 17.2 mm of bracket edge and 14.3 mm of depth. It is parked rotated 90° with its origin on the
  bracket edge line (x = 220.13 mm), so it only has to move up. Today the bracket edge holds the Pico's USB end
  (y ≈ 41–64), H1 (64–71), J5 (76–85), RV1 (86–96), J8 (105–114) and H2 (120–127): the only stretch long
  enough is where the Pico is, so moving the Pico down frees the place for J11. (Firmware updates go over ISA
  with `pgusinit`, so the Pico's micro-USB doesn't need to reach the bracket.)
* **F1, C34, C35** next to J11 (C34 is the bulk cap for plugging devices in); **R25/R26** and **U12** close to
  J11 too.
* **D+/D−**: keep TP2/TP3 → J11 short and together (12 Mbit/s full speed, so length matching is not critical).
  At J11 the USB-A and USB-C pin orders differ and the A6/B6 and A7/B7 links run diagonally: route D+ and D− on
  different layers around the connector. One 0.25 mm track fits between the USB-C rows, none between two pins
  of one row.
* **U3/U4** need 20.9 × 9.8 mm each (the plain TSSOP needed about 7 × 5 mm). When routing, each needs 16 short
  links from the TSSOP pads to the DIP pads and 4 vias under the TSSOP body; the feasibility study did this
  with 0 DRC errors on the board's own rules.
* **Bracket cut-out**, centred on J11's centre line: USB-A 14.0 × 7.0 mm, centre 4.1 mm above the board top
  (spans 0.6–7.6 mm); USB-C 9.5 × 3.6 mm with R1.8 ends, centre 1.9 mm above the board top. The USB-A cut-out
  also passes a USB-C plug, so one bracket can take either connector.
* Under the Pico, the footprint keeps the copper keep-outs for the Pico's exposed test pads TP4–TP6
  (Dwgs layer) and marks the Pico 2's TP7.
* When the board is routed: refill the zones, run DRC with schematic parity, and add `hw-tht-v1.3.1/PicoGUS-THT`
  to the matrix in `.github/workflows/hardware.yml`.

## Socket build

`BOM-THT-sockets.csv` (with `BOM-THT-sockets-digikey-upload.csv` and `-10x`) lists everything needed to
build the card with every IC socketed. The same parts are the optional socket lines of `BOM-THT.csv`.

| Socket for | Part (DigiKey) | Height above the board | Top of the part |
|---|---|---|---|
| U2 (DIP-20) | On Shore SA203000 (ED3020-ND) | 4.6 mm | ≤ 9.8 mm, **under the daughterboard** |
| U5, U6, U7, U10 (DIP-14 ×4) | Adam Tech ICM-314-1-GT-HT (2057-ICM-314-1-GT-HT-ND) | 4.6 mm | ≤ 9.8 mm, **under the daughterboard** |
| U11 (DIP-8) | On Shore SA083000 (ED3038-ND) | 4.6 mm | ≤ 9.8 mm, outside |
| U3, U4 (DIP-16 ×2) | On Shore SA163000 (ED3016-ND) + Chip Quik PA0034C adapters ×2 (315-PA0034C-ND) | 4.6 mm | ~7.9 mm, outside |
| U1 Pico | Adafruit 5585 short sockets (1528-5585-ND) + 5584 short plug headers (1528-5584-ND) | 5.0 mm | Pico board at ~7.5 mm |

* All are machined (round-pin) sockets with 0.5 mm tails that fit the existing 0.8 mm DIP holes. LCSC
  equivalents (CONNFLY DS1001-01, XKB) are in the CSV.
* **Daughterboard clearance**: a Waveblaster daughterboard sits ~11 mm above the card. U2, U5, U6, U7 and U10 are
  under it, and a socketed DIP reaches ~9.8 mm (about 1.2 mm worst-case margin, 2 mm typical). That is fine for a
  daughterboard with a flat underside; if yours has parts underneath, solder those five ICs directly or use the
  low-profile Mill-Max 115-43-3xx-41-003000 sockets (2.41 mm), which need 1.0 mm holes (a footprint change; the
  nested U3/U4 footprint already has them). The TE stamped sockets of the v1.3.0 BOM are 5.1 mm high (not ~4 mm as
  the v1.3.0 README says).
* **U3/U4 in sockets**: the TSSOP chips go on 300-mil adapters with **round** pins. The Chip Quik PA0034C has them
  fitted (DigiKey had 28 on 2026-10-18). Cheaper: SparkFun BOB-13994 (one pack = two adapters) with round 0.5 mm
  machined pins (LCSC C9742 / SparkFun PRT-00117, not at DigiKey). Never push square header pins into a machined
  socket: they ruin its contacts. Soldered straight into the board, the BOB-13994 takes plain square pins.
* **A socketed Pico has no USB host port** (J11): its USB test pads only reach the board when the Pico is soldered
  flat. Use the micro-USB OTG route of v1.3.0 instead, or solder three short wires from the Pico's TP1–TP3 into the
  holes (which ties the Pico to the board again). With the standard 8.5 mm Sullins sockets the Pico sits at
  ~11–12 mm and its SWD end touches the last ~1.6 mm of a full-size daughterboard outline in the v1.3.0 placement.
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

Only the parts that exist in SMD packages alone stay SMD. All of them have a pitch of 0.65 mm or more and can be hand-soldered:

* **U3, U4** SN74CB3T3257: FET bus-switch mux with 5 V→3.3 V clamping. It only comes in
  TSSOP-16/TVSOP. No DIP part does this job without redesigning the ISA bus interface (and the PIO timing).
* **U8** APS6404L PSRAM: SOIC-8.
* **U9** PCM5100A/PCM5101A/PCM5102A DAC: TSSOP-20.

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
* The footprint includes the standard support hole at the far end of the daughterboard
  (the M3 × 11 mm standoff is in the BOM).

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
  (0.1 µF) and C15 (10 µF) at the pin. DVDD/CPVDD stay on the plain 3.3 V. The bead drops only millivolts
  at the DAC's ~10 mA.
* **Capacitor types in the signal path:** C19/C20 (DAC output filter) and C30/C31 (op-amp feedback) are
  **C0G/NP0**, which has no voltage or temperature coefficient and so adds no distortion. X7R is used only for
  supply decoupling.

### DAC option: on-board PCM5100A **or** a GY-PCM5102 module (J10)

The PCM output can come from **either** the on-board DAC **or** a plug-in GY-PCM5102 module. Fit one of
the two, **never both** (two DAC outputs would drive the same line).

| Build | Fit | Leave out |
|---|---|---|
| On-board DAC (default) | U9, C12–C20, FB3, R7, R8 | J10 (the module) |
| GY-PCM5102 module | the module, soldered almost flat in J10 | **U9, C12–C20, FB3, R7, R8** |

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
PicoGUS logo), 76 × 27 mm, just above the ISA fingers, with the dedication higher up on the back, above the DAC area: *This version is
dedicated to Mila and Alisa, my two wonderful princesses, who fill my world with joy.* Both are mirrored
on B.SilkS so they read correctly from the back. The back is full of through-hole pads, so the artwork is
cut out 0.3 mm around every pad and hole (silkscreen never lands on copper).

### Other changes

* The board is **103.7 × 99.9 mm**. It has the same length, ISA edge connector and bracket holes as v1.2
  (the MIDI and line-out jacks are in the same places), but it is taller, like PicoGUS 2.0, to fit the
  Waveblaster header along the top edge. 1.6 mm FR4 (the v1.2 project file said 0.57 mm).
* Design rules are relaxed for THT/DIY: 0.2 mm clearance, 0.25 mm signal / 0.4 mm power tracks
  (+5 V, +3.3 V, VSYS), 0.7/0.35 mm vias. GND is routed and poured on both layers, with stitching vias.
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

  The optional sockets and Pico headers add ~39 kr (~$4) (v1.3.0 figures; rev B adds about 35 kr for the USB
  port: J11 17 kr, F1 4 kr, R25/R26 2 kr, C34/C35 ~10 kr; the machined socket set is ~80 kr plus ~21 kr for the Pico sockets). NOK figures are DigiKey's USD prices converted
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
  Only U3/U4, U8/U9 and the optional U12 are SMD on this board; the LCSC column comes from the schematic fields.
  U3/U4 are on the nested through-hole footprint; the script adds them to the CPL file anyway (rev B).
* `PicoGUS-THT-drc.rpt`: the DRC report.

The **Hardware (JLCPCB files)** GitHub Action runs the same script for `hw-tht` (v1.3.0) and `hw-smd`. Rev B is
not in it yet: until the board is routed, its DRC fails on the unrouted nets (`FORCE=1` exports anyway).

Order options: 2 layers, **1.6 mm**, and **Gold fingers: Yes** with the **45° finger chamfer** (the ISA
edge connector). ENIG is a good choice for the rest of the board. Check the rotations in JLCPCB's
placement preview before paying for assembly.

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
   under it (DIP ICs, axial resistors, radial MLCCs, the J1 jumper block as on PicoGUS 2.0). Electrolytics,
   the pot and the Pico are outside that area. IC sockets add 4.6–5.1 mm and may touch a daughterboard
   that has parts on its underside, so solder the DIPs directly or check the clearance (see [Socket build](#socket-build)).
6. **RV1.** The BOM uses the Bourns PTD902-2020K-A103 (DigiKey 426, 24 kr). It has the same 2.5 mm
   pin grid and M7 bushing as the Alps RK097 the footprint was drawn for; test-fit one in the bracket
   before a large order. LCSC doesn't list it, so the LCSC column keeps the Alps RK09712200HA
   (C470545). The Alps parts (RK09712200HA: DigiKey 210 / LCSC 175, 45 kr; the switched RK0971221Z0X
   also fits) have low stock, and Alps North America has published a discontinuation notice that
   covers parts of the range.
7. **ISA bracket.** Drill one more hole (about 7 mm, for RV1's bushing) between the MIDI and
   line-out jack holes. Those two holes are where they are on v1.2.
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
12. **Rev B: USB-C VBUS is always on.** A USB-C source is supposed to switch VBUS on only after it sees a
    device's Rd on CC ("cold socket"). J11 has +5 V on VBUS all the time, through F1. With a device (mouse, stick,
    keyboard) that is harmless, but never connect the USB-C port to another source (a charger, a PC, a phone)
    with a C-to-C cable: the two 5 V supplies would be tied together, and F1 does not stop back-feeding into the
    ISA +5 V. The USB-A option has no such problem. A CC-detecting load switch would fix it if wanted.
13. **Rev B: the Pico must be soldered flat** for the USB port (see above), and the micro-USB on the Pico is in
    parallel with J11: use one or the other.
