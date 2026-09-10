# PGDFS - the PicoGUS USB drive as a DOS drive letter

PGDFS is a small DOS TSR (about 10 KB resident) that maps the USB drive
plugged into a PicoGUS ISA card to a DOS drive letter, in the same way
PicoMEM's PMDFS does. DOS sees a network drive: `DIR`, `COPY`, `TYPE`,
`CD`, `MD`, `DEL`, `REN`, `ATTRIB`, running programs from it, and so on all
work as usual. The card serves the file system itself from FatFs, so DOS
never sees sectors and the drive format is the card's business.

The DOS side is derived from Mateusz Viste's EtherDFS client; the requests
and answers are the EtherDFS "EDF5" protocol, moved over the ISA bus instead
of Ethernet. The protocol is documented in `sw/dfs/PROTOCOL.md`.

## Requirements

* A PicoGUS with firmware that includes PGDFS (protocol version 5 or later).
  Older firmware makes PGDFS print "this PicoGUS firmware has no PGDFS
  support"; upgrade with `pgusinit /flash picogus.uf2`.
* A FAT12/FAT16/FAT32-formatted USB drive plugged into the PicoGUS.
* MS-DOS 5.0 or later (or a compatible DOS such as FreeDOS). DOS must have a
  free drive letter: raise `LASTDRIVE=` in `CONFIG.SYS` if PGDFS complains
  that it cannot map the letter.
* Any PC: PGDFS runs on an 8088 upwards (it detects the CPU and only uses
  `REP INSB`/`REP OUTSB` on an 80186 or later).

## Usage

```
PGDFS X: [/Q] [/R]     map the PicoGUS USB drive to drive X:
PGDFS /U [/Q]          unload PGDFS from memory
PGDFS /T               push the DOS date and time to the card and exit
```

* `X:` - the drive letter to use (any unused letter up to `LASTDRIVE`).
* `/R` - map the drive read-only. Writes, creates, deletes, renames,
  `MD`/`RD` and attribute changes are refused with "access denied" without
  bothering the card.
* `/Q` - quiet: print nothing when loading or unloading succeeded.
* `/U` - unload the TSR. Works when PGDFS was the last program to hook
  INT 2Fh (unload TSRs in the reverse order of loading).
* `/T` - only send the DOS clock to the card, whether or not the TSR is
  loaded. PGDFS does this at install time as well; the card uses the clock
  for the timestamps of files created or modified from DOS. Run `PGDFS /T`
  again after changing the DOS date or time.
* `/?` - help.

At install time PGDFS checks for the card, prints the mounted USB drive
(label, file system, size) and the resident size, and maps the letter. The
USB drive does not have to be present when PGDFS loads: without one, any
access to the drive letter fails with "drive not ready" until a drive is
plugged in.

Example `AUTOEXEC.BAT` line: `PGDFS E: /Q`.

`PGUSINIT.EXE` shows the state of the USB drive on a `USB drive:` line of
its normal output when the firmware has PGDFS.

## Test tool

`PGDFSTST.EXE` talks to the card through the same transport as the TSR
without installing anything, and doubles as the protocol conformance check
for the firmware. Every failure is reported with the status byte, the DOS
result (AX) and the lengths involved.

```
PGDFSTST /INFO              card, protocol, frame size, USB drive info, free space
PGDFSTST /ECHO [n]          echo 64/512/4096-byte payloads n times, verify, KB/s
PGDFSTST /DIR [path]        list a directory (FINDFIRST/FINDNEXT)
PGDFSTST /TYPE file         show a file (READ)
PGDFSTST /GET remote local  copy a file from the USB drive
PGDFSTST /PUT local remote  copy a file to the USB drive
PGDFSTST /TIME              push the DOS clock to the card
```

Remote paths are relative to the root of the USB drive (`\DIR\FILE.TXT`); a
drive letter prefix is ignored. `PGDFSTST /INFO` followed by `PGDFSTST /ECHO`
is the first thing to run on new firmware.

## Limitations

- `COPY` does not preserve timestamps: a file copied to the drive gets the time of the copy (from the DOS clock pushed at install or with `/T`). EtherDFS behaves the same; DOS sets the source time on the handle and expects the redirector to apply it at close, which PGDFS does not do yet.

* DOS sees 8.3 names only. Long file names on the USB drive appear as their
  short aliases (`LONGNA~1.EXT`); files created from DOS get plain 8.3
  names.
* One drive: the first FAT volume of the USB drive is served as remote drive
  index 0. exFAT is not supported yet.
* DOS 3.x is not supported yet (PGDFS uses the DOS 4+ layout of the SDA and
  the CDS, like EtherDFS).
* Windows 9x (DOS box and Windows itself) is untested.
* No file locking across programs: LOCK/UNLOCK calls are accepted and
  ignored, as in EtherDFS.
* Free space and total size are reported capped just under 2 GB, which is
  what DOS can represent.
* Unplugging the USB drive while files are open on it invalidates those
  files; programs get "drive not ready" on the next access.

## Troubleshooting

* `PicoGUS not detected` - nothing answered on port 1D0h. Check that the
  card is seated and that `PGUSINIT` finds it.
* `This PicoGUS firmware has no PGDFS support` - the firmware is older than
  the PGDFS protocol (protocol 5). Upgrade the firmware with `pgusinit /flash`.
* `PicoGUS firmware uses protocol N` - same as above, older protocol.
* `Drive not ready` / "not ready reading drive X:" when accessing the drive
  - no USB drive is mounted on the card (none inserted, unplugged, or the
  card is still mounting it), the drive is not FAT-formatted, or the card
  did not answer within 5 seconds. `PGUSINIT` and `PGDFSTST /INFO` show
  what the card sees.
* `Cannot map this drive letter (LASTDRIVE too low?)` - raise `LASTDRIVE=`
  in `CONFIG.SYS` or pick a lower letter.
* `This drive letter is already in use` - the letter belongs to a local
  disk, a SUBST or another network drive.
* `PGDFS is already loaded` - use `PGDFS /U` first to change the mapping.
* Wrong timestamps on files created from DOS: run `PGDFS /T` after setting
  the DOS clock (the card keeps DOS time + elapsed time).
* Emulators: DOSBox and DOSBox-X do not emulate a PicoGUS, so PGDFS reports
  "PicoGUS not detected" there. The undocumented `/N` option installs the
  TSR without looking for the card (for testing the TSR mechanics); it
  needs a real DOS kernel with a CDS, which DOSBox-X's built-in DOS does not
  provide.

## Performance notes

Every DOS file operation becomes one request/answer transaction over the
ISA bus: the request bytes are streamed to port 1D3h, the card serves it
from FatFs on its second core, and the answer is streamed back. The
transfer runs at ISA I/O speed (roughly 1 microsecond per byte with
`REP INSB`/`OUTSB`, several times slower with the 8088 loop), so throughput is
bounded by the bus and by the USB drive on the card. Reads and writes are
chunked to the frame payload size (4096 bytes, or less if the firmware
reports a smaller `CMD_DFSMAXLEN`); DOS programs that read in large blocks
get the best speed, programs that read byte by byte pay one full transaction
per call. `PGDFSTST /ECHO` measures the raw transport speed on a given
machine.

The resident part keeps one 4100-byte frame buffer plus a private stack, so
it needs about 10 KB of conventional memory; it can be loaded high with
`LOADHIGH`/`LH`.

## Building

OpenWatcom 2.0: `make` (GNU make, as used by the CI) or `wmake -f makefile.wat`
builds `PGDFS.EXE` and `PGDFSTST.EXE`. The TSR is compiled with `-0 -s -ms`
(8086 code, no stack checks, small model); its resident code lives in the
`BEGTEXT` segment and must not call the C library. Check `pgdfs.map` after
changes: the `DGROUP` size must not exceed `DATASEGSZ` in `globals.h`.

## Credits

* EtherDFS by Mateusz Viste (MIT license), whose DOS client PGDFS is
  derived from, and whose EDF5 protocol it speaks:
  http://etherdfs.sourceforge.net
* PMDFS in the PicoMEM project by FreddyV, the model for serving EtherDFS
  requests from a FatFs volume on an ISA card.
* `chint086.asm` contains code from the Open Watcom project (Sybase Open
  Watcom Public License).

See `LICENSE` in this directory.
