# PicoGUS SMD v1.3.0 – based on PicoGUS v1.2

Board revision **v1.3.0 (SMD)**: the PicoGUS **v1.2** SMD card with the additions made for the
through-hole board in [`hw-tht/`](../hw-tht/README.md):

* **Waveblaster (wavetable) daughterboard header** J9, with MIDI and /RESET drive
* **Analogue WT volume** (RV1) and an **NE5532 op-amp mixer** that sums the WT and PCM audio at unity gain
* **Cleaner audio supplies:** a ferrite-bead-filtered DAC AVDD and RC-filtered ±12 V for the op-amp
* **Same layout as the THT board:** taller card, Pico rotated along the top edge, DAC turned so its
  outputs face the mixer, and one KiCad group per circuit block

> **Status: prototype design, not yet fabricated or tested.**
> The schematic passes ERC (0 errors). The board was autorouted and checked with DRC in KiCad 10
> (0 errors, 0 unconnected), but nobody has built one yet. `hw/` still holds the released v1.2.

| File | What |
|---|---|
| `PicoGUS-SMD.kicad_pro/.kicad_sch/.kicad_pcb` | KiCad **10** project (the root sheet is the v1.2 logic) |
| `audio.kicad_sch` | PCM510x DAC sheet (v1.2 circuit, AVDD now filtered) |
| `wavetable.kicad_sch` | New: Waveblaster header, WT control (U12), WT volume, op-amp mixer, line-out jack |
| `PicoGUS-SMD-schematic.pdf` | Schematic PDF |
| `BOM-SMD.csv` | BOM with a **DigiKey link** for every line, stock and NOK price (checked 2026-09-29) |

## What changed compared to v1.2

The v1.2 logic is unchanged: U2 74LVC244, U3/U4 CB3T3257, U5 74AHC14, U6 74LVC00,
U7 74LVC1G00, U8 PSRAM, U9 PCM510x and U10 74LVC2G06, with the same footprints.

* **Waveblaster header J9** (2x13, 2.54 mm) along the top edge, with 5 V, ±12 V and decoupling at the
  header (C24–C27). The daughterboard's outputs go through C22/C23 to RV1.
* **U12 (74LVC2G06, new)** drives the daughterboard's /RESET (inverted ISA RESET) and MIDI (the same
  UART data as the MIDI-out jack). Its outputs are open drain with 10 k pull-ups to +3.3 V (R9/R16), so
  3.3 V daughterboards are safe. U12B takes its input from the same U5A output that drives U10B (net `~MIDI_DRV`).
* **Mixer U11 (NE5532, SOIC-8)** is an inverting summer. PCM (after the v1.2 470 Ω / 2.2 nF filter) and
  WT (after RV1) each go through 10 kΩ into the virtual ground, with 10 kΩ ∥ 100 pF feedback and a 100 Ω
  output resistor. The PCM level at the jack is the same as on v1.2.
* **Audio quality:**
  * DAC AVDD (U9 pin 8) sits behind ferrite bead FB3, with C13 (0.1 µF) and C15 (10 µF) at the pin.
    DVDD and CPVDD stay on the Pico's 3.3 V.
  * Op-amp ±12 V comes through R23/R24 (10 Ω) and C32/C33 (47 µF), an RC low-pass at ~340 Hz, with
    C28/C29 (0.1 µF) at the pins.
  * C22/C23 are **bipolar** electrolytics, because the DC level across them depends on the daughterboard.
  * The signal-path capacitors (C19/C20, C30/C31) are **C0G**. The signal-path resistors (R7/R8,
    R10–R13, R19/R20) are **0.1 % thin film**, which has lower noise and distortion than thick film and
    keeps left and right matched.
* **Removed:** the GY-PCM5102 module option (I2S_DAC1/J4). The mixer needs the on-board DAC, and the
  module's jack would bypass it. J2 (the ISA debug header, never fitted) is also removed.
* **Pico VBUS** gets a PWR_FLAG, which clears the one ERC error that v1.2 also had.
* **1.6 mm FR4.** The v1.2 project file said 0.57 mm, which is too thin for an ISA edge connector.

## Card dimensions (ISA 8-bit)

The card is **103.7 mm long (bracket to far edge) × 99.9 mm tall** (fingers to top edge). The IBM PC/XT
full-height limit is 4.2 in (106.7 mm), so the card is 6.8 mm under it; the AT limit is 4.8 in (121.9 mm).
The edge connector (62 contacts, 0.1 in pitch), its position and the bracket holes H1/H2 are copied
from v1.2. The existing v1.2 bracket fits, plus one extra hole for RV1 (see below). The tallest
parts are the pot RV1 and the pin headers. A Waveblaster daughterboard sits about 11 mm above the
card, well inside the 0.8 in (20.3 mm) slot pitch of an AT case, as on Sound Blaster cards with a
Waveblaster header. The next card along must not have tall parts on its solder side.

## Ordering

`BOM-SMD.csv` has a DigiKey part number, a clickable digikey.no link, stock and price for every
line. Every line was in stock at DigiKey on 2026-09-29.

| DigiKey, parts only (no PCB, no shipping) | 1 board | per board, 10-board order |
|---|---|---|
| NOK excl. MVA | ~389 kr | ~299 kr |
| NOK incl. 25 % MVA | ~486 kr | ~373 kr |
| USD | ~$41 | ~$31 |

The NOK prices are DigiKey's price breaks converted by findchips.com, so digikey.no may differ by a
few percent. The biggest items are RV1 (45 kr), the Pico (44 kr), the PCM5100A (25 kr) and U3/U4 (40 kr).

Several v1.2 BOM lines are **out of stock at DigiKey today**, so they are replaced with equivalent parts:

| Part | v1.2 BOM | v1.3.0 BOM |
|---|---|---|
| 0.1 µF | Samsung CL21B104KCFNNNE | Samsung CL21B104KBCNNNC |
| 1 µF | Samsung CL21B105KBFNNNE | TDK C2012X7R1H105K125AB |
| 2.2 µF | Samsung CL21A225KBQNNNE | TDK C2012X7R1H225K125AC |
| 10 µF (C15) | Samsung CL21A106KAYNNNE, 0805 | Samsung CL31A106KAHNNNE, **1206** (no 10 µF 0805 in stock) |
| 47 µF | Samsung CL31A476MPHNNNE | TDK C3216X5R1A476M160AB |
| U2 | Nexperia 74LVC244APW,118 | TI SN74LVC244APW |
| R1/R2/R5/R6 | Bourns CR0805 (low stock) | Yageo RC0805FR |

This BOM doesn't include LCSC/JLCPCB assembly data. The `hw/jlcpcb` files are for v1.2.

## Caveats

1. **Untested.** The board is built from the proven v1.2 circuit plus the THT board's additions, but
   it hasn't been fabricated. It was autorouted with Freerouting, so check the routing before ordering.
2. **Audio now needs −12 V.** The mixer runs from ISA ±12 V, so without −12 V there is no line
   output at all (not even PCM). Every AT/ATX PC power supply provides −12 V.
3. **C22/C23 footprint.** KiCad has no non-polar SMD electrolytic footprint, so the silkscreen shows
   a "+". The parts are bipolar, so either orientation is correct.
4. **RV1 availability.** Alps RK097 dual pots have low stock (DigiKey 210). The switched RK0971221Z0X
   fits the same footprint.
5. **ISA bracket.** Drill one more hole (about 7 mm, for RV1's bushing) between the MIDI and line-out
   jack holes.
6. **Silkscreen.** Reference labels were placed automatically, and some sit on their default spot
   next to a neighbour. Tidy them before ordering.
