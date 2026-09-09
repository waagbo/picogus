/*
 *  PGDFS file server: FatFs binding.
 *
 *  Open-file table, directory-id table with one cached DIR for
 *  FINDFIRST/FINDNEXT, DOS<->FatFs conversions and the get_fattime()
 *  clock fed by the DOS driver.
 *
 *  Derived from ethersrv-linux, Copyright (C) 2017, 2018 Mateusz Viste,
 *  MIT License; long-file-name modifications by Eric Voirin (oerg866);
 *  FatFs adaptation in the PicoMEM project by Freddy Vetele.
 *  PicoGUS integration Copyright (C) 2026 PicoGUS contributors.
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
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <string.h>
#include "ff.h"
#include "dfs_fs.h"
#include "dfs_server.h"     /* dfs_platform_millis() */

#if FF_FS_READONLY || FF_FS_MINIMIZE || !FF_USE_CHMOD || !FF_USE_LABEL || FF_FS_RPATH || !FF_USE_LFN
#error "PGDFS needs FatFs with FF_FS_READONLY 0, FF_FS_MINIMIZE 0, FF_USE_CHMOD 1, FF_USE_LABEL 1, FF_FS_RPATH 0, FF_USE_LFN 1"
#endif

/* Timestamp used before the driver has sent the DOS clock: 2026-01-01 00:00:00 */
#define DFS_DEFAULT_FATTIME (((DWORD)(2026 - 1980) << 25) | ((DWORD)1 << 21) | ((DWORD)1 << 16))

/* ---- static state (core 1 only) ------------------------------------------ */

static FIL      files[DFS_MAX_FILES];
static char     file_path[DFS_MAX_FILES][DFS_PATH_MAX];  /* for f_utime() */
static uint8_t  file_used;                               /* bit i = files[i] in use */

typedef struct {
    uint16_t id;                /* 0 = free */
    uint32_t stamp;             /* LRU clock */
    char     path[DFS_PATH_MAX];
} dir_entry_t;

static dir_entry_t dirs[DFS_MAX_DIRS];
static uint32_t    dir_clock;
static uint16_t    dir_next_id = 1;

static struct {
    DIR      dir;
    uint16_t id;                /* directory id the DIR is positioned in */
    uint32_t next;              /* index of the entry the next f_readdir() yields */
    bool     valid;
} cache;

static DIR     scan_dir;        /* wildcard DELETE */
static FILINFO fno;             /* shared scratch, ~280 bytes */
static char    scan_path[DFS_PATH_MAX + 14];
static char    label_buf[24];

/* DOS clock: seconds since 1980-01-01 00:00:00 at millis() == time_base_ms */
static uint32_t time_base_secs;
static uint32_t time_base_ms;
static bool     time_valid;

/* ---- helpers -------------------------------------------------------------- */

uint16_t dfs_fr2dos(FRESULT fr) {
    switch (fr) {
    case FR_OK:                  return DFS_ERR_OK;
    case FR_NO_FILE:             return DFS_ERR_FILE;
    case FR_NO_PATH:
    case FR_INVALID_NAME:        return DFS_ERR_PATH;
    case FR_DENIED:
    case FR_EXIST:               return DFS_ERR_ACCESS;
    case FR_WRITE_PROTECTED:     return DFS_ERR_WRPROT;
    case FR_INVALID_OBJECT:      return DFS_ERR_HANDLE;
    case FR_TOO_MANY_OPEN_FILES: return DFS_ERR_HANDLES;
    case FR_LOCKED:
    case FR_TIMEOUT:             return DFS_ERR_SHARING;
    case FR_NOT_ENOUGH_CORE:     return DFS_ERR_MEM;
    case FR_INVALID_DRIVE:       return DFS_ERR_DRIVE;
    case FR_INVALID_PARAMETER:   return DFS_ERR_FUNC;
    default:                     return DFS_ERR_NOTREADY; /* DISK_ERR, INT_ERR, NOT_READY, NOT_ENABLED, NO_FILESYSTEM */
    }
}

/* "FILE.TXT" -> "FILE    TXT", "*.*" -> "???????????", "." -> ".          " */
void dfs_name2fcb(char *fcb, const char *name) {
    int i = 0, j = 0;
    memset(fcb, ' ', 11);
    if (name[0] == '.') {                       /* dot entries: '.' and '..' */
        while (name[j] == '.' && i < 11) fcb[i++] = name[j++];
        return;
    }
    while (i < 8) {
        char c = name[j];
        if (c == 0 || c == '.') break;
        if (c == '*') { while (i < 8) fcb[i++] = '?'; break; }
        fcb[i++] = dfs_upper(c);
        j++;
    }
    while (name[j] != 0 && name[j] != '.') j++; /* rest of an over-long body */
    if (name[j] != '.') return;
    j++;
    i = 8;
    while (i < 11) {
        char c = name[j];
        if (c == 0 || c == '.') break;
        if (c == '*') { while (i < 11) fcb[i++] = '?'; break; }
        fcb[i++] = dfs_upper(c);
        j++;
    }
}

bool dfs_fcb_match(const char *mask, const char *fcb) {
    for (int i = 0; i < 11; i++) {
        if (mask[i] != '?' && dfs_upper(mask[i]) != dfs_upper(fcb[i])) return false;
    }
    return true;
}

bool dfs_path_is_root(const char *path) {
    while (dfs_is_sep(*path)) path++;
    return *path == 0;
}

static bool path_eq(const char *a, const char *b) {
    while (*a && *b) {
        char ca = dfs_upper(*a), cb = dfs_upper(*b);
        if (dfs_is_sep(ca)) ca = '\\';
        if (dfs_is_sep(cb)) cb = '\\';
        if (ca != cb) return false;
        a++; b++;
    }
    return *a == *b;
}

static void fill_info(dfs_finfo_t *info, const FILINFO *f) {
    info->size = (uint32_t)f->fsize;
    info->time = f->ftime;
    info->date = f->fdate;
    info->attr = f->fattrib;
    /* altname is the 8.3 alias when the entry has a long name (or NT case
     * bits); otherwise fname already is the upper-case short name */
    dfs_name2fcb(info->fcb, f->altname[0] ? f->altname : f->fname);
}

static FIL *get_file(uint16_t id) {
    if (id < 1 || id > DFS_MAX_FILES) return NULL;
    if (!(file_used & (1u << (id - 1)))) return NULL;
    return &files[id - 1];
}

/* ---- state ---------------------------------------------------------------- */

void dfs_fs_invalidate(void) {
    file_used = 0;              /* the volume is gone: no f_close(), just forget */
    cache.valid = false;
    memset(dirs, 0, sizeof(dirs));
}

void dfs_fs_reset(void) {
    dfs_fs_invalidate();
    dir_clock = 0;
    dir_next_id = 1;
    time_valid = false;
}

/* ---- time ----------------------------------------------------------------- */

static const uint8_t mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static bool is_leap(uint32_t y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

static uint32_t days_in_month(uint32_t y, uint32_t m) {
    return mdays[m - 1] + ((m == 2 && is_leap(y)) ? 1 : 0);
}

static uint32_t dos2secs(uint16_t t, uint16_t d) {
    uint32_t y = 1980 + (d >> 9);
    uint32_t m = (d >> 5) & 15;
    uint32_t day = d & 31;
    uint32_t days = 0;
    if (m < 1) m = 1;
    if (m > 12) m = 12;
    if (day < 1) day = 1;
    for (uint32_t yy = 1980; yy < y; yy++) days += is_leap(yy) ? 366 : 365;
    for (uint32_t mm = 1; mm < m; mm++) days += days_in_month(y, mm);
    days += day - 1;
    return days * 86400u + (uint32_t)(t >> 11) * 3600u + (uint32_t)((t >> 5) & 63) * 60u + (uint32_t)(t & 31) * 2u;
}

static uint32_t secs2fattime(uint32_t secs) {
    uint32_t days = secs / 86400u, rem = secs % 86400u;
    uint32_t y = 1980, m = 1;
    for (;;) {
        uint32_t yd = is_leap(y) ? 366 : 365;
        if (days < yd || y >= 2107) break;
        days -= yd;
        y++;
    }
    for (;;) {
        uint32_t md = days_in_month(y, m);
        if (days < md || m >= 12) break;
        days -= md;
        m++;
    }
    if (days > 30) days = 30;
    return ((uint32_t)(y - 1980) << 25) | (m << 21) | ((days + 1) << 16) |
           ((rem / 3600u) << 11) | (((rem / 60u) % 60u) << 5) | ((rem % 60u) >> 1);
}

void dfs_fs_set_dos_time(uint16_t dos_time, uint16_t dos_date) {
    time_base_secs = dos2secs(dos_time, dos_date);
    time_base_ms = dfs_platform_millis();
    time_valid = true;
}

DWORD get_fattime(void) {
    uint32_t elapsed;
    if (!time_valid) return DFS_DEFAULT_FATTIME;
    /* fold the elapsed time into the base so the 32-bit millisecond counter
     * may wrap between calls without losing anything */
    elapsed = dfs_platform_millis() - time_base_ms;
    if (elapsed >= 1000) {
        uint32_t s = elapsed / 1000;
        time_base_secs += s;
        time_base_ms += s * 1000;
    }
    return secs2fattime(time_base_secs);
}

/* ---- volume --------------------------------------------------------------- */

static char *put_str(char *p, const char *end, const char *s) {
    while (*s && p < end) *p++ = *s++;
    return p;
}

static char *put_dec(char *p, const char *end, uint32_t v) {
    char tmp[11];
    int n = 0;
    do { tmp[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (n > 0 && p < end) *p++ = tmp[--n];
    return p;
}

static char *put_hex32(char *p, const char *end, uint32_t v) {
    for (int i = 28; i >= 0 && p < end; i -= 4) *p++ = "0123456789ABCDEF"[(v >> i) & 15];
    return p;
}

bool dfs_fs_volume_info(char *out, size_t cap) {
    FATFS *fs;
    DWORD nfree, serial = 0;
    uint32_t sectors, mb;
    const char *type;
    char *p = out, *end = out + cap - 1;

    out[0] = 0;
    if (cap < 2) return false;
    if (f_getfree("", &nfree, &fs) != FR_OK) return false;
    label_buf[0] = 0;
    if (f_getlabel("", label_buf, &serial) != FR_OK) label_buf[0] = 0;
    switch (fs->fs_type) {
    case FS_FAT12: type = "FAT12"; break;
    case FS_FAT16: type = "FAT16"; break;
    case FS_FAT32: type = "FAT32"; break;
#if FF_FS_EXFAT
    case FS_EXFAT: type = "EXFAT"; break;
#endif
    default:       type = "FAT";   break;
    }
    sectors = (fs->n_fatent - 2) * fs->csize;    /* fine below 2 TB */
#if FF_MAX_SS != FF_MIN_SS
    mb = sectors / (1048576u / fs->ssize);
#else
    mb = sectors / (1048576u / FF_MAX_SS);
#endif
    p = put_str(p, end, label_buf);
    p = put_str(p, end, "|");
    p = put_str(p, end, type);
    p = put_str(p, end, "|");
    p = put_dec(p, end, mb);
    p = put_str(p, end, "|");
    p = put_hex32(p, end, serial);
    *p = 0;
    return true;
}

bool dfs_fs_label(char *fcb) {
    int i;
    label_buf[0] = 0;
    if (f_getlabel("", label_buf, NULL) != FR_OK || label_buf[0] == 0) return false;
    memset(fcb, ' ', 11);
    for (i = 0; i < 11 && label_buf[i]; i++) fcb[i] = label_buf[i];
    return true;
}

uint16_t dfs_fs_diskspace(uint16_t *total_units, uint16_t *free_units) {
    FATFS *fs;
    DWORD nfree;
    uint32_t clusters, total, avail, ss;
    FRESULT fr = f_getfree("", &nfree, &fs);
    if (fr != FR_OK) return dfs_fr2dos(fr);
#if FF_MAX_SS != FF_MIN_SS
    ss = fs->ssize;
#else
    ss = FF_MAX_SS;
#endif
    clusters = fs->n_fatent - 2;
    /* express the sizes in 32 KB units without overflowing 32 bits */
    if ((uint32_t)fs->csize * ss >= 32768u) {
        uint32_t per = ((uint32_t)fs->csize * ss) / 32768u;
        total = clusters * per;
        avail = nfree * per;
    } else {
        uint32_t div = 32768u / ((uint32_t)fs->csize * ss);
        total = clusters / div;
        avail = nfree / div;
    }
    /* MS-DOS gets confused beyond 2 GB (65535 * 32 KB) */
    *total_units = (total > 0xFFFF) ? 0xFFFF : (uint16_t)total;
    *free_units = (avail > 0xFFFF) ? 0xFFFF : (uint16_t)avail;
    return DFS_ERR_OK;
}

/* ---- open files ----------------------------------------------------------- */

uint16_t dfs_fs_open(const char *path, uint8_t fa_mode, uint8_t set_attr, uint16_t *id, dfs_finfo_t *info) {
    int slot;
    uint8_t bit;
    FRESULT fr;

    for (slot = 0; slot < DFS_MAX_FILES; slot++) {
        if (!(file_used & (1u << slot))) break;
    }
    if (slot == DFS_MAX_FILES) return DFS_ERR_HANDLES;
    if (strlen(path) >= DFS_PATH_MAX) return DFS_ERR_PATH;
    bit = (uint8_t)(1u << slot);

    fr = f_open(&files[slot], path, fa_mode);
    DFS_LOG("f_open('%s', %02X) = %d\n", path, fa_mode, fr);
    if (fr != FR_OK) return dfs_fr2dos(fr);
    file_used |= bit;
    strcpy(file_path[slot], path);

    set_attr &= DFS_ATTR_RDO | DFS_ATTR_HID | DFS_ATTR_SYS;
    if (set_attr) f_chmod(path, set_attr, DFS_ATTR_RDO | DFS_ATTR_HID | DFS_ATTR_SYS);

    fr = f_stat(path, &fno);
    if (fr != FR_OK) {
        f_close(&files[slot]);
        file_used &= (uint8_t)~bit;
        return dfs_fr2dos(fr);
    }
    fill_info(info, &fno);
    *id = (uint16_t)(slot + 1);
    return DFS_ERR_OK;
}

uint16_t dfs_fs_close(uint16_t id) {
    FIL *fp = get_file(id);
    FRESULT fr;
    if (!fp) return DFS_ERR_HANDLE;
    fr = f_close(fp);
    file_used &= (uint8_t)~(1u << (id - 1));
    return dfs_fr2dos(fr);
}

uint16_t dfs_fs_read(uint16_t id, uint32_t offset, uint8_t *dst, uint16_t len, uint16_t *got) {
    FIL *fp = get_file(id);
    FRESULT fr;
    UINT br = 0;
    *got = 0;
    if (!fp) return DFS_ERR_HANDLE;
    /* never seek past the end: on a writable handle that would grow the file */
    if (offset >= f_size(fp)) return DFS_ERR_OK;
    fr = f_lseek(fp, offset);
    if (fr == FR_OK) fr = f_read(fp, dst, len, &br);
    if (fr != FR_OK) return dfs_fr2dos(fr);
    *got = (uint16_t)br;
    return DFS_ERR_OK;
}

uint16_t dfs_fs_write(uint16_t id, uint32_t offset, const uint8_t *src, uint16_t len, uint16_t *done) {
    FIL *fp = get_file(id);
    FRESULT fr;
    UINT bw = 0;
    *done = 0;
    if (!fp) return DFS_ERR_HANDLE;
    if (!(fp->flag & FA_WRITE)) return DFS_ERR_ACCESS;
    fr = f_lseek(fp, offset);           /* extends the file when offset > size */
    if (fr == FR_OK) {
        if (len == 0) {
            fr = f_truncate(fp);        /* DOS: zero-length write sets the size */
        } else {
            fr = f_write(fp, src, len, &bw);
        }
    }
    if (fr == FR_OK) fr = f_sync(fp);   /* an unplug should lose as little as possible */
    if (fr != FR_OK) return dfs_fr2dos(fr);
    *done = (uint16_t)bw;
    return DFS_ERR_OK;
}

uint16_t dfs_fs_size(uint16_t id, uint32_t *size) {
    FIL *fp = get_file(id);
    if (!fp) return DFS_ERR_HANDLE;
    *size = (uint32_t)f_size(fp);
    return DFS_ERR_OK;
}

uint16_t dfs_fs_utime(uint16_t id, uint16_t dos_time, uint16_t dos_date) {
    FIL *fp = get_file(id);
    FRESULT fr;
    if (!fp) return DFS_ERR_HANDLE;
    /* flush first: a later f_close() only rewrites the entry when data is pending */
    fr = f_sync(fp);
    if (fr != FR_OK) return dfs_fr2dos(fr);
    fno.ftime = dos_time;
    fno.fdate = dos_date;
    return dfs_fr2dos(f_utime(file_path[id - 1], &fno));
}

/* ---- paths ---------------------------------------------------------------- */

uint16_t dfs_fs_stat(const char *path, dfs_finfo_t *info) {
    FRESULT fr;
    if (dfs_path_is_root(path)) {
        memset(info, 0, sizeof(*info));
        memset(info->fcb, ' ', 11);
        info->attr = DFS_ATTR_DIR;
        return DFS_ERR_OK;
    }
    fr = f_stat(path, &fno);
    if (fr != FR_OK) return dfs_fr2dos(fr);
    fill_info(info, &fno);
    return DFS_ERR_OK;
}

uint16_t dfs_fs_chmod(const char *path, uint8_t attr) {
    const uint8_t mask = DFS_ATTR_RDO | DFS_ATTR_HID | DFS_ATTR_SYS | DFS_ATTR_ARC;
    if (dfs_path_is_root(path)) return DFS_ERR_ACCESS;
    return dfs_fr2dos(f_chmod(path, attr & mask, mask));
}

uint16_t dfs_fs_mkdir(const char *path) {
    if (dfs_path_is_root(path)) return DFS_ERR_ACCESS;
    return dfs_fr2dos(f_mkdir(path));
}

uint16_t dfs_fs_rmdir(const char *path) {
    FRESULT fr;
    if (dfs_path_is_root(path)) return DFS_ERR_ACCESS;
    fr = f_stat(path, &fno);
    if (fr != FR_OK) return (fr == FR_NO_FILE) ? DFS_ERR_PATH : dfs_fr2dos(fr);
    if (!(fno.fattrib & AM_DIR)) return DFS_ERR_PATH;
    cache.valid = false;                /* the cached DIR may sit in it */
    return dfs_fr2dos(f_unlink(path));  /* FR_DENIED when not empty or read-only */
}

uint16_t dfs_fs_chdir(const char *path) {
    if (dfs_path_is_root(path)) return DFS_ERR_OK;
    if (f_stat(path, &fno) != FR_OK) return DFS_ERR_PATH;
    return (fno.fattrib & AM_DIR) ? DFS_ERR_OK : DFS_ERR_PATH;
}

uint16_t dfs_fs_rename(const char *from, const char *to) {
    if (dfs_path_is_root(from) || dfs_path_is_root(to)) return DFS_ERR_ACCESS;
    cache.valid = false;
    return dfs_fr2dos(f_rename(from, to));   /* FR_EXIST -> access denied */
}

uint16_t dfs_fs_unlink(const char *path) {
    FRESULT fr;
    if (dfs_path_is_root(path)) return DFS_ERR_ACCESS;
    fr = f_stat(path, &fno);
    if (fr != FR_OK) return dfs_fr2dos(fr);
    if (fno.fattrib & AM_DIR) return DFS_ERR_ACCESS;
    return dfs_fr2dos(f_unlink(path));       /* FR_DENIED on a read-only file */
}

uint16_t dfs_fs_delete_wild(const char *dir, const char *fcbmask) {
    FRESULT fr;
    uint16_t err = DFS_ERR_OK;
    int deleted = 0;
    size_t dlen = strlen(dir);
    char fcb[11];

    if (dlen >= DFS_PATH_MAX) return DFS_ERR_PATH;
    fr = f_opendir(&scan_dir, dir);
    if (fr != FR_OK) return (fr == FR_NO_FILE) ? DFS_ERR_PATH : dfs_fr2dos(fr);
    for (;;) {
        const char *name;
        fr = f_readdir(&scan_dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        if (fno.fattrib & AM_DIR) continue;
        name = fno.altname[0] ? fno.altname : fno.fname;   /* always the 8.3 alias */
        dfs_name2fcb(fcb, name);
        if (!dfs_fcb_match(fcbmask, fcb)) continue;
        memcpy(scan_path, dir, dlen);
        scan_path[dlen] = '\\';
        strcpy(scan_path + dlen + 1, name);
        fr = f_unlink(scan_path);
        DFS_LOG("f_unlink('%s') = %d\n", scan_path, fr);
        if (fr == FR_OK) {
            deleted++;
        } else if (err == DFS_ERR_OK) {
            err = dfs_fr2dos(fr);
        }
    }
    f_closedir(&scan_dir);
    if (err != DFS_ERR_OK) return err;
    return deleted ? DFS_ERR_OK : DFS_ERR_FILE;
}

/* ---- directory search ----------------------------------------------------- */

static int dir_slot_by_id(uint16_t id) {
    if (id == 0) return -1;
    for (int i = 0; i < DFS_MAX_DIRS; i++) {
        if (dirs[i].id == id) return i;
    }
    return -1;
}

/* find or insert (LRU) a directory path, return its id (never 0) */
static uint16_t dir_id_for_path(const char *path) {
    int i, victim = 0;
    for (i = 0; i < DFS_MAX_DIRS; i++) {
        if (dirs[i].id != 0 && path_eq(dirs[i].path, path)) {
            dirs[i].stamp = ++dir_clock;
            return dirs[i].id;
        }
    }
    for (i = 0; i < DFS_MAX_DIRS; i++) {
        if (dirs[i].id == 0) { victim = i; break; }
        if (dirs[i].stamp < dirs[victim].stamp) victim = i;
    }
    if (cache.valid && cache.id == dirs[victim].id) cache.valid = false;
    if (dir_next_id == 0) dir_next_id = 1;
    dirs[victim].id = dir_next_id++;
    dirs[victim].stamp = ++dir_clock;
    strcpy(dirs[victim].path, path);
    return dirs[victim].id;
}

/* DOS search attribute semantics: hidden, system and directory entries only
 * show up when the mask asks for them; read-only and archive never matter */
static bool attr_ok(uint8_t fattrib, uint8_t mask) {
    return ((fattrib & (AM_HID | AM_SYS | AM_DIR)) & (uint8_t)~mask) == 0;
}

/* Return the first entry at index >= start of directory dir_id that matches.
 * Index 0 and 1 are '.' and '..' in non-root directories; the rest counts
 * f_readdir() items whether they match or not, so a position can be resumed
 * by skipping. */
static uint16_t search(uint16_t dir_id, uint32_t start, uint8_t attr, const char *fcbmask,
                       dfs_finfo_t *info, uint16_t *pos, bool first) {
    int slot = dir_slot_by_id(dir_id);
    const char *path;
    uint32_t dots, idx;
    FRESULT fr;

    if (slot < 0) return DFS_ERR_NOMORE;            /* evicted or bogus id */
    path = dirs[slot].path;
    dots = (path[0] == 0) ? 0 : 2;
    dirs[slot].stamp = ++dir_clock;

    if (!(cache.valid && cache.id == dir_id && cache.next == start)) {
        cache.valid = false;
        fr = f_opendir(&cache.dir, path);
        DFS_LOG("f_opendir('%s') = %d\n", path, fr);
        if (fr != FR_OK) {
            uint16_t e = dfs_fr2dos(fr);
            if (!first) return DFS_ERR_NOMORE;
            return (e == DFS_ERR_FILE) ? DFS_ERR_PATH : e;
        }
        cache.valid = true;
        cache.id = dir_id;
        for (idx = dots; idx < start; idx++) {      /* skip to the resume point */
            fr = f_readdir(&cache.dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) {
                cache.valid = false;
                return DFS_ERR_NOMORE;
            }
        }
        cache.next = start;
    }

    for (idx = start; idx <= 0xFFFF; idx++) {
        if (idx < dots) {                           /* synthesized '.' and '..' */
            cache.next = idx + 1;
            if (!(attr & AM_DIR)) continue;
            dfs_name2fcb(info->fcb, idx == 0 ? "." : "..");
            if (!dfs_fcb_match(fcbmask, info->fcb)) continue;
            info->attr = DFS_ATTR_DIR;
            info->size = 0;
            info->time = 0;
            info->date = 0;
            if (f_stat(path, &fno) == FR_OK) {      /* the directory's own stamp */
                info->time = fno.ftime;
                info->date = fno.fdate;
            }
            *pos = (uint16_t)idx;
            return DFS_ERR_OK;
        }
        fr = f_readdir(&cache.dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        cache.next = idx + 1;
        if (!attr_ok(fno.fattrib, attr)) continue;
        dfs_name2fcb(info->fcb, fno.altname[0] ? fno.altname : fno.fname);
        if (!dfs_fcb_match(fcbmask, info->fcb)) continue;
        info->size = (uint32_t)fno.fsize;
        info->time = fno.ftime;
        info->date = fno.fdate;
        info->attr = fno.fattrib;
        *pos = (uint16_t)idx;
        return DFS_ERR_OK;
    }
    cache.valid = false;
    return DFS_ERR_NOMORE;
}

uint16_t dfs_fs_find_first(const char *dir, uint8_t attr, const char *fcbmask,
                           dfs_finfo_t *info, uint16_t *dir_id, uint16_t *pos) {
    if (strlen(dir) >= DFS_PATH_MAX) return DFS_ERR_PATH;
    *dir_id = dir_id_for_path(dir);
    return search(*dir_id, 0, attr, fcbmask, info, pos, true);
}

uint16_t dfs_fs_find_next(uint16_t dir_id, uint16_t pos, uint8_t attr, const char *fcbmask,
                          dfs_finfo_t *info, uint16_t *newpos) {
    return search(dir_id, (uint32_t)pos + 1, attr, fcbmask, info, newpos, false);
}
