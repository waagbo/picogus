/*
 * PGDFS - PicoGUS DOS file system: ISA I/O port transport
 *
 * Copyright (C) 2026 PicoGUS contributors
 * Distributed under the MIT license, see LICENSE.
 *
 * This module moves EDF5 frames between DOS and the PicoGUS card through the
 * PicoGUS control port protocol (knock on 1D0h, select a register, move data
 * through 1D1h/1D2h) plus the DFS_DATA_PORT byte stream on 1D3h. The wire
 * protocol is described in sw/dfs/PROTOCOL.md; register numbers and status
 * values come from common/picogus.h.
 *
 * The code obeys the TSR rules of PGDFS.C: no libc calls, no static
 * initializers that need startup code, only inp()/outp() intrinsics and
 * inline assembly. PGDFS.C #includes XPORT.C into its resident (BEGTEXT)
 * segment; PGDFSTST.C links the same file as a normal module.
 */

#ifndef XPORT_H_SENTINEL
#define XPORT_H_SENTINEL

#include "../common/picogus.h"

/* largest payload the driver ever puts in one frame, and the buffer size a
 * caller of xport_transact() needs for it */
#define XPORT_MAX_PAYLOAD DFS_DEFAULT_MAX_PAYLOAD
#define XPORT_FRAME_SIZE (DFS_HDR_LEN + XPORT_MAX_PAYLOAD)

/* how long to wait for the card to answer: BIOS ticks at 18.2 Hz, ~5 s */
#define XPORT_TIMEOUT_TICKS 91

/* CMD_DFSMAXLEN outside this range means "no PGDFS in this firmware" */
#define XPORT_MAXLEN_MIN 128
#define XPORT_MAXLEN_MAX 32768u

/* results of xport_transact() */
#define XPORT_OK       0
#define XPORT_NODRIVE  1  /* status NODRIVE: no USB drive mounted (or no card) */
#define XPORT_ABORTED  2  /* request rejected twice */
#define XPORT_TIMEOUT  3  /* no READY within XPORT_TIMEOUT_TICKS, card aborted */
#define XPORT_BADLEN   4  /* answer header announces an impossible length */
#define XPORT_TOOLONG  5  /* request frame does not fit the buffer */

/* 0 = 8086/8088 (IN/OUT + STOSB/LODSB loops), 1 = 80186+ (REP INSB/OUTSB).
 * set by xport_detect_cpu() and read by the byte movers. */
extern unsigned char xport_cpu186;

/* diagnostics: the last status byte read from CMD_DFSSTAT, and whether the
 * last transaction had to be re-sent after an ABORTED status */
extern unsigned char xport_last_status;
extern unsigned char xport_retried;

/* detects the CPU (push sp / pop ax test), stores and returns the result */
unsigned char xport_detect_cpu(void);

/* knock on the control port and select register reg */
void xport_select(unsigned char reg);

/* select reg and read one byte from DATA_PORT_HIGH */
unsigned char xport_read8(unsigned char reg);

/* select reg and read a 16-bit value through DATA_PORT_LOW (as pgusinit does) */
unsigned short xport_read16(unsigned char reg);

/* returns non-zero if a PicoGUS answers with the magic byte */
int xport_present(void);

/* low word of the BIOS tick counter at 0040:006Ch */
unsigned short xport_ticks(void);

/* move n bytes from src to DFS_DATA_PORT / from DFS_DATA_PORT to dst.
 * src and dst are near pointers into the data segment. */
void xport_out_bytes(const unsigned char *src, unsigned short n);
void xport_in_bytes(unsigned char *dst, unsigned short n);

/* read the status byte (also stored in xport_last_status) */
unsigned char xport_status(void);

/* abort whatever transaction the card has in flight */
void xport_abort(void);

/* run one transaction: buf holds a complete request frame (4-byte header
 * with the total length, then the payload); on XPORT_OK it holds the answer
 * frame instead (header with total length and AX, then the payload).
 * bufsize is the size of buf; the answer is never allowed to exceed it.
 * An ABORTED status makes the request go out a second time before failing. */
int xport_transact(unsigned char *buf, unsigned short bufsize);

/* copy the CMD_DFSINFO string into dst (at most max-1 characters plus a
 * terminating zero). returns the string length; 0 = no drive mounted. */
unsigned short xport_info(char *dst, unsigned short max);

/* write the DOS clock in FAT packed format through CMD_DFSTIME */
void xport_settime(unsigned short dostime, unsigned short dosdate);

/* read the DOS clock (INT 21h/2Ch and 2Ah) and pack it in FAT format:
 * time = hour<<11 | min<<5 | sec/2, date = (year-1980)<<9 | month<<5 | day */
void xport_getdostime(unsigned short *dostime, unsigned short *dosdate);

#endif
