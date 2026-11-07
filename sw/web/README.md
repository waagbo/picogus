# PicoGUS web server (Pico W)

PicoGUS boards with a **Raspberry Pi Pico W** (PicoGUS v1.x and PicoGUS Femto
with a Pico W fitted) can run a small web server next to the sound card
emulation. Open the card's address in a browser to:

* see what the card is doing: mode, firmware, WiFi, loaded CD image, and how
  much time the web server takes from the sound card
* change volumes, live
* change settings (ports, IRQ/DMA, GUS/SB/MPU/mouse/CD options) and save them
* switch the card mode (GUS, Sound Blaster, AdLib, MPU-401, Tandy/CMS, USB)
* pick a CD image from the USB drive
* control an [mt32-pi](https://github.com/dwhinham/mt32-pi) connected to the
  MIDI out: MT-32/SoundFont synth, ROM set, SoundFont, reversed stereo, reboot,
  text on the MT-32 display, GM/GS/MT-32 resets, all notes off, raw MIDI bytes
* upload files: a SysEx file (`.syx`) to send to MIDI out, or a firmware
  update (`.uf2`)

All sound modes include it when built for a Pico W board (`PICO_BOARD=picow_fast`).
The NE2000 firmware does not: it hands the WiFi chip to DOS. PicoGUS 2.0
boards have no WiFi chip.

## Setting up

WiFi uses the same settings as the NE2000 firmware. From DOS:

```
pgusinit /wifissid MyNetwork /wifipass MyPassword /save
pgusinit /wifistatus
```

`/wifistatus` prints the address of the web page, e.g. `Connected to SSID
MyNetwork, Signal -55 dB, web UI: http://192.168.1.42/`. The card also registers
the host name `picogus` with your router's DHCP server, which many home routers
make reachable as `http://picogus/`. While the web page can be reached, the
LED on the Pico W is lit.

Without a configured network the WiFi chip stays off and nothing changes for
the sound card.

**There is no password.** Anyone on your network can change the card's
settings or install firmware. Only use it on a network you trust.

## Staying out of the way of the sound card

The ISA bus and the sound emulation always come first:

* **Core 0**, which serves the ISA bus, does no network work at all. It only
  sets a few flags for pgusinit's WiFi commands, which take no time.
* **Everything else runs on core 1, at the lowest priority.** The web server is
  polled from the core 1 main loop of each mode (`web_task()`), in thread mode.
  Audio rendering, the emulated interrupt controller and USB run in interrupts,
  so they always preempt it. lwIP runs in "poll" mode: the WiFi chip's
  interrupt only marks work as pending and runs at the lowest interrupt
  priority, so it never delays the audio interrupt.
* **Bounded work per poll.** `web_task()` runs at most every 2ms. Small TCP
  windows (2 × 1460 bytes) limit how much data one poll can process. Uploads
  are throttled through TCP flow control: MIDI is fed at wire speed, and
  firmware is written one 512-byte UF2 block per poll.
* **Audio buffers come first.** In Sound Blaster and AdLib modes, whose OPL
  synthesis runs in the main loop, the web server only runs while at least
  half of the OPL output buffer (~2.9ms of audio) is filled.
* **No interrupts on core 0.** The WiFi driver's waits are busy-waits on
  core 1 (`--wrap` in `webserver.cmake`); in poll mode they would otherwise set
  alarms whose interrupts fire on core 0.
* **Flash is shared safely.** The WiFi firmware and the web page are read from
  flash, and firmware updates write it. A hardware spinlock keeps core 0 from
  saving settings (which erases flash) at the same time. The save is then done
  on a later ISA write, exactly like `pgusinit /save`. The boot ROM's flash
  routines turn the XIP cache back on, so it is turned off again after every
  flash write, and flash data is read through the cache-bypassing alias (see
  Memory below).
* **WiFi reconnects are rare.** Joining a network keeps core 1 busy for a few
  milliseconds. While the network can't be reached, retries back off from
  every 20 seconds to every ~5 minutes.
* **Settings changes use pgusinit's code.** The web UI goes through the same
  code that handles pgusinit's control port writes (`picogus_web_write()` in
  `picogus.cpp`), and the flash write of a save happens on core 0, as for
  pgusinit.
* **MIDI from the web UI never splits a message of the running program.**
  Messages from the web UI (`MIDI_Inject()` in `mpu401/midi.c`) are sent from
  the core 1 main loop only:
  * only when the DOS program's MIDI stream is between messages and not inside
    a SysEx;
  * SysEx is paced like the MPU-401's SysEx delay;
  * while a program uses MPU-401 intelligent mode, they wait. That mode also
    sends MIDI from core 0 and from an interrupt, which must never wait for
    web data; if it needs the line in the middle of a web message, the rest of
    that message is dropped.

  SysEx messages from files may be up to 512 bytes.

The status page shows how much of core 1 the web server used over the last
10 seconds, and its longest single run.

### Memory

PicoGUS runs entirely from RAM. The web server adds about 55KB of code and data
to each mode (Sound Blaster mode keeps ~39KB free):

| Part | RAM |
| --- | --- |
| lwIP (TCP, UDP, DHCP, ARP, ICMP) | ~22KB |
| CYW43 WiFi driver and SDK glue | ~20KB |
| web server and MIDI queue | ~13KB |

All network *buffers* (lwIP's heap, HTTP connections, upload buffers; 16KB in
total) live in the RP2040's XIP cache memory. PicoGUS turns the flash cache off
at boot, after which that memory can be used as ordinary SRAM at `0x15000000`
(see `web_xip_sram.h`).

A cached flash read would overwrite those buffers, so in these firmwares
everything that reads flash at run time uses the cache-bypassing alias
`0x13000000` instead: the web page, the shared WiFi firmware, and AdLib mode's
OPL level table.

The ~225KB WiFi chip firmware stays in flash. In the multi-mode `picogus.uf2`,
all modes share a single copy, stored right after the bootloader
(`web_cyw43_fw_shared.h`, `multifw/uf2create`).

## Firmware update

Upload a `picogus.uf2` built for Pico W boards to keep the web page. The file
is checked block by block and written straight to flash. Blocks for other
chips are skipped, and the settings sector is never touched. Sound stutters
while flash is erased, and the card restarts into the same mode when done.

If the upload breaks off half way, the old firmware is already partly
overwritten. The card keeps running from RAM, so upload again before
restarting. As a last resort, the Pico W can always be reflashed over USB by
holding BOOTSEL.

## Build options

| CMake option | Default | |
| --- | --- | --- |
| `WEBSERVER` | `ON` on Pico W boards | include the web server in the sound card firmwares |
| `WEB_MDNS` | `OFF` | also answer mDNS for `http://picogus.local/`. Costs ~12KB of RAM, which Sound Blaster mode needs for CD images made of many track files. |

## HTTP API

The web page uses this API; scripts can too. Forms are
`application/x-www-form-urlencoded`, answers are JSON.

| Request | |
| --- | --- |
| `GET /api/status` | firmware, mode, network, all settings (`s`), upload progress, CD image |
| `POST /api/set` | any of the settings keys from `s`, e.g. `sbvol=80&sbirq=7`. Applied immediately, all or nothing. |
| `POST /api/save` | save settings to flash |
| `POST /api/defaults` | load default settings (not saved). The WiFi network is kept. |
| `POST /api/reboot` | restart the card |
| `POST /api/mode` | `mode=1..6` (GUS, AdLib, MPU, PSG, SB, USB), `save=1` to make it the default |
| `POST /api/wifi` | `ssid=...&pass=...`: join another network (save to keep it) |
| `POST /api/midi` | `hex=F07D0300F7`: complete MIDI messages to send to MIDI out |
| `POST /api/upload/syx` | body: a `.syx` file |
| `POST /api/upload/firmware` | body: a `.uf2` file |
| `GET /api/cd` | list CD images on the USB drive (answers `{"busy":true}` until the list is ready) |
| `POST /api/cd/load` | `index=n` from the list, 0 to eject |

## Tests

`web/test` runs the real lwIP stack and the web server on a PC over a
loopback interface. It sends requests in awkward pieces and checks them:
headers, forms, settings, MIDI checks, SysEx and firmware uploads with
backpressure, broken and aborted uploads, connection limits, and a 10 minute
polling run. It also tests the MIDI injection in `mpu401/midi.c`.

```
make -C sw/web/test PICO_SDK_PATH=/path/to/pico-sdk test
```

## Files

| File | |
| --- | --- |
| `web_main.c` | startup, WiFi, pgusinit `/wifistatus`, flash lock, timing statistics |
| `web_http.c` | HTTP server on the lwIP raw API |
| `web_api.cpp` | API handlers |
| `web_upload.c` | firmware and SysEx uploads, MIDI message checks |
| `www/index.html` | the web page, gzipped into the firmware by `scripts/web_embed.py` |
| `lwipopts.h`, `web_cyw43_config.h` | lwIP and WiFi driver configuration |
| `webserver.cmake` | build integration |
