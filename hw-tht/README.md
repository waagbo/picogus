# PicoGUS THT (through-hole) v1.3.0 – based on PicoGUS v1.2

Board revision **v1.3.0**: a through-hole variant of the PicoGUS **v1.2** ISA card, with an added
**Waveblaster (wavetable) daughterboard header** and an **analogue volume
control** that mixes the wavetable output with the PicoGUS audio.

> **Status: prototype design, not yet fabricated or tested.**
> The schematic passes ERC and was netlist-compared against v1.2. The board was
> autorouted and checked with DRC in KiCad 10, but nobody has built one yet.
> Read the [caveats](#caveats) before ordering boards.

Files:

| File | What |
|---|---|
| `PicoGUS-THT.kicad_pro/.kicad_sch/.kicad_pcb` | KiCad **10** project (the root sheet is the v1.2 logic) |
| `audio.kicad_sch` | PCM510x DAC sheet (same circuit as v1.2, with THT passives) |
| `wavetable.kicad_sch` | New: Waveblaster header, WT control logic, WT volume, op-amp mixer, line-out jack |
| `PicoGUS-THT-schematic.pdf` | Schematic PDF |
| `BOM-THT.csv` | BOM with DigiKey **and** LCSC part numbers, plus a digikey.no link per line (stock checked 2026-09-28) |
| `BOM-THT-digikey-upload.csv` (+ `-optional`) | DigiKey cart/BOM upload: quantity, DigiKey part number, MPN, and the reference designators as Customer Reference (printed on each bag) |

The shared `PiGUS library` symbols and footprints are linked from `hw-common/`, like the other boards.
All other footprints come from KiCad's standard libraries.

## What changed compared to v1.2

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
PCM510x OUTL --- 470R ---+--- 10k (R10) ---+-------+---- 100pF (C30) ---+
                         |                 |       +---- 10k (R19) -----+
                       2.2nF               |       |                    |
                         |                 |       +--|- \              |
                        GND                |          |NE5532 >---------+--- 100R (R21) --> LINE OUT L (J8)
WT L (J9.24) -- 10uF bipolar (C22) --+-- RV1A --- 10k (R12)   +--|+ /
                             |   10k log   (from wiper)    |
                         100k (R14)                       GND
                             |
                            GND                 (right channel: R11, R13, R20, C31, R22, RV1B, C23, R15)
```

* The two sources are summed by a **NE5532 inverting summer** (U11, DIP-8, powered from ISA ±12 V).
  Both inputs have unity gain, so the PCM line out has the same level as on v1.2 (2.1 Vrms full scale),
  and the WT source is not attenuated by the mixing. The output is phase-inverted, which you can't hear.
* **RV1** (dual-gang 10 k audio taper, Alps RK097 dual, right angle) sets the Waveblaster
  level. Its shaft goes through the ISA bracket between the MIDI and line-out jacks. The signal is on the
  CW-end terminals (3/6) and GND is on the CCW end (1/4), so the level rises clockwise. The 10 k summing
  resistor loads the wiper only a little, because a log pot's wiper is near the GND end for most of its travel.
* C22/C23 (10 µF **bipolar** electrolytics, Panasonic SU) block any DC offset from the daughterboard.
  The DC across them depends on the daughterboard (some outputs are AC-coupled, some carry an offset), so a
  polarised cap could end up reverse-biased; a bipolar one doesn't care. R14/R15 (100 k) give the
  capacitors a defined DC level when no daughterboard is fitted. Without a daughterboard, RV1's setting doesn't matter.
* The PCM510x sees a ~10.5 kΩ load (its datasheet minimum is 1 kΩ). The output is DC-coupled, which is fine because the
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

### Back silkscreen

The back carries the "Mila and Alisa" artwork (the princesses, the castle and the PicoGUS logo) with the
dedication below it: *This version is dedicated to Mila and Alisa, my two wonderful princesses, who fill
my world with joy.* It is mirrored on B.SilkS so it reads correctly from the back. On this board the back is full of through-hole pads, so the artwork is cut out 0.3 mm around every pad and hole (silkscreen never lands on copper) and the dedication sits in the one pad-free strip below it.

### Other changes

* The GY-PCM5102 **module option is removed** (I2S_DAC1/J4 of v1.2; on v1.2, J4 carried
  AGND/ROUT/AGND/LROUT). The Waveblaster mix needs the on-board DAC, and the module's own jack would bypass
  the mixer. The DAC itself stays SMD, as it must.
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
  | NOK excl. MVA (digikey.no) | ~495 kr | ~373 kr |
  | NOK incl. 25 % MVA | ~618 kr | ~466 kr |
  | USD | ~$52 | ~$39 |

  The optional sockets and Pico headers add ~39 kr (~$4). NOK figures are DigiKey's USD prices converted
  at 9.53 NOK/USD, so digikey.no can differ by a few percent. The Pico (44 kr), RV1 (45 kr),
  U3/U4 (40 kr) and the 18 × 0.1 µF (59 kr) are the big items.
* **Ceramic capacitors must have 5.0 mm (or 5.08 mm) lead pitch.** 2.5 mm parts don't fit (e.g. TDK FG1x, Vishay K…L2).

Mistakes found in the v1.2 BOM/JLCPCB data (don't copy these from the v1.2 files):
C139966 is a Vilsion part, not the APS6404L. C2910528 is a 3.5 mm jack with a different pinout.
C2905423 is a female header, not the Pico. C2935925 is DEALON, not Amphenol. C6942 is the SOIC SN74AHC14DR.

## JLCPCB files

```
hw-common/tools/jlcpcb.sh hw-tht/PicoGUS-THT.kicad_pcb        # -> hw-tht/build/jlcpcb/
```

The script stops if DRC (including the schematic-parity check) finds errors. `FORCE=1` exports anyway. It writes:

* `PicoGUS-THT-gerbers.zip`: upload this to jlcpcb.com. It holds the Gerbers (Protel extensions, no X2)
  and the Excellon drill files (PTH and NPTH separate, mm).
* `PicoGUS-THT-bom-jlc.csv` and `PicoGUS-THT-cpl-jlc.csv`: the BOM and placement files for JLCPCB assembly (SMD parts only).
  Only U3/U4 (and U8/U9) are SMD on this board; the LCSC column comes from the schematic fields.
* `PicoGUS-THT-drc.rpt`: the DRC report.

The **Hardware (JLCPCB files)** GitHub Action runs the same script on every push that touches the
boards. The files can be downloaded as build artifacts (`jlcpcb-tht`, `jlcpcb-smd`).

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
   the pot and the Pico are outside that area. IC sockets add ~4 mm and may touch a daughterboard
   that has parts on its underside, so solder the DIPs directly or check the clearance.
6. **RV1 availability.** Alps RK097 dual pots have low stock at both distributors
   (RK09712200HA: DigiKey 210 / LCSC 175). The switched RK0971221Z0X fits the same footprint.
   Alps North America has published a discontinuation notice that covers parts of the range.
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
