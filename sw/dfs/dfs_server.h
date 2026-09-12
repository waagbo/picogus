/*
 *  PGDFS file server: EtherDFS (EDF5) request processing on FatFs.
 *
 *  Derived from ethersrv-linux, Copyright (C) 2017, 2018 Mateusz Viste,
 *  MIT License, and its FatFs adaptation in the PicoMEM project.
 *  PicoGUS integration Copyright (C) 2026 PicoGUS contributors, GPL v2+.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../../common/picogus.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pure request processor, host-testable. buf holds a request frame
 * (DFS_HDR_LEN header + payload, see PROTOCOL.md); on return it holds the
 * answer frame. Returns the total answer length (>= DFS_HDR_LEN). The
 * answer never exceeds buf_size. Runs on core 1 only.
 */
uint16_t dfs_process(uint8_t *buf, uint16_t req_len, uint16_t buf_size);

/*
 * Core notes: dfs_server_init() runs on core 0 at boot. dfs_server_drive_present()
 * and dfs_server_info_string() are called from core 0 inside an ISA bus cycle:
 * O(1), no FatFs, no loops. The info string may be replaced by core 1 while
 * core 0 iterates over it (a torn read is harmless). dfs_server_set_dos_time()
 * is called from dfs_tasks() on core 1, so the server's clock state is core 1 only.
 * dfs_server_drive_unmounted() can be invoked from inside dfs_process() (a USB
 * unplug surfaces through tuh_task() while FatFs waits on disk I/O), so it must
 * only mark state and never touch the frame being processed.
 * dfs_fs.c owns the strong DWORD get_fattime(void); usb_msc/msc_app.c carries a
 * weak fallback for builds without PGDFS.
 */
void dfs_server_init(void);                 /* reset tables, no FatFs access */
void dfs_server_drive_mounted(void);        /* volume "" is mounted: refresh info string */
void dfs_server_drive_unmounted(void);      /* invalidate every open file/dir handle */
bool dfs_server_drive_present(void);

/* DOS packed time/date (FAT format) as sent by the driver. Used by get_fattime(). */
void dfs_server_set_dos_time(uint16_t dos_time, uint16_t dos_date);

/* "LABEL|FAT32|<size MB>|<serial hex>" or "" when nothing is mounted. Stable storage. */
const char *dfs_server_info_string(void);

/* Provided by the platform (dfs_transport.c on the Pico, the test harness on host). */
uint32_t dfs_platform_millis(void);

/* PGDFS-specific subfunctions on top of the EDF5 set (AL values 0x00-0x2E). */
#define DFS_AL_ECHO     0xF0 /* answer payload = request payload, AX = 0 */
#define DFS_AL_LONGNAME 0xF1 /* request: path as DOS sees it ("\\DIR\\FILE~1.TXT", 8.3);
                              * answer: the entry's long file name in the FatFs code page
                              * (the same as the short name when it has none), AX = 0,
                              * or AX = 2/3 when the path does not exist. Lets tools show
                              * long names although the redirector interface is 8.3-only. */

#ifdef __cplusplus
}
#endif
