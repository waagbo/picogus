/*
 *  Copyright (C) 2022-2024  Ian Scott
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

#include "pico_reflash.h"

#include <hardware/flash.h>
#include "../include/pg_debug.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

static union {
    uint8_t buf[512];
    struct UF2_Block {
        // 32 byte header
        uint32_t magicStart0;
        uint32_t magicStart1;
        uint32_t flags;
        uint32_t targetAddr;
        uint32_t payloadSize;
        uint32_t blockNo;
        uint32_t numBlocks;
        uint32_t fileSize; // or familyID;
        uint8_t data[476];
        uint32_t magicEnd;
    } uf2;
} uf2_buf;

#define UF2_FLAG_FAMILY_ID_PRESENT 0x00002000u
#if PICO_RP2040
#define UF2_FAMILY_ID_THIS_CHIP 0xe48bff56u // RP2040
#else
#define UF2_FAMILY_ID_THIS_CHIP 0xe48bff59u // RP2350, Arm Secure
#endif

// Settings live in the last flash sector (see flash_settings.c)
#define FIRMWARE_MAX_SIZE (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static uint32_t pico_firmware_curByte = 0;
static uint32_t pico_firmware_curBlock = 0;
static uint32_t pico_firmware_numBlocks = 0;
static uint32_t pico_firmware_payloadSize = 0;

static volatile pico_firmware_status_t pico_firmware_status = PICO_FIRMWARE_IDLE;

static void pico_firmware_reset(pico_firmware_status_t status)
{
    pico_firmware_curByte = 0;
    pico_firmware_curBlock = 0;
    pico_firmware_numBlocks = 0;
    pico_firmware_payloadSize = 0;
    pico_firmware_status = status;
}

// Blocks are programmed back to back from the start of flash, so only accept
// UF2s for this chip that are laid out that way. The first block is checked
// before anything is erased, so a UF2 for the wrong chip (e.g. an RP2350 build
// on an RP2040 card) is refused without harming the installed firmware.
static bool pico_firmware_block_valid(void)
{
    if (uf2_buf.uf2.magicStart0 != 0x0A324655 || uf2_buf.uf2.magicStart1 != 0x9E5D5157 || uf2_buf.uf2.magicEnd != 0x0AB16F30) {
        // Invalid UF2 file
        ERR_PUTS("Invalid UF2 data!");
        return false;
    }
    if (uf2_buf.uf2.flags & UF2_FLAG_FAMILY_ID_PRESENT) {
        if (uf2_buf.uf2.fileSize != UF2_FAMILY_ID_THIS_CHIP) {
            ERR_PUTS("UF2 is for a different chip!");
            return false;
        }
    } else {
#if !PICO_RP2040
        // Only RP2040 UF2s predate family IDs
        ERR_PUTS("UF2 has no family ID!");
        return false;
#endif
    }
    if (uf2_buf.uf2.payloadSize != FLASH_PAGE_SIZE ||
        uf2_buf.uf2.targetAddr != XIP_BASE + pico_firmware_curBlock * FLASH_PAGE_SIZE) {
        ERR_PUTS("Unsupported UF2 layout!");
        return false;
    }
    if (pico_firmware_curBlock == 0 &&
        (uf2_buf.uf2.numBlocks == 0 || uf2_buf.uf2.numBlocks > FIRMWARE_MAX_SIZE / FLASH_PAGE_SIZE)) {
        ERR_PUTS("Firmware too big!");
        return false;
    }
    return true;
}

static void pico_firmware_process_block(void)
{
    if (!pico_firmware_block_valid()) {
        pico_firmware_reset(PICO_FIRMWARE_ERROR);
        return;
    }
    pico_firmware_status = PICO_FIRMWARE_BUSY;
    if (pico_firmware_curBlock == 0) {
        DBG_PUTS("Starting firmware write...");
        pico_firmware_numBlocks = uf2_buf.uf2.numBlocks;
        DBG_PRINTF("numBlocks: %u\n", pico_firmware_numBlocks);
        pico_firmware_payloadSize = uf2_buf.uf2.payloadSize;
        uint32_t totalSize = pico_firmware_numBlocks * pico_firmware_payloadSize;
        // Point of no return... erasing the flash! Stays clear of the settings sector.
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(0, (totalSize + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE);
        restore_interrupts(ints);
    }
    uint32_t curAddress = pico_firmware_curBlock * pico_firmware_payloadSize;
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(curAddress, uf2_buf.uf2.data, pico_firmware_payloadSize);
    restore_interrupts(ints);
    DBG_PRINTF("curBlock: %u\n", pico_firmware_curBlock);
    ++pico_firmware_curBlock;
    if (pico_firmware_curBlock == pico_firmware_numBlocks) {
        // Final block has been written
        DBG_PUTS("Final block written.");
        pico_firmware_status = PICO_FIRMWARE_DONE;
    } else {
        pico_firmware_status = PICO_FIRMWARE_WRITING;
    }
}


void pico_firmware_write(uint8_t data)
{
    multicore_fifo_push_blocking(data);
}

void firmware_loop() {
    DBG_PUTS("starting core 1");

    for (;;) {
        if (!multicore_fifo_rvalid()) {
            continue;
        }
        uint8_t data = (uint8_t)multicore_fifo_pop_blocking();
        if (pico_firmware_curByte == 0 && pico_firmware_curBlock == 0) {
            // Writing first byte
            pico_firmware_status = PICO_FIRMWARE_WRITING;
        }
        uf2_buf.buf[pico_firmware_curByte++] = data;
        if (pico_firmware_curByte == 512) {
            pico_firmware_process_block();
            pico_firmware_curByte = 0;
        }
    }
}

static void pico_firmware_reboot()
{
    DBG_PUTS("Rebooting!");
    // Stop second core
    multicore_reset_core1();
    // Go back to stock speed
    set_sys_clock_khz(125000, true);
    // Reboot the Pico!
#if PICO_RP2040
    // Undocumented method to reboot the Pico without messing around with the watchdog. Source:
    // https://forums.raspberrypi.com/viewtopic.php?p=1928868&sid=09bfd964ebd49cc6349581ced3b4b9b9#p1928868
    #define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
    AIRCR_Register = 0x5FA0004;
#else
    // On RP2350 SYSRESETREQ is only a processor warm reset; a watchdog reset
    // restarts the rest of the chip as well (the scratch registers survive)
    watchdog_reboot(0, 0, 0);
    for (;;) {
        tight_loop_contents();
    }
#endif
}

void pico_firmware_start()
{
    if (pico_firmware_status == PICO_FIRMWARE_DONE) {
        pico_firmware_reboot();
        return;
    }
    pico_firmware_reset(PICO_FIRMWARE_IDLE);
    // Stop second core
    multicore_reset_core1();
    // Clock down the RP2040 so the flash at its default 1/2 clock divider is within spec (<=133MHz)
    set_sys_clock_khz(240000, true);
    multicore_launch_core1(&firmware_loop);
}

pico_firmware_status_t pico_firmware_getStatus(void)
{
    return pico_firmware_status;
}
