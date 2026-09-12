# PGDFS wire protocol

PGDFS lets a DOS network-redirector TSR (`PGDFS.EXE`) use the USB drive plugged
into the PicoGUS as a drive letter. DOS forwards every file operation on that
letter (open, read, find-first, mkdir, ...) to the TSR as an INT 2Fh/11h call;
the TSR packs the call into a request frame, ships it to the card, and unpacks
the answer. The card serves the request from its own FatFs volume, so DOS never
sees sectors and the volume format is the card's business.

The request/answer payloads are the EtherDFS "EDF5" protocol (Mateusz Viste,
MIT), which is also what PicoMEM's PMDFS speaks. Only the framing and the
transport differ: EtherDFS uses raw Ethernet frames, PMDFS a shared-RAM window,
PGDFS a byte stream over a two-port ISA I/O window.

## Ports and registers

Everything rides on the existing PicoGUS control protocol (`common/picogus.h`):
knock `CCh` on `CONTROL_PORT` (1D0h), write a register number, then move data
through `DATA_PORT_LOW` (1D1h) / `DATA_PORT_HIGH` (1D2h). PGDFS adds a two-port
data window and eight registers.

| Reg / port | Name | Access | Meaning |
|---|---|---|---|
| `1D4h`-`1D5h` (default) | data window | byte stream, 8- or 16-bit | Request bytes in (`rep outsw`), answer bytes out (`rep insw`). Two consecutive ports at an even base; both feed the same byte stream, so a word access carries stream byte n in the low byte (base) and n+1 in the high byte (base+1), and single-byte accesses to either port work as well. Direction follows the selected register. Base set with `CMD_DFSPORT`. |
| `80h` | `CMD_DFSSTAT` | read 1D2h / write 1D2h | Read: `dfs_status_t`. Any write aborts the current transaction and returns to IDLE. |
| `81h` | `CMD_DFSREQ` | select | Rewinds the write pointer. Subsequent data window writes append to the request frame. Status becomes RECEIVING. |
| `82h` | `CMD_DFSEXEC` | write 1D2h (any value) | Validates the frame (received bytes == declared length, length within limits) and sets BUSY; core 1 serves it. Invalid frame: ABORTED. |
| `83h` | `CMD_DFSRESP` | select | Rewinds the read pointer. Subsequent data window reads stream the answer frame. Valid only in READY. |
| `84h` | `CMD_DFSINFO` | read string 1D2h | Zero-terminated `LABEL|FS|<size MB>|<serial hex>`; empty string when no drive is mounted. Reading the terminator rewinds. |
| `85h` | `CMD_DFSMAXLEN` | read 16-bit 1D1h/1D2h | Largest payload (bytes, excluding header) the card accepts in one frame. Reads 0 when PGDFS is disabled (`CMD_DFSPORT` = 0). Firmware without PGDFS returns FF00h here (low byte 00h, high byte FFh); the driver treats anything outside 128..32768 as "not supported". |
| `86h` | `CMD_DFSTIME` | write 1D2h, 4 bytes | DOS packed time (lo, hi) then packed date (lo, hi), FAT format. Used for timestamps on files the card creates or modifies. |
| `87h` | `CMD_DFSPORT` | read/write 16-bit 1D1h/1D2h | Base of the data window, handled like `CMD_CDPORT`: write the low byte to 1D1h, then the high byte to 1D2h, which commits; reads return the current value, 0 when disabled. Bit 0 is ignored (the base is forced even). 0 disables PGDFS: `CMD_DFSMAXLEN` reads 0 and the window is not decoded. Bases below 100h, above 3FEh, or whose window overlaps 1D0h-1D3h are rejected and the previous value kept. Stored in the card's settings (`pgusinit /dfsport`, persisted by `pgusinit /save`); default 1D4h. |

Protocol version (`CMD_PROTOCOL`) is 5 with PGDFS present. The driver requires `>= 5`.

Reads of `1D2h` and of the data window hold IOCHRDY only for the time it takes
core 0 to fetch one byte from RAM. Writes to the data window use the
IOCHRDY-stalled path as well, so the PIO FIFO can never overflow during
`rep outsw`. Core 0 never waits on USB or FatFs inside a bus cycle; the DOS side
waits on the status byte instead.

### Why a word-wide window on an 8-bit card

The PicoGUS is an 8-bit ISA card, so it never sees a 16-bit bus cycle. When
the CPU executes `out dx, ax` / `in ax, dx` (or `rep outsw` / `rep insw`) on an
8-bit port, the 8088 itself, or the bus controller of a 286+ motherboard,
splits the access into two 8-bit cycles, the base port with the low byte and
then base+1 with the high byte, with no extra CPU work. Both ports of the
window feed the same stream, so every iteration moves two stream bytes and the
CPU cost per byte roughly halves compared with `rep outsb` / `rep insb`.
`insw`/`outsw` are 186+ instructions; an 8086/8088 driver gets the same effect
from `in ax, dx` / `stosw` and `lodsw` / `out dx, ax` loops.

The card decodes the other emulated devices (GUS, SB, MPU-401, CD-ROM, ...)
before the PGDFS window, so a window placed on one of their ports is shadowed
by that device: pick a free even base, as for any other card in the machine.

## Status machine

```
            CMD_DFSREQ                CMD_DFSEXEC (ok)          core 1 done
  IDLE ---------------> RECEIVING -----------------------> BUSY ------------> READY
   ^                       |                                                  |
   |   CMD_DFSEXEC (bad)   v                                                  |
   +---------------- ABORTED <--------- CMD_DFSSTAT write (from any state) <--+
   |
   +--- NODRIVE: reported instead of IDLE while no USB drive is mounted
```

* `IDLE (0)`: nothing in flight. Selecting `CMD_DFSREQ` starts a transaction.
* `RECEIVING (1)`: the card is collecting request bytes.
* `BUSY (2)`: core 1 owns the buffer and is talking to FatFs/USB.
* `READY (3)`: the answer frame is in the buffer; select `CMD_DFSRESP` and read.
  The next `CMD_DFSREQ` discards it.
* `ABORTED (FEh)`: the last request was rejected (length mismatch, oversize) or
  aborted through `CMD_DFSSTAT`. Sticky until the next `CMD_DFSREQ`.
* `NODRIVE (FFh)`: no USB drive is mounted; reported in place of IDLE and
  ABORTED. Requests are refused. Also what firmware without PGDFS returns for
  an unknown register, so the driver must check `CMD_DFSMAXLEN` (FF00h without
  PGDFS, 0 with PGDFS disabled) and the protocol version at install, not this
  byte.

A write to `CMD_DFSSTAT` while BUSY cannot stop core 1: the status keeps
reading BUSY until core 1 finishes, then it becomes ABORTED (the result is
dropped through a generation counter). A driver that gave up on a request
therefore sees BUSY on its next transaction and simply waits for it; it
never streams a new frame into a buffer the card is still writing. A
`CMD_DFSREQ` or `CMD_DFSEXEC` arriving while BUSY is ignored.

## Frame format

Little-endian. The answer overwrites the request in the same buffer on both
sides, so the driver needs one buffer, not two.

```
request : LL LL DD AL  payload...
          LL LL = total frame length including this 4-byte header
          DD    = drive index in bits 0-4 (0 = the USB drive), flags in bits 5-7 (0)
          AL    = INT 2Fh/11h subfunction (EDF5 "query"), or a PGDFS extension

answer  : LL LL AX AX  payload...
          LL LL = total frame length including this 4-byte header
          AX AX = DOS result: 0 = success, else the DOS error code (2 = file not
                  found, 3 = path not found, 5 = access denied, 15h = drive not
                  ready, 12h = no more files, ...)
```

Maximum payload per frame is `CMD_DFSMAXLEN` (4096 by default). READ and WRITE
requests are chunked by the driver to fit; every other subfunction fits in one
frame.

## Transaction, driver side

`dfs_port` is the window base read from `CMD_DFSPORT` at install (0: PGDFS is
disabled, do not install). Frames move a word at a time; an odd byte count ends
with one byte access.

```
out 1D0h, CCh          ; knock
out 1D0h, 81h          ; CMD_DFSREQ: open request buffer
mov dx, dfs_port
mov cx, frame_len      ; the whole frame, header first
shr cx, 1              ; words; CF = one trailing byte
rep outsw              ; 186+ (8086/8088: lodsw / out dx, ax loop)
jnc @f
outsb
@@:
out 1D0h, 82h          ; CMD_DFSEXEC
out 1D2h, 01h
out 1D0h, 80h          ; CMD_DFSSTAT
loop: in al, 1D2h
      3 -> ready, FEh -> aborted, FFh -> no drive, else keep polling
      give up after ~30 s (BIOS tick count): abort via out 1D2h, 0 and fail with error 15h
out 1D0h, 83h          ; CMD_DFSRESP
mov dx, dfs_port
in ax, dx              ; header: frame length
mov cx, ax
in ax, dx              ; header: AX result
sub cx, 4
shr cx, 1
rep insw               ; payload (8086/8088: in ax, dx / stosw loop)
jnc @f
insb
@@:
```

The driver never re-sends a request that got READY. On ABORTED it re-sends
once (a lost byte during a bus glitch, or the card finishing a request the
driver had already given up on); a second ABORTED is reported to DOS as
error 15h (drive not ready). The poll timeout is about 30 s: an unplugged
drive is reported at once as NODRIVE, so the timeout only matters for hangs,
and legal work such as a wildcard DELETE over hundreds of files can take
several seconds.

The knock plus register select is part of every transaction because another
program (pgusinit) may have changed the selected register in between.

## Time

At install, and on `PGDFS /T`, the driver writes the DOS clock through
`CMD_DFSTIME`. Core 0 collects the four bytes and hands them to core 1,
which keeps `dos_time + elapsed` and returns it from `get_fattime()`. Without it, files created from DOS carry the FatFs default
timestamp (1 Jan 1980 or the firmware's fixed value).

## Drive info

`CMD_DFSINFO` returns `LABEL|FAT32|30528|1A2B3C4D` style strings: volume label
(may be empty), filesystem (`FAT12`/`FAT16`/`FAT32`/`EXFAT`), size in MB,
volume serial in hex. `pgusinit` shows it; `PGDFS` shows it at install.

## Subfunctions (EDF5 payloads)

Payloads are unchanged from EtherDFS. Summary, from EtherDFS `protocol.txt`
(Mateusz Viste, MIT). "SS" is the 16-bit file id the server assigned at OPEN
(EtherDFS calls it "starting sector"); "CC" is the 16-bit directory id and "pp"
the position used to continue a search.

| AL | Name | Request payload | Answer payload |
|---|---|---|---|
| 01h | RMDIR | path (`\DIR\SUB`) | - |
| 03h | MKDIR | path | - |
| 05h | CHDIR | path (existence check) | - |
| 06h | CLOSEFILE | SS | - |
| 08h | READFILE | OOOO offset, SS, LL length | data (may be shorter than LL at EOF) |
| 09h | WRITEFILE | OOOO offset, SS, data... | LL bytes written (LL=0 with data length 0 truncates at offset) |
| 0Ah/0Bh | LOCK/UNLOCK | NN count, SS, (OOOO ZZZZ)* | - (always success) |
| 0Ch | DISKSPACE | - | BX total clusters, CX bytes/sector, DX free clusters; AX = sectors/cluster (1) |
| 0Eh | SETATTR | A attr, path | - |
| 0Fh | GETATTR | path | tt time, dd date, ssss size, A attr |
| 11h | RENAME | L srclen, src path, dst path | - |
| 13h | DELETE | path (wildcards allowed) | - |
| 16h/17h/2Eh | OPEN/CREATE/SPOPNFIL | SS stack word (open mode for 16h, attributes for 17h/2Eh), CC action, MM mode, path | A attr, 11-byte FCB name, tt, dd, ssss size, CC file id, RR result (1 opened, 2 created, 3 truncated), o open mode |
| 1Bh | FINDFIRST | A attr mask, path with mask (`\DIR\FILE????.???`) | A attr, 11-byte FCB name, tt, dd, ssss, CC dir id, pp position |
| 1Ch | FINDNEXT | CC dir id, pp position, A attr, 11-byte FCB mask | same as FINDFIRST |
| 21h | SEEKFROMEND | oooo offset from end, SS | oooo offset from start |
| 24h | SETFILETIMESTAMP | tt, dd, SS | - |
| F0h | ECHO (PGDFS) | any bytes | the same bytes |
| F1h | LONGNAME (PGDFS) | path as DOS sees it: drive-relative, backslashes, 8.3 names (`\DIR\LONGNA~1.EXT`), no wildcards | the entry's long file name, no terminator (the short name itself when the entry has none, empty for the root); AX = 0, or GETATTR's errors: 2 file not found, 3 path not found, 15h no drive |

Behaviour that the MS-DOS side relies on (all inherited from ethersrv-linux):
FINDFIRST in a non-root directory returns `.` and `..` first; a failing
FINDFIRST returns 12h (no more files), not 02h; DISKSPACE totals are capped just
under 2 GB; deleting a read-only file returns 05h; RENAME onto an existing name
fails; file and directory names are matched case-insensitively and reported as
8.3 names (FatFs short names, so long names appear as `LONGNA~1.EXT`).

## Long file names and the code page

The INT 2Fh/11h redirector interface is 8.3-only, so PGDFS always reports the
FatFs short name (`LONGNA~1.EXT` for an entry that carries a long name) and
DOS programs open files by that alias. `LONGNAME` (F1h) is the way back: given
the 8.3 path, it returns the entry's long name so a tool can show it
(`PGDFSTST /LDIR`). The card finds the entry by scanning its directory and
resumes that scan on the next lookup, so asking for a directory's entries in
listing order costs about one directory read per name. Names travel in the
FatFs OEM code page, a firmware build option (`-DFATFS_CODE_PAGE=437` by
default; 850 Western Europe, 865 Nordic, ...): it decides how the long names on
the drive, stored as UTF-16 by Windows, become the 8-bit names DOS sees, and
how DOS names are matched against them. A long name with characters outside the
code page is reported as its 8.3 alias. Build the firmware with the code page
the DOS machine runs (`CHCP`).
