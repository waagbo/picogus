/*
 *  PGBOOT: floppy / hard disk image server for the boot ROM.
 *
 *  See PROTOCOL.md for the wire protocol and bootdisk.h for the API.
 *
 *  Threading (same model as PGDFS, sw/dfs/dfs.h):
 *    - bd_ctl_* run on core 0 inside an ISA bus cycle: O(1), no FatFs, no
 *      loops. They read and write the settings storage bound by bd_init()
 *      (the name buffers and the options byte) and bump a per-unit commit
 *      sequence number when a name is committed.
 *    - Everything else runs on core 1 (bd_tasks() from dfs_tasks(), the
 *      mount hooks from the USB glue, bd_process() from dfs_process()).
 *      Core 1 copies a name out of the settings buffer only between two
 *      reads of the commit sequence that agree, so a name core 0 is still
 *      writing is never half-applied for good: the commit that ends the
 *      write bumps the sequence and core 1 copies again.
 *
 *  Each unit keeps its image open in its own FIL with a fast seek cluster
 *  link map, so a random sector read costs one seek in the map instead of a
 *  walk down the FAT chain. Writes are synced before the answer.
 *
 *  Portable C: no Pico SDK headers, so the host tests and the emulator's
 *  card simulator build the same file.
 *
 *  Copyright (C) 2026  PicoGUS contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#include <string.h>
#include "ff.h"
#include "bootdisk.h"
#include "bd_geometry.h"

#if FF_FS_READONLY || FF_MAX_SS != 512
#error "PGBOOT needs FatFs with write support and 512-byte sectors"
#endif

#define BD_FENCE() __atomic_thread_fence(__ATOMIC_SEQ_CST)

/* Normalised path: a leading '\' is added to the configured name. The same
 * path spelled with 8.3 aliases can be longer ("a b" -> "AB~1"). */
#define BD_PATH_BUF     (BD_NAME_BUF + 1)
#define BD_SFN_BUF      (BD_NAME_BUF + 48)

/* Fast seek cluster link map sizes (DWORDs): 2 per fragment + 2. A floppy
 * image is small and rarely fragmented; a hard disk image gets more room.
 * An image with more fragments falls back to plain f_lseek(). */
#define BD_CLMT_FD      16
#define BD_CLMT_HD      64

/* ---- core 0 state (register handlers) ------------------------------------- */

static char    *cfg_name[BD_UNITS];         /* settings storage, BD_NAME_BUF each */
static uint8_t *cfg_opts;
static uint8_t  wr_pos[BD_UNITS];           /* core 0 only */
static uint8_t  rd_pos[BD_UNITS];           /* core 0 only */
static volatile uint32_t commit_seq[BD_UNITS];  /* written by core 0 only */

/* ---- core 1 state ---------------------------------------------------------- */

typedef struct {
    FIL       fil;
    bool      open;             /* fil is open on the mounted volume */
    bool      file_ro;          /* opened read-only (AM_RDO attribute or write-protected) */
    bool      vhd;              /* fixed VHD: the footer is not part of the disk */
    bool      changed;          /* media changed since the last BDINFO */
    uint8_t   state;            /* BD_STATE_* */
    uint8_t   gen;              /* media generation */
    uint32_t  applied_seq;      /* commit_seq value the path was copied at */
    bd_geom_t geom;
    char      path[BD_PATH_BUF];/* active image path, normalised; "" = none */
    char      sfn[BD_SFN_BUF];  /* the same by 8.3 aliases while open; "" = unknown */
} bd_unit_t;

static bd_unit_t units[BD_UNITS];
static bool      mounted;
#if FF_USE_FASTSEEK
static DWORD     clmt_fd[BD_CLMT_FD];
static DWORD     clmt_hd[BD_CLMT_HD];
#endif
static FILINFO   fno;               /* 8.3 alias lookup */
static bd_sec0_t sec0;              /* identification scratch */
static uint8_t   scratch[128];

static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)get16(p) | ((uint32_t)get16(p + 2) << 16); }
static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { put16(p, (uint16_t)v); put16(p + 2, (uint16_t)(v >> 16)); }

static char upper(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }
static bool is_sep(char c) { return c == '\\' || c == '/'; }

/* ================= core 0: control registers ================================ */

void bd_ctl_name_select(uint8_t unit) {
    if (unit >= BD_UNITS) return;
    wr_pos[unit] = 0;
    rd_pos[unit] = 0;
}

void bd_ctl_name_write(uint8_t unit, uint8_t c) {
    char *n;
    if (unit >= BD_UNITS || (n = cfg_name[unit]) == NULL) return;
    if (c == 0) {
        n[wr_pos[unit]] = 0;        /* wr_pos <= BD_NAME_MAX */
        wr_pos[unit] = 0;
        BD_FENCE();                 /* the name is complete before the commit shows */
        commit_seq[unit] = commit_seq[unit] + 1;
        return;
    }
    if (wr_pos[unit] < BD_NAME_MAX) {   /* longer names are cut, the ROM shows what stuck */
        n[wr_pos[unit]++] = (char)c;
        n[wr_pos[unit]] = 0;        /* always terminated, even before the commit */
    }
}

uint8_t bd_ctl_name_read(uint8_t unit) {
    const char *n;
    uint8_t p, c;
    if (unit >= BD_UNITS || (n = cfg_name[unit]) == NULL) return 0;
    p = rd_pos[unit];
    c = (p < BD_NAME_MAX) ? (uint8_t)n[p] : 0;
    rd_pos[unit] = c ? (uint8_t)(p + 1) : 0;    /* the terminator rewinds */
    return c;
}

uint8_t bd_ctl_opts_read(void) {
    return (uint8_t)((cfg_opts ? (*cfg_opts & BD_OPT_MASK) : 0) | BD_OPTS_SIGNATURE);
}

void bd_ctl_opts_write(uint8_t v) {
    if (cfg_opts) *cfg_opts = (uint8_t)(v & BD_OPT_MASK);
}

/* ================= core 1 ==================================================== */

static uint8_t opts_now(void) {
    return cfg_opts ? (uint8_t)(*(volatile uint8_t *)cfg_opts & BD_OPT_MASK) : 0;
}

static bool unit_ro(int u) {
    uint8_t bit = (u == BD_UNIT_FD) ? BD_OPT_FD_RO : BD_OPT_HD_RO;
    return units[u].file_ro || (opts_now() & bit);
}

/* "E:/Dir//img.IMA" -> "\Dir\img.IMA": drive letter dropped, one kind of
 * separator, no doubles, a leading one, no trailing one. "" stays "". */
static void norm_path(char *dst, size_t cap, const char *src) {
    size_t n = 0;
    if (src[0] == 0) {
        dst[0] = 0;
        return;
    }
    if (((src[0] >= 'A' && src[0] <= 'Z') || (src[0] >= 'a' && src[0] <= 'z')) && src[1] == ':') src += 2;
    dst[n++] = '\\';
    for (; *src && n < cap - 1; src++) {
        char c = is_sep(*src) ? '\\' : *src;
        if (c == '\\' && dst[n - 1] == '\\') continue;
        dst[n++] = c;
    }
    while (n > 1 && dst[n - 1] == '\\') n--;
    dst[n] = 0;
}

/* Copy unit u's configured name into its active path. */
static void fetch_name(int u) {
    bd_unit_t *un = &units[u];
    uint32_t s;
    do {
        s = commit_seq[u];
        BD_FENCE();
        if (cfg_name[u]) {
            norm_path(un->path, sizeof(un->path), cfg_name[u]);
        } else {
            un->path[0] = 0;
        }
        BD_FENCE();
    } while (s != commit_seq[u]);
    un->applied_seq = s;
}

static void forget_image(bd_unit_t *un) {
    un->open = false;
    un->file_ro = false;
    un->vhd = false;
    un->sfn[0] = 0;
    memset(&un->geom, 0, sizeof(un->geom));
#if FF_USE_FASTSEEK
    un->fil.cltbl = NULL;
#endif
}

static void close_image(bd_unit_t *un) {
    if (un->open && mounted) f_close(&un->fil);     /* never dirty: every write is synced */
    forget_image(un);
}

static void media_changed(bd_unit_t *un) {
    un->gen++;
    un->changed = true;
}

static bool read_at(FIL *fp, uint32_t ofs, void *dst, UINT len) {
    UINT br = 0;
    if (f_lseek(fp, ofs) != FR_OK) return false;
    return f_read(fp, dst, len, &br) == FR_OK && br == len;
}

/* Recognise the open image of unit u: format, data size and geometry. */
static bool identify(int u) {
    bd_unit_t *un = &units[u];
    FIL *fp = &un->fil;
    uint32_t size = (uint32_t)f_size(fp), data = size, sum = 0;
    bd_vhd_t vhd;
    int kind = BD_VHD_NONE;
    bool have_sec0 = false, ok;

#if FF_USE_FASTSEEK
    {
        DWORD *tbl = (u == BD_UNIT_FD) ? clmt_fd : clmt_hd;
        FRESULT fr;
        tbl[0] = (u == BD_UNIT_FD) ? BD_CLMT_FD : BD_CLMT_HD;
        fp->cltbl = tbl;
        fr = f_lseek(fp, CREATE_LINKMAP);
        if (fr == FR_NOT_ENOUGH_CORE) {
            fp->cltbl = NULL;           /* too fragmented for the map: plain seeks */
        } else if (fr != FR_OK) {
            fp->cltbl = NULL;
            return false;
        }
    }
#endif
    memset(&vhd, 0, sizeof(vhd));
    if (size >= 2 * BD_SECTOR && (size % BD_SECTOR) == 0) {
        /* the footer in pieces: bytes 128..511 for the checksum, then the head */
        uint32_t base = size - BD_SECTOR;
        for (uint32_t off = sizeof(scratch); off < BD_SECTOR; off += sizeof(scratch)) {
            if (!read_at(fp, base + off, scratch, sizeof(scratch))) return false;
            sum += bd_vhd_sum(scratch, off, sizeof(scratch));
        }
        if (!read_at(fp, base, scratch, sizeof(scratch))) return false;
        sum += bd_vhd_sum(scratch, 0, sizeof(scratch));
        kind = bd_vhd_check(scratch, sum, size, &vhd);
        if (kind == BD_VHD_OTHER) return false;         /* dynamic / differencing */
        if (kind == BD_VHD_FIXED) data = vhd.data_size;
    }
    if (data >= BD_SECTOR) {
        if (!read_at(fp, 0, sec0.head, BD_SEC0_HEAD_LEN)) return false;
        if (!read_at(fp, BD_SEC0_PTAB_OFF, sec0.ptab, BD_SEC0_PTAB_LEN)) return false;
        have_sec0 = true;
    }
    if (u == BD_UNIT_FD) {
        ok = bd_geom_floppy(data, have_sec0 ? &sec0 : NULL, &un->geom);
    } else {
        ok = bd_geom_harddisk(data, have_sec0 ? &sec0 : NULL, kind == BD_VHD_FIXED ? &vhd : NULL, &un->geom);
    }
    un->vhd = (kind == BD_VHD_FIXED);
    return ok;
}

/* The open image's path by 8.3 aliases, the form PGDFS paths arrive in. */
static void build_sfn(bd_unit_t *un) {
    char tmp[BD_PATH_BUF];
    size_t n = 0, i;

    un->sfn[0] = 0;
    strcpy(tmp, un->path);                          /* "\A\B.IMG" */
    for (i = 1; ; i++) {
        char c = tmp[i];
        const char *alias;
        size_t len;
        if (c != '\\' && c != 0) continue;
        tmp[i] = 0;
        if (f_stat(tmp, &fno) != FR_OK) {
            un->sfn[0] = 0;
            return;
        }
        tmp[i] = c;
        alias = fno.altname[0] ? fno.altname : fno.fname;
        len = strlen(alias);
        if (n + 1 + len >= sizeof(un->sfn)) {
            un->sfn[0] = 0;
            return;
        }
        un->sfn[n++] = '\\';
        memcpy(un->sfn + n, alias, len);
        n += len;
        un->sfn[n] = 0;
        if (c == 0) return;
    }
}

/* Close unit u and open its active path again. Always a media change. */
static void reopen(int u) {
    bd_unit_t *un = &units[u];
    FRESULT fr;

    close_image(un);
    media_changed(un);
    if (un->path[0] == 0) {
        un->state = BD_STATE_NONE;
        return;
    }
    if (!mounted) {
        un->state = BD_STATE_NODRIVE;
        return;
    }
    fr = f_open(&un->fil, un->path, FA_READ | FA_WRITE);
    if (fr == FR_DENIED || fr == FR_WRITE_PROTECTED) {  /* AM_RDO file or write-protected drive */
        fr = f_open(&un->fil, un->path, FA_READ);
        un->file_ro = true;
    }
    if (fr != FR_OK) {
        un->file_ro = false;
        un->state = (fr == FR_NO_FILE || fr == FR_NO_PATH || fr == FR_INVALID_NAME || fr == FR_INVALID_DRIVE)
                  ? BD_STATE_NOTFOUND : BD_STATE_UNUSABLE;
        return;
    }
    un->open = true;
    if (un->fil.obj.attr & AM_RDO) un->file_ro = true;
    if (!identify(u)) {
        close_image(un);
        un->state = BD_STATE_UNUSABLE;
        return;
    }
    build_sfn(un);
    un->state = BD_STATE_READY;
}

/* ---- setup ------------------------------------------------------------------ */

void bd_init(char *fd_name, char *hd_name, uint8_t *opts) {
    cfg_name[BD_UNIT_FD] = fd_name;
    cfg_name[BD_UNIT_HD] = hd_name;
    cfg_opts = opts;
    for (int u = 0; u < BD_UNITS; u++) {
        char *n = cfg_name[u];
        if (n) {
            n[BD_NAME_MAX] = 0;                     /* whatever flash held */
            if ((uint8_t)n[0] == 0xFF) n[0] = 0;    /* erased flash */
        }
        wr_pos[u] = 0;
        rd_pos[u] = 0;
        commit_seq[u] = 0;
        memset(&units[u], 0, sizeof(units[u]));
        fetch_name(u);
        units[u].state = units[u].path[0] ? BD_STATE_NODRIVE : BD_STATE_NONE;
    }
    if (opts) *opts &= BD_OPT_MASK;
    mounted = false;
}

/* ---- core 1 hooks ------------------------------------------------------------- */

void bd_tasks(void) {
    /* A floppy name commit swaps the disk at once; a hard disk name waits
     * for the next BDINFO with OPEN (the next boot). */
    if (commit_seq[BD_UNIT_FD] != units[BD_UNIT_FD].applied_seq) {
        fetch_name(BD_UNIT_FD);
        reopen(BD_UNIT_FD);
    }
}

void bd_on_drive_mounted(void) {
    mounted = true;
    for (int u = 0; u < BD_UNITS; u++) {
        if (units[u].path[0]) reopen(u);
    }
}

void bd_on_drive_unmounted(void) {
    /* The volume is gone: forget the FILs, no FatFs call. Only marks state,
     * so it is safe even if it ever runs underneath bd_process(). */
    mounted = false;
    for (int u = 0; u < BD_UNITS; u++) {
        bd_unit_t *un = &units[u];
        forget_image(un);
        if (un->path[0]) {
            un->state = BD_STATE_NODRIVE;
            media_changed(un);
        } else {
            un->state = BD_STATE_NONE;
        }
    }
}

/* ---- requests ------------------------------------------------------------------ */

/* The display name for the BDINFO record: the active path, or its tail with
 * a "..." in front when it is longer than BD_INFO_NAME_MAX. */
static size_t display_name(const bd_unit_t *un, char *out) {
    size_t n = strlen(un->path);
    if (n <= BD_INFO_NAME_MAX) {
        memcpy(out, un->path, n);
        return n;
    }
    memcpy(out, "...", 3);
    memcpy(out + 3, un->path + n - (BD_INFO_NAME_MAX - 3), BD_INFO_NAME_MAX - 3);
    return BD_INFO_NAME_MAX;
}

static uint16_t do_info(const uint8_t *req, uint16_t req_len, uint8_t *answ, uint16_t maxpl, uint16_t *ax) {
    uint8_t rec[BD_INFO_LEN];
    char name[BD_INFO_NAME_MAX];
    uint8_t u, flags = 0;
    bd_unit_t *un;
    size_t n;

    if (req_len < 1 || req[0] >= BD_UNITS || maxpl < BD_INFO_LEN) {
        *ax = BD_ST_BADCMD;
        return 0;
    }
    u = req[0];
    if (req_len >= 2 && (req[1] & BD_INFO_OPEN)) {
        fetch_name(u);
        reopen(u);
    }
    un = &units[u];

    memset(rec, 0, sizeof(rec));
    rec[BD_INFO_OFF_VERSION] = BD_INFO_VERSION;
    rec[BD_INFO_OFF_STATE] = un->state;
    if (un->state != BD_STATE_NONE) {
        rec[BD_INFO_OFF_TYPE] = (u == BD_UNIT_FD) ? BD_TYPE_FLOPPY : BD_TYPE_HARDDISK;
        if (unit_ro(u)) flags |= BD_FLAG_RO;
    }
    if (un->vhd) flags |= BD_FLAG_VHD;
    if (un->changed) flags |= BD_FLAG_CHANGED;
    rec[BD_INFO_OFF_FLAGS] = flags;
    put16(rec + BD_INFO_OFF_CYL, un->geom.cyl);
    put16(rec + BD_INFO_OFF_HEADS, un->geom.heads);
    put16(rec + BD_INFO_OFF_SPT, un->geom.spt);
    put32(rec + BD_INFO_OFF_TOTAL, un->geom.total);
    rec[BD_INFO_OFF_DRVTYPE] = un->geom.drive_type;
    rec[BD_INFO_OFF_GEN] = un->gen;
    un->changed = false;

    n = display_name(un, name);
    if (n > (size_t)(maxpl - BD_INFO_LEN)) n = (size_t)(maxpl - BD_INFO_LEN);
    rec[BD_INFO_OFF_NAMELEN] = (uint8_t)n;
    memcpy(answ, rec, BD_INFO_LEN);                 /* req is consumed: answ may alias it */
    memcpy(answ + BD_INFO_LEN, name, n);
    *ax = BD_ST_OK;
    return (uint16_t)(BD_INFO_LEN + n);
}

/* Common checks of BDREAD/BDWRITE. Returns a status; *un_out, *ofs, *len on OK. */
static uint16_t check_io(const uint8_t *req, uint16_t req_len, bool write, uint16_t maxpl,
                         bd_unit_t **un_out, uint32_t *ofs, uint32_t *len) {
    uint8_t u, nn;
    uint32_t lba, total;
    bd_unit_t *un;

    if (req_len < BD_IO_HDR_LEN) return BD_ST_BADCMD;
    u = req[0];
    nn = req[1];
    lba = get32(req + 2);
    if (u >= BD_UNITS || nn == 0) return BD_ST_BADCMD;
    *len = (uint32_t)nn * BD_SECTOR;
    if (write) {
        if ((uint32_t)req_len < BD_IO_HDR_LEN + *len) return BD_ST_BADCMD;   /* data missing */
    } else {
        if (*len > maxpl) return BD_ST_BADCMD;      /* answer would not fit the frame */
    }
    un = &units[u];
    if (un->state != BD_STATE_READY || !un->open) return BD_ST_NOTREADY;
    if (write && unit_ro(u)) return BD_ST_WRPROT;
    total = un->geom.total;
    if (lba >= total || nn > total - lba) return BD_ST_NOSECTOR;
    *un_out = un;
    *ofs = lba * BD_SECTOR;                         /* < 4 GB: total came from a FatFs file size */
    return BD_ST_OK;
}

static uint16_t io_failed(bd_unit_t *un) {
    if (!un->open) return BD_ST_NOTREADY;           /* the drive went away underneath */
    /* FatFs keeps a hard error sticky in the FIL; clear it so the next
     * request tries the drive again instead of failing forever. The FIL
     * stays consistent: FatFs only records a sector as cached after reading
     * it, and a failed write-back leaves the sector marked dirty. */
    un->fil.err = 0;
    return BD_ST_CTRLFAIL;
}

static uint16_t do_read(const uint8_t *req, uint16_t req_len, uint8_t *answ, uint16_t maxpl, uint16_t *ax) {
    bd_unit_t *un = NULL;
    uint32_t ofs = 0, len = 0;
    UINT br = 0;
    FRESULT fr;

    *ax = check_io(req, req_len, false, maxpl, &un, &ofs, &len);
    if (*ax != BD_ST_OK) return 0;
    fr = f_lseek(&un->fil, ofs);
    if (fr == FR_OK) fr = f_read(&un->fil, answ, (UINT)len, &br);     /* answ may alias req: parsed already */
    if (fr != FR_OK || br != len || !un->open) {
        *ax = io_failed(un);
        return 0;
    }
    return (uint16_t)len;
}

static uint16_t do_write(const uint8_t *req, uint16_t req_len, uint16_t *ax) {
    bd_unit_t *un = NULL;
    uint32_t ofs = 0, len = 0;
    UINT bw = 0;
    FRESULT fr;

    *ax = check_io(req, req_len, true, 0, &un, &ofs, &len);
    if (*ax != BD_ST_OK) return 0;
    fr = f_lseek(&un->fil, ofs);
    if (fr == FR_OK) fr = f_write(&un->fil, req + BD_IO_HDR_LEN, (UINT)len, &bw);
    if (fr == FR_OK && bw != len) fr = FR_DISK_ERR;     /* never extends: cannot run out of space */
    if (fr == FR_OK) fr = f_sync(&un->fil);
    if (fr != FR_OK || !un->open) *ax = io_failed(un);
    return 0;
}

uint16_t bd_process(uint8_t al, const uint8_t *req, uint16_t req_len,
                    uint8_t *answ, uint16_t maxpl, uint16_t *ax) {
    bd_tasks();                     /* a floppy commit applies before anything is served */
    switch (al) {
    case BD_AL_INFO:  return do_info(req, req_len, answ, maxpl, ax);
    case BD_AL_READ:  return do_read(req, req_len, answ, maxpl, ax);
    case BD_AL_WRITE: return do_write(req, req_len, ax);
    default:
        *ax = BD_ST_BADCMD;
        return 0;
    }
}

/* ---- PGDFS interlock ------------------------------------------------------------- */

/* Case-insensitive compare of two normalised paths; prefix: a may also name
 * a directory on b's path. */
static bool path_match(const char *a, const char *b, bool prefix) {
    while (*a && *b) {
        if (upper(*a) != upper(*b)) return false;
        a++;
        b++;
    }
    if (*a) return false;
    return *b == 0 || (prefix && *b == '\\');
}

static bool open_image_match(const char *path, bool prefix) {
    char norm[BD_SFN_BUF];
    bool any = false;
    for (int u = 0; u < BD_UNITS; u++) any |= units[u].open;
    if (!any || path == NULL || path[0] == 0) return false;
    if (strlen(path) >= sizeof(norm)) return false;     /* longer than any image path we hold */
    norm_path(norm, sizeof(norm), path);
    if (norm[1] == 0) return false;                     /* the root */
    for (int u = 0; u < BD_UNITS; u++) {
        const bd_unit_t *un = &units[u];
        if (!un->open) continue;
        if (path_match(norm, un->path, prefix)) return true;
        if (un->sfn[0] && path_match(norm, un->sfn, prefix)) return true;
    }
    return false;
}

bool bd_path_is_open_image(const char *path) {
    return open_image_match(path, false);
}

bool bd_path_holds_open_image(const char *path) {
    return open_image_match(path, true);
}

#ifdef BD_HOST_TEST
/* Host tests only: whether unit u serves through a fast seek link map. */
bool bd_test_fastseek(int u) {
#if FF_USE_FASTSEEK
    return u >= 0 && u < BD_UNITS && units[u].open && units[u].fil.cltbl != NULL;
#else
    return false;
#endif
}
#endif
