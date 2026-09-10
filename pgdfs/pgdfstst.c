/*
 * PGDFSTST - PicoGUS PGDFS transport and protocol test tool
 *
 * Copyright (C) 2026 PicoGUS contributors
 * Distributed under the MIT license, see LICENSE.
 *
 * A plain (non-resident) DOS program that talks to the PicoGUS through the
 * same transport module as the PGDFS TSR (xport.c) and exercises the EDF5
 * requests described in sw/dfs/PROTOCOL.md. It is meant as the protocol
 * conformance check for the firmware, so every failure is reported with the
 * status byte, AX and the lengths involved.
 *
 *   PGDFSTST /INFO            card, protocol, max frame payload, drive info
 *   PGDFSTST /ECHO [n]        echo 64/512/4096-byte payloads n times, verify
 *   PGDFSTST /DIR [path]      FINDFIRST/FINDNEXT listing
 *   PGDFSTST /TYPE file       READ a file to stdout
 *   PGDFSTST /GET remote local  copy a file from the USB drive
 *   PGDFSTST /PUT local remote  copy a file to the USB drive
 *   PGDFSTST /TIME            push the DOS clock to the card
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <i86.h>
#include "../common/picogus.h"
#include "xport.h"
#include "version.h"

/* EDF5 subfunctions used here (AL values of the INT 2Fh/11h calls) */
#define AL_CLSFIL     0x06
#define AL_READFIL    0x08
#define AL_WRITEFIL   0x09
#define AL_DISKSPACE  0x0C
#define AL_OPEN       0x16
#define AL_CREATE     0x17
#define AL_FINDFIRST  0x1B
#define AL_FINDNEXT   0x1C
#define AL_ECHO       0xF0  /* PGDFS extension */

#define TICKS_PER_SEC 18.2065

static unsigned char buf[XPORT_FRAME_SIZE];
static unsigned short chunk = XPORT_MAX_PAYLOAD; /* largest payload per frame */

static const char *xport_errname(int r) {
  switch (r) {
    case XPORT_OK: return "ok";
    case XPORT_NODRIVE: return "NODRIVE (no USB drive mounted, or no PGDFS)";
    case XPORT_ABORTED: return "ABORTED twice (request rejected by the card)";
    case XPORT_TIMEOUT: return "TIMEOUT (no READY status within 5 s)";
    case XPORT_BADLEN: return "BADLEN (answer header announces an impossible length)";
    case XPORT_TOOLONG: return "TOOLONG (request does not fit the buffer)";
  }
  return "unknown";
}

static const char *statusname(unsigned char st) {
  switch (st) {
    case DFS_STATUS_IDLE: return "IDLE";
    case DFS_STATUS_RECEIVING: return "RECEIVING";
    case DFS_STATUS_BUSY: return "BUSY";
    case DFS_STATUS_READY: return "READY";
    case DFS_STATUS_ABORTED: return "ABORTED";
    case DFS_STATUS_NODRIVE: return "NODRIVE";
  }
  return "?";
}

/* sends one request and reads the answer. the payload is either copied from
 * payload (plen bytes), or, when payload is NULL, expected to be in place at
 * buf+4 already. returns the answer payload length (>= 0) with *ax set to
 * the DOS result, or -1 on transport failure (already reported). */
static int query(unsigned char al, const void *payload, unsigned short plen, unsigned short *ax) {
  unsigned short len = plen + DFS_HDR_LEN;
  int r;
  if (len > sizeof(buf)) {
    printf("ERROR: request of %u bytes does not fit the %u-byte frame buffer\n", len, (unsigned)sizeof(buf));
    return -1;
  }
  buf[0] = len & 0xFF;
  buf[1] = len >> 8;
  buf[2] = 0;   /* drive index 0 = the USB drive, no flags */
  buf[3] = al;
  if ((payload != NULL) && (plen != 0)) memcpy(buf + DFS_HDR_LEN, payload, plen);
  r = xport_transact(buf, sizeof(buf));
  if (r != XPORT_OK) {
    printf("ERROR: AL=%02Xh request (%u bytes): %s, last status byte %02Xh (%s)%s\n",
           al, len, xport_errname(r), xport_last_status, statusname(xport_last_status),
           xport_retried ? ", re-sent once" : "");
    return -1;
  }
  *ax = buf[2] | (buf[3] << 8);
  len = buf[0] | (buf[1] << 8);
  return len - DFS_HDR_LEN;
}

/* like query(), but also treats AX != 0 as failure and reports it */
static int query_ok(unsigned char al, const void *payload, unsigned short plen, const char *what) {
  unsigned short ax;
  int len = query(al, payload, plen, &ax);
  if (len < 0) return -1;
  if (ax != 0) {
    printf("ERROR: %s: AL=%02Xh answered AX=%04Xh (DOS error %u), %d payload bytes\n", what, al, ax, ax, len);
    return -1;
  }
  return len;
}

/* card presence, protocol version and CMD_DFSMAXLEN check shared by every
 * command. returns 0 when PGDFS is usable. */
static int checkcard(int verbose) {
  unsigned char proto, st;
  unsigned short maxlen;
  xport_detect_cpu();
  if (verbose) printf("CPU: %s\n", xport_cpu186 ? "80186 or later (using REP INSB/OUTSB)" : "8086/8088 (using IN/OUT loops)");
  if (!xport_present()) {
    printf("ERROR: PicoGUS not detected (CMD_MAGIC on port %03Xh/%03Xh did not answer DDh)\n", CONTROL_PORT, DATA_PORT_HIGH);
    return 1;
  }
  proto = xport_read8(CMD_PROTOCOL);
  maxlen = xport_read16(CMD_DFSMAXLEN);
  st = xport_status();
  if (verbose) {
    printf("PicoGUS detected, protocol version %u (need >= %u)\n", proto, PICOGUS_PROTOCOL_VER);
    printf("CMD_DFSMAXLEN: %u (%04Xh)\n", maxlen, maxlen);
    printf("CMD_DFSSTAT:   %02Xh (%s)\n", st, statusname(st));
  }
  if (proto < PICOGUS_PROTOCOL_VER) {
    printf("ERROR: firmware protocol %u is too old for PGDFS (need %u)\n", proto, PICOGUS_PROTOCOL_VER);
    return 1;
  }
  if ((maxlen < XPORT_MAXLEN_MIN) || (maxlen > XPORT_MAXLEN_MAX)) {
    printf("ERROR: CMD_DFSMAXLEN=%u is outside %u..%u: this firmware has no PGDFS support\n", maxlen, XPORT_MAXLEN_MIN, XPORT_MAXLEN_MAX);
    return 1;
  }
  if (maxlen < chunk) chunk = maxlen;
  if (verbose) printf("Frame payload used by this tool: %u bytes\n", chunk);
  return 0;
}

static void printinfo(void) {
  char info[256];
  char *p, *label, *fs, *mb, *serial;
  if (xport_info(info, sizeof(info)) == 0) {
    printf("USB drive:     none mounted (CMD_DFSINFO is empty)\n");
    return;
  }
  printf("CMD_DFSINFO:   \"%s\"\n", info);
  label = info;
  fs = mb = serial = "";
  p = strchr(info, '|');
  if (p != NULL) { *p++ = 0; fs = p; p = strchr(p, '|'); }
  if (p != NULL) { *p++ = 0; mb = p; p = strchr(p, '|'); }
  if (p != NULL) { *p++ = 0; serial = p; }
  printf("USB drive:     label \"%s\", %s, %s MB, serial %s\n", label, fs, mb, serial);
}

static int cmd_info(void) {
  unsigned short ax;
  int len;
  printf("PGDFSTST v%s - PicoGUS PGDFS test tool\n", PVER);
  if (checkcard(1) != 0) return 1;
  printinfo();
  /* a DISKSPACE query is the cheapest end-to-end check */
  len = query(AL_DISKSPACE, NULL, 0, &ax);
  if (len < 0) return 1;
  if (len == 6) {
    unsigned short bx = buf[4] | (buf[5] << 8), cx = buf[6] | (buf[7] << 8), dx = buf[8] | (buf[9] << 8);
    printf("DISKSPACE:     %u sectors/cluster, %u bytes/sector, %u total clusters, %u free\n", ax, cx, bx, dx);
    printf("               %lu KB total, %lu KB free\n",
           ((unsigned long)bx * cx / 1024) * ax, ((unsigned long)dx * cx / 1024) * ax);
  } else {
    printf("WARNING: DISKSPACE answered %d payload bytes (expected 6), AX=%04Xh\n", len, ax);
  }
  printf("PGDFS ready.\n");
  return 0;
}

static int cmd_echo(int rounds) {
  static const unsigned short sizes[3] = {64, 512, 4096};
  int s, r, len, fails = 0;
  unsigned short ax, size, i;
  unsigned short t0, t1;
  if (checkcard(0) != 0) return 1;
  for (s = 0; s < 3; s++) {
    size = sizes[s];
    if (size > chunk) size = chunk;
    printf("ECHO %4u bytes x %d: ", size, rounds);
    fflush(stdout);
    t0 = xport_ticks();
    for (r = 0; r < rounds; r++) {
      unsigned char seed = (unsigned char)(r * 31 + s * 7 + 1);
      for (i = 0; i < size; i++) buf[DFS_HDR_LEN + i] = (unsigned char)(i * 7 + seed);
      len = query(AL_ECHO, NULL, size, &ax);
      if (len < 0) { fails++; break; }
      if ((len != size) || (ax != 0)) {
        printf("\n  round %d: answer has %d payload bytes (sent %u), AX=%04Xh\n", r, len, size, ax);
        fails++;
        break;
      }
      for (i = 0; i < size; i++) {
        if (buf[DFS_HDR_LEN + i] != (unsigned char)(i * 7 + seed)) {
          printf("\n  round %d: data mismatch at offset %u: got %02Xh, expected %02Xh\n",
                 r, i, buf[DFS_HDR_LEN + i], (unsigned char)(i * 7 + seed));
          fails++;
          break;
        }
      }
      if (i != size) break;
    }
    t1 = xport_ticks();
    if (r == rounds) {
      double secs = (unsigned short)(t1 - t0) / TICKS_PER_SEC;
      double kb = (double)size * rounds / 1024.0;
      if (secs < 0.001) secs = 0.001;
      printf("ok, %.1f KB each way in %.2f s = %.1f KB/s (%.1f KB/s counting both directions)\n",
             kb, secs, kb / secs, 2 * kb / secs);
    }
  }
  if (fails) {
    printf("ECHO test FAILED (%d failure%s)\n", fails, fails == 1 ? "" : "s");
    return 1;
  }
  printf("ECHO test passed.\n");
  return 0;
}

/* normalizes a remote path: strips an optional drive letter, forces a
 * leading backslash, converts / to \ and upper-cases everything */
static void normpath(char *dst, const char *src, size_t max) {
  size_t n = 0;
  if ((src[0] != 0) && (src[1] == ':')) src += 2;
  if (*src != '\\' && *src != '/') dst[n++] = '\\';
  while ((*src != 0) && (n + 1 < max)) {
    char c = *src++;
    if (c == '/') c = '\\';
    dst[n++] = (char)toupper((unsigned char)c);
  }
  dst[n] = 0;
}

static void fcb2name(const unsigned char *fcb, char *name) {
  int i, n = 0;
  for (i = 0; i < 8 && fcb[i] != ' '; i++) name[n++] = fcb[i];
  if (fcb[8] != ' ') {
    name[n++] = '.';
    for (i = 8; i < 11 && fcb[i] != ' '; i++) name[n++] = fcb[i];
  }
  name[n] = 0;
}

static void printentry(const unsigned char *e) {
  /* e -> AfffffffffffttddssssCCpp (24 bytes) */
  char name[13];
  unsigned char attr = e[0];
  unsigned short tm = e[12] | (e[13] << 8), dt = e[14] | (e[15] << 8);
  unsigned long size = (unsigned long)e[16] | ((unsigned long)e[17] << 8) | ((unsigned long)e[18] << 16) | ((unsigned long)e[19] << 24);
  fcb2name(e + 1, name);
  printf("%-12s %c%c%c%c%c ", name,
         (attr & 0x10) ? 'D' : '-', (attr & 0x01) ? 'R' : '-', (attr & 0x02) ? 'H' : '-',
         (attr & 0x04) ? 'S' : '-', (attr & 0x20) ? 'A' : '-');
  if (attr & 0x10) printf("     <DIR> "); else printf("%10lu ", size);
  printf("%04u-%02u-%02u %02u:%02u:%02u\n",
         (dt >> 9) + 1980, (dt >> 5) & 15, dt & 31, tm >> 11, (tm >> 5) & 63, (tm & 31) * 2);
}

static int cmd_dir(const char *path) {
  char dir[128], mask[160];
  unsigned char req[32];
  unsigned short ax, dirid, pos;
  int len, n = 0;
  size_t l;
  if (checkcard(0) != 0) return 1;
  normpath(dir, (path != NULL) ? path : "\\", sizeof(dir));
  l = strlen(dir);
  if ((l > 1) && (dir[l - 1] == '\\')) dir[l - 1] = 0;
  sprintf(mask, "%s\\????????.???", (strcmp(dir, "\\") == 0) ? "" : dir);
  printf("Directory of %s\n\n", dir);
  /* FINDFIRST: A + path with mask */
  req[0] = 0x16; /* look for hidden, system and directories, too */
  memcpy(buf + DFS_HDR_LEN, req, 1);
  memcpy(buf + DFS_HDR_LEN + 1, mask, strlen(mask));
  len = query(AL_FINDFIRST, NULL, 1 + strlen(mask), &ax);
  if (len < 0) return 1;
  if (ax != 0) {
    if (ax == 0x12) {
      printf("(empty)\n");
      return 0;
    }
    printf("ERROR: FINDFIRST \"%s\" answered AX=%04Xh (DOS error %u), %d payload bytes\n", mask, ax, ax, len);
    return 1;
  }
  for (;;) {
    if (len != 24) {
      printf("ERROR: FIND answer has %d payload bytes, expected 24\n", len);
      return 1;
    }
    printentry(buf + DFS_HDR_LEN);
    n++;
    dirid = buf[DFS_HDR_LEN + 20] | (buf[DFS_HDR_LEN + 21] << 8);
    pos = buf[DFS_HDR_LEN + 22] | (buf[DFS_HDR_LEN + 23] << 8);
    /* FINDNEXT: CC pp A + 11-byte template */
    req[0] = dirid & 0xFF; req[1] = dirid >> 8;
    req[2] = pos & 0xFF;   req[3] = pos >> 8;
    req[4] = 0x16;
    memset(req + 5, '?', 11);
    len = query(AL_FINDNEXT, req, 16, &ax);
    if (len < 0) return 1;
    if (ax != 0) {
      if (ax == 0x12) break; /* no more files */
      printf("ERROR: FINDNEXT (dir %04Xh, pos %u) answered AX=%04Xh (DOS error %u)\n", dirid, pos, ax, ax);
      return 1;
    }
  }
  printf("\n%d entries\n", n);
  return 0;
}

/* OPEN (or CREATE) a remote file; returns the 16-bit file id, or -1 */
static long openremote(const char *rpath, int create, unsigned long *size) {
  unsigned char req[6];
  char path[128];
  int len;
  normpath(path, rpath, sizeof(path));
  /* SS = stack word (attributes), CC = action, MM = mode, then the path */
  req[0] = 0; req[1] = 0;                     /* attributes: normal file */
  req[2] = create ? 0x12 : 0x01; req[3] = 0; /* action: create/truncate or open existing */
  req[4] = create ? 0x02 : 0x00; req[5] = 0; /* mode: read/write or read-only */
  memcpy(buf + DFS_HDR_LEN, req, 6);
  memcpy(buf + DFS_HDR_LEN + 6, path, strlen(path));
  len = query_ok(create ? AL_CREATE : AL_OPEN, NULL, 6 + strlen(path), create ? "CREATE" : "OPEN");
  if (len < 0) return -1;
  if (len != 25) {
    printf("ERROR: OPEN/CREATE answer has %d payload bytes, expected 25\n", len);
    return -1;
  }
  if (size != NULL) {
    *size = (unsigned long)buf[DFS_HDR_LEN + 16] | ((unsigned long)buf[DFS_HDR_LEN + 17] << 8) |
            ((unsigned long)buf[DFS_HDR_LEN + 18] << 16) | ((unsigned long)buf[DFS_HDR_LEN + 19] << 24);
  }
  return buf[DFS_HDR_LEN + 20] | (buf[DFS_HDR_LEN + 21] << 8);
}

static int closeremote(unsigned short id) {
  unsigned char req[2];
  req[0] = id & 0xFF; req[1] = id >> 8;
  return (query_ok(AL_CLSFIL, req, 2, "CLOSE") < 0) ? -1 : 0;
}

/* READ from offset; returns bytes read (in buf+4), or -1 */
static int readremote(unsigned short id, unsigned long offset, unsigned short want) {
  unsigned char req[8];
  int len;
  req[0] = offset & 0xFF; req[1] = (offset >> 8) & 0xFF; req[2] = (offset >> 16) & 0xFF; req[3] = (offset >> 24) & 0xFF;
  req[4] = id & 0xFF; req[5] = id >> 8;
  req[6] = want & 0xFF; req[7] = want >> 8;
  len = query_ok(AL_READFIL, req, 8, "READ");
  if (len < 0) return -1;
  if (len > want) {
    printf("ERROR: READ of %u bytes at offset %lu answered %d bytes\n", want, offset, len);
    return -1;
  }
  return len;
}

/* copies a remote file to out (stdout or a file). returns 0 on success */
static int dumpremote(const char *rpath, FILE *out) {
  long id;
  unsigned long size, offset = 0;
  int len;
  id = openremote(rpath, 0, &size);
  if (id < 0) return 1;
  for (;;) {
    len = readremote((unsigned short)id, offset, chunk);
    if (len < 0) { closeremote((unsigned short)id); return 1; }
    if (len == 0) break;
    if (fwrite(buf + DFS_HDR_LEN, 1, len, out) != (size_t)len) {
      printf("ERROR: local write failed\n");
      closeremote((unsigned short)id);
      return 1;
    }
    offset += len;
    if (len < chunk) break; /* short read = EOF */
  }
  if (closeremote((unsigned short)id) != 0) return 1;
  if (out != stdout) printf("%lu bytes copied (remote size %lu)\n", offset, size);
  if (offset != size) printf("WARNING: read %lu bytes but OPEN reported a size of %lu\n", offset, size);
  return 0;
}

static int cmd_type(const char *rpath) {
  if (checkcard(0) != 0) return 1;
  return dumpremote(rpath, stdout);
}

static int cmd_get(const char *rpath, const char *lpath) {
  FILE *f;
  int r;
  if (checkcard(0) != 0) return 1;
  f = fopen(lpath, "wb");
  if (f == NULL) {
    printf("ERROR: cannot create local file %s\n", lpath);
    return 1;
  }
  r = dumpremote(rpath, f);
  fclose(f);
  return r;
}

static int cmd_put(const char *lpath, const char *rpath) {
  FILE *f;
  long id;
  unsigned long offset = 0;
  unsigned short wchunk = chunk - 6;
  size_t n;
  int len;
  if (checkcard(0) != 0) return 1;
  wchunk = chunk - 6;
  f = fopen(lpath, "rb");
  if (f == NULL) {
    printf("ERROR: cannot open local file %s\n", lpath);
    return 1;
  }
  id = openremote(rpath, 1, NULL);
  if (id < 0) { fclose(f); return 1; }
  for (;;) {
    unsigned short written;
    n = fread(buf + DFS_HDR_LEN + 6, 1, wchunk, f);
    if (n == 0) break;
    /* OOOO SS data... */
    buf[DFS_HDR_LEN + 0] = offset & 0xFF; buf[DFS_HDR_LEN + 1] = (offset >> 8) & 0xFF;
    buf[DFS_HDR_LEN + 2] = (offset >> 16) & 0xFF; buf[DFS_HDR_LEN + 3] = (offset >> 24) & 0xFF;
    buf[DFS_HDR_LEN + 4] = id & 0xFF; buf[DFS_HDR_LEN + 5] = (id >> 8) & 0xFF;
    len = query_ok(AL_WRITEFIL, NULL, 6 + n, "WRITE");
    if (len < 0) { fclose(f); closeremote((unsigned short)id); return 1; }
    if (len != 2) {
      printf("ERROR: WRITE answer has %d payload bytes, expected 2\n", len);
      fclose(f); closeremote((unsigned short)id); return 1;
    }
    written = buf[DFS_HDR_LEN] | (buf[DFS_HDR_LEN + 1] << 8);
    if (written != n) {
      printf("ERROR: WRITE of %u bytes at offset %lu wrote only %u\n", (unsigned)n, offset, written);
      fclose(f); closeremote((unsigned short)id); return 1;
    }
    offset += written;
    if (n < wchunk) break;
  }
  fclose(f);
  if (closeremote((unsigned short)id) != 0) return 1;
  printf("%lu bytes copied\n", offset);
  return 0;
}

static int cmd_time(void) {
  unsigned short t, d;
  if (checkcard(0) != 0) return 1;
  xport_getdostime(&t, &d);
  xport_settime(t, d);
  printf("Sent DOS time %04Xh (%02u:%02u:%02u) and date %04Xh (%04u-%02u-%02u) to the card\n",
         t, t >> 11, (t >> 5) & 63, (t & 31) * 2, d, (d >> 9) + 1980, (d >> 5) & 15, d & 31);
  return 0;
}

static void usage(void) {
  printf("PGDFSTST v%s - PicoGUS PGDFS transport and protocol test tool\n\n", PVER);
  printf("  PGDFSTST /INFO              card, protocol, frame size, USB drive info\n");
  printf("  PGDFSTST /ECHO [n]          echo 64/512/4096-byte payloads n times (default 10)\n");
  printf("  PGDFSTST /DIR [path]        list a directory (FINDFIRST/FINDNEXT)\n");
  printf("  PGDFSTST /TYPE file         show a file (READ)\n");
  printf("  PGDFSTST /GET remote local  copy a file from the USB drive\n");
  printf("  PGDFSTST /PUT local remote  copy a file to the USB drive\n");
  printf("  PGDFSTST /TIME              push the DOS clock to the card\n\n");
  printf("Remote paths are relative to the root of the USB drive, e.g. \\DIR\\FILE.TXT\n");
}

int main(int argc, char **argv) {
  const char *cmd;
  if (argc < 2) {
    usage();
    return 1;
  }
  cmd = argv[1];
  if ((cmd[0] == '/') || (cmd[0] == '-')) cmd++;
  if (stricmp(cmd, "INFO") == 0) return cmd_info();
  if (stricmp(cmd, "ECHO") == 0) {
    int n = (argc > 2) ? atoi(argv[2]) : 10;
    if (n < 1) n = 1;
    return cmd_echo(n);
  }
  if (stricmp(cmd, "DIR") == 0) return cmd_dir((argc > 2) ? argv[2] : NULL);
  if ((stricmp(cmd, "TYPE") == 0) && (argc > 2)) return cmd_type(argv[2]);
  if ((stricmp(cmd, "GET") == 0) && (argc > 3)) return cmd_get(argv[2], argv[3]);
  if ((stricmp(cmd, "PUT") == 0) && (argc > 3)) return cmd_put(argv[2], argv[3]);
  if (stricmp(cmd, "TIME") == 0) return cmd_time();
  usage();
  return 1;
}
