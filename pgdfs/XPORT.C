/*
 * PGDFS - PicoGUS DOS file system: ISA I/O port transport
 *
 * Copyright (C) 2026 PicoGUS contributors
 * Distributed under the MIT license, see LICENSE.
 *
 * See XPORT.H for the interface and sw/dfs/PROTOCOL.md for the protocol.
 *
 * RESIDENT CODE RULES (this file is #included into the BEGTEXT segment of
 * PGDFS.C): no libc calls of any kind, no string literals, no static
 * initializers beyond plain zero, no stack checks (-s). inp()/outp() are
 * compiler intrinsics and compile to IN/OUT instructions. The code must run
 * on an 8086, so REP INSB/OUTSB (80186+) are only used after
 * xport_detect_cpu() said so, and they are emitted as raw bytes because the
 * assembler rejects them with -0.
 */

#include <conio.h>  /* inp() / outp() / inpw() */
#include <i86.h>    /* MK_FP() */
#include "xport.h"

/* force IN/OUT instructions instead of libc calls, whatever the -o options */
#pragma intrinsic(inp, inpw, outp, outpw)

unsigned char xport_cpu186;
unsigned char xport_last_status;
unsigned char xport_retried;

unsigned char xport_detect_cpu(void) {
  unsigned char r = 0;
  /* the 8086/8088 push SP after decrementing it, every later CPU pushes the
   * value SP had before the push */
  _asm {
    push sp
    pop ax
    cmp ax, sp
    jne is8086
    mov r, 1
    is8086:
  }
  xport_cpu186 = r;
  return(r);
}

void xport_select(unsigned char reg) {
  outp(CONTROL_PORT, 0xCC); /* knock on the door... */
  outp(CONTROL_PORT, reg);  /* ...and select the register */
}

unsigned char xport_read8(unsigned char reg) {
  xport_select(reg);
  return(inp(DATA_PORT_HIGH));
}

unsigned short xport_read16(unsigned char reg) {
  xport_select(reg);
  return(inpw(DATA_PORT_LOW));
}

int xport_present(void) {
  return(xport_read8(CMD_MAGIC) == 0xDD);
}

unsigned short xport_ticks(void) {
  return(*(unsigned short volatile far *)MK_FP(0x40, 0x6C));
}

void xport_out_bytes(const unsigned char *src, unsigned short n) {
  unsigned short port = DFS_DATA_PORT;
  if (n == 0) return;
  if (xport_cpu186 != 0) {
    _asm {
      mov si, src
      mov cx, n
      mov dx, port
      cld
      db 0F3h, 6Eh   /* rep outsb (DS:SI -> port DX) */
    }
  } else {
    _asm {
      mov si, src
      mov cx, n
      mov dx, port
      cld
      outnext:
      lodsb
      out dx, al
      loop outnext
    }
  }
}

void xport_in_bytes(unsigned char *dst, unsigned short n) {
  unsigned short port = DFS_DATA_PORT;
  if (n == 0) return;
  if (xport_cpu186 != 0) {
    _asm {
      push es
      push ds
      pop es
      mov di, dst
      mov cx, n
      mov dx, port
      cld
      db 0F3h, 6Ch   /* rep insb (port DX -> ES:DI) */
      pop es
    }
  } else {
    _asm {
      push es
      push ds
      pop es
      mov di, dst
      mov cx, n
      mov dx, port
      cld
      innext:
      in al, dx
      stosb
      loop innext
      pop es
    }
  }
}

unsigned char xport_status(void) {
  xport_select(CMD_DFSSTAT);
  xport_last_status = inp(DATA_PORT_HIGH);
  return(xport_last_status);
}

void xport_abort(void) {
  xport_select(CMD_DFSSTAT);
  outp(DATA_PORT_HIGH, 0); /* any write to CMD_DFSSTAT aborts */
}

int xport_transact(unsigned char *buf, unsigned short bufsize) {
  unsigned short len, start;
  unsigned long polls;
  unsigned char st;
  int attempt;

  xport_retried = 0;
  len = buf[0] | (buf[1] << 8);
  if ((len < DFS_HDR_LEN) || (len > bufsize)) return(XPORT_TOOLONG);

  for (attempt = 0;; attempt++) {
    /* open the request buffer and stream the whole frame in */
    xport_select(CMD_DFSREQ);
    xport_out_bytes(buf, len);
    /* execute */
    xport_select(CMD_DFSEXEC);
    outp(DATA_PORT_HIGH, 1);
    /* poll the status until READY, ABORTED, NODRIVE or timeout. the poll
     * counter is a backstop for the (abnormal) case where interrupts are
     * off and the BIOS tick counter does not advance. */
    xport_select(CMD_DFSSTAT);
    start = xport_ticks();
    polls = 0;
    for (;;) {
      unsigned short now;
      st = inp(DATA_PORT_HIGH);
      if ((st == DFS_STATUS_READY) || (st == DFS_STATUS_ABORTED) || (st == DFS_STATUS_NODRIVE)) break;
      polls++;
      now = xport_ticks();
      if (now < start) start = now; /* the BIOS resets the tick count at midnight */
      if (((unsigned short)(now - start) >= XPORT_TIMEOUT_TICKS) || ((polls & 0x00FFFFFFul) == 0)) {
        xport_last_status = st;
        xport_abort();
        return(XPORT_TIMEOUT);
      }
    }
    xport_last_status = st;
    if (st == DFS_STATUS_READY) break;
    if (st == DFS_STATUS_NODRIVE) return(XPORT_NODRIVE);
    /* ABORTED: the request buffer is still intact (nothing has been read
     * back yet), so send it once more */
    if (attempt != 0) return(XPORT_ABORTED);
    xport_retried = 1;
  }

  /* fetch the answer: header first, then as many payload bytes as it says */
  xport_select(CMD_DFSRESP);
  xport_in_bytes(buf, DFS_HDR_LEN);
  len = buf[0] | (buf[1] << 8);
  if ((len < DFS_HDR_LEN) || (len > bufsize)) {
    xport_abort();
    return(XPORT_BADLEN);
  }
  xport_in_bytes(buf + DFS_HDR_LEN, len - DFS_HDR_LEN);
  return(XPORT_OK);
}

unsigned short xport_info(char *dst, unsigned short max) {
  unsigned short i = 0, n = 0;
  unsigned char c;
  xport_select(CMD_DFSINFO);
  /* the string is at most 255 bytes; reading the terminator rewinds the
   * card's pointer, selecting the register again rewinds it as well */
  for (n = 0; n < 255; n++) {
    c = inp(DATA_PORT_HIGH);
    if ((c == 0) || (c == 0xFF)) break; /* 0xFF: nobody home */
    if (i + 1 < max) dst[i++] = c;
  }
  if (max != 0) dst[i] = 0;
  return(i);
}

void xport_settime(unsigned short dostime, unsigned short dosdate) {
  xport_select(CMD_DFSTIME);
  outp(DATA_PORT_HIGH, dostime & 0xFF);
  outp(DATA_PORT_HIGH, dostime >> 8);
  outp(DATA_PORT_HIGH, dosdate & 0xFF);
  outp(DATA_PORT_HIGH, dosdate >> 8);
}

void xport_getdostime(unsigned short *dostime, unsigned short *dosdate) {
  /* (variable names avoid register and directive names like ss, dd) */
  unsigned char hr = 0, mn = 0, sc = 0, mo = 1, dy = 1;
  unsigned short yr = 1980;
  _asm {
    mov ah, 2Ch  /* get system time: CH=hour CL=min DH=sec DL=1/100 */
    int 21h
    mov hr, ch
    mov mn, cl
    mov sc, dh
    mov ah, 2Ah  /* get system date: CX=year DH=month DL=day */
    int 21h
    mov yr, cx
    mov mo, dh
    mov dy, dl
  }
  if (yr < 1980) yr = 1980;
  *dostime = (hr << 11) | (mn << 5) | (sc >> 1);
  *dosdate = ((yr - 1980) << 9) | (mo << 5) | dy;
}
