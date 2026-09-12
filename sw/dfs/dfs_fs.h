/*
 *  PGDFS file server: FatFs binding (internal header).
 *
 *  Derived from ethersrv-linux, Copyright (C) 2017, 2018 Mateusz Viste,
 *  MIT License; long-file-name modifications by Eric Voirin (oerg866);
 *  FatFs adaptation in the PicoMEM project by Freddy Vetele.
 *  PicoGUS integration Copyright (C) 2026 PicoGUS contributors, GPL v2+.
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
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ff.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Everything in here runs on core 1 only (dfs_process() is single-threaded),
 * so the FatFs objects and the scratch buffers are static: core 1 has a 4 KB
 * stack and FatFs itself needs a few hundred bytes of it.
 */

#define DFS_MAX_FILES   8       /* open-file table, ids 1..DFS_MAX_FILES     */
#define DFS_MAX_DIRS    32      /* directory-id table for FINDFIRST/FINDNEXT; XCOPY /S
                                 * keeps one open search per level in both trees      */
#define DFS_PATH_MAX    128     /* DOS canonical path without "X:", incl. NUL */

/* DOS error codes returned in AX */
#define DFS_ERR_OK        0x00
#define DFS_ERR_FUNC      0x01  /* invalid function                     */
#define DFS_ERR_FILE      0x02  /* file not found                       */
#define DFS_ERR_PATH      0x03  /* path not found                       */
#define DFS_ERR_HANDLES   0x04  /* too many open files                  */
#define DFS_ERR_ACCESS    0x05  /* access denied                        */
#define DFS_ERR_HANDLE    0x06  /* invalid handle                       */
#define DFS_ERR_MEM       0x08  /* insufficient memory                  */
#define DFS_ERR_DRIVE     0x0F  /* invalid drive                        */
#define DFS_ERR_NOMORE    0x12  /* no more files                        */
#define DFS_ERR_WRPROT    0x13  /* disk write-protected                 */
#define DFS_ERR_NOTREADY  0x15  /* drive not ready                      */
#define DFS_ERR_SHARING   0x20  /* sharing violation                    */
#define DFS_ERR_EXISTS    0x50  /* file exists (create-new on existing) */

/* Attributes as DOS and FAT see them (same values as FatFs AM_*) */
#define DFS_ATTR_RDO  0x01
#define DFS_ATTR_HID  0x02
#define DFS_ATTR_SYS  0x04
#define DFS_ATTR_VOL  0x08
#define DFS_ATTR_DIR  0x10
#define DFS_ATTR_ARC  0x20

#ifdef DFS_DEBUG
#include <stdio.h>
#define DFS_LOG(...) printf(__VA_ARGS__)
#else
#define DFS_LOG(...) do { } while (0)
#endif

/* What the EDF5 answers carry about a directory entry */
typedef struct {
    uint32_t size;
    uint16_t time;      /* FAT packed time */
    uint16_t date;      /* FAT packed date */
    uint8_t  attr;
    char     fcb[11];   /* "FILE0001TXT", space padded, upper case */
} dfs_finfo_t;

/* ---- helpers -------------------------------------------------------------- */
static inline bool dfs_is_sep(char c) { return c == '\\' || c == '/'; }
static inline char dfs_upper(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }

uint16_t dfs_fr2dos(FRESULT fr);                          /* FatFs result -> DOS error */
void     dfs_name2fcb(char *fcb, const char *name);       /* "FILE.TXT" / "*.*" -> 11-byte FCB form */
bool     dfs_fcb_match(const char *mask, const char *fcb);/* '?' wildcards, case-insensitive */
bool     dfs_path_is_root(const char *path);              /* "", "\", "/" ... */

/* ---- state ---------------------------------------------------------------- */
void dfs_fs_reset(void);           /* boot: forget handles, dir ids, cache and time; no FatFs calls */
void dfs_fs_invalidate(void);      /* drive gone: drop every handle and cached DIR; no FatFs calls  */
void dfs_fs_set_dos_time(uint16_t dos_time, uint16_t dos_date);
/* "LABEL|FAT32|<size MB>|<serial hex>" for the mounted volume; false if FatFs refuses */
bool dfs_fs_volume_info(char *out, size_t cap);
/* volume label as an 11-byte FCB block; false when the volume has none */
bool dfs_fs_label(char *fcb);

/* ---- open files (ids 1..DFS_MAX_FILES) ------------------------------------ */
/* fa_mode: FatFs FA_* flags. set_attr: DOS attributes applied after a create (RDO/HID/SYS). */
uint16_t dfs_fs_open(const char *path, uint8_t fa_mode, uint8_t set_attr, uint16_t *id, dfs_finfo_t *info);
uint16_t dfs_fs_close(uint16_t id);
uint16_t dfs_fs_read(uint16_t id, uint32_t offset, uint8_t *dst, uint16_t len, uint16_t *got);
/* len == 0 truncates (or extends) the file at offset. Always f_sync()s. */
uint16_t dfs_fs_write(uint16_t id, uint32_t offset, const uint8_t *src, uint16_t len, uint16_t *done);
uint16_t dfs_fs_size(uint16_t id, uint32_t *size);
uint16_t dfs_fs_utime(uint16_t id, uint16_t dos_time, uint16_t dos_date);

/* ---- paths ---------------------------------------------------------------- */
uint16_t dfs_fs_stat(const char *path, dfs_finfo_t *info);   /* root is synthesized as a directory */
/* Long file name of the entry in dir ("" for the root) whose 8.3 name matches
 * fcbmask (DFS_AL_LONGNAME). *name points at static scratch, valid until the next
 * dfs_fs_* call: the long name in the FatFs OEM code page, or the short name
 * itself (with the case Windows recorded) when the entry has none. DFS_ERR_PATH
 * when dir does not exist, DFS_ERR_FILE when no entry matches. Scans dir with
 * f_readdir() and resumes the scan on the next call for the same dir. */
uint16_t dfs_fs_longname(const char *dir, const char *fcbmask, const char **name);
uint16_t dfs_fs_chmod(const char *path, uint8_t attr);
uint16_t dfs_fs_mkdir(const char *path);
uint16_t dfs_fs_rmdir(const char *path);
uint16_t dfs_fs_chdir(const char *path);                     /* existence check only */
uint16_t dfs_fs_rename(const char *from, const char *to);
uint16_t dfs_fs_unlink(const char *path);                    /* one file, no wildcards */
uint16_t dfs_fs_delete_wild(const char *dir, const char *fcbmask); /* every matching file in dir */
/* DOS DISKSPACE numbers: 32 KB units, capped just under 2 GB */
uint16_t dfs_fs_diskspace(uint16_t *total_units, uint16_t *free_units);

/* ---- directory search ----------------------------------------------------- */
/* dir: directory path without trailing separator ("" for the root). Returns 0 and
 * fills info/dir_id/pos, or DFS_ERR_NOMORE / DFS_ERR_PATH. */
uint16_t dfs_fs_find_first(const char *dir, uint8_t attr, const char *fcbmask,
                           dfs_finfo_t *info, uint16_t *dir_id, uint16_t *pos);
/* continue after entry pos of directory dir_id; *pos receives the new position */
uint16_t dfs_fs_find_next(uint16_t dir_id, uint16_t pos, uint8_t attr, const char *fcbmask,
                          dfs_finfo_t *info, uint16_t *newpos);

#ifdef __cplusplus
}
#endif
