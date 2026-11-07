// Memory map of the web server's buffers in the RP2040's XIP cache SRAM.
//
// PicoGUS runs entirely from RAM and turns the XIP (flash) cache off at boot
// (see main() in picogus.cpp). With the cache off, its 16KB of memory can be
// used as ordinary SRAM at 0x15000000 - so the network buffers live there and
// don't take main RAM away from the sound card emulation.
//
// Only core 1 touches this memory, and only from web_task().
// Plain defines only: this file is also included by lwipopts.h.

#ifndef _WEB_XIP_SRAM_H
#define _WEB_XIP_SRAM_H

#define WEB_XIP_SRAM_BASE   0x15000000u
#define WEB_XIP_SRAM_SIZE   0x4000u

// First part: buffers of the web server itself (web_xip_alloc())
#define WEB_XIP_WEB_SIZE    0x1400u
// Rest: lwIP's heap, from which all of lwIP's memory is allocated
#define WEB_XIP_LWIP_BASE   (WEB_XIP_SRAM_BASE + WEB_XIP_WEB_SIZE)
#define WEB_XIP_LWIP_SIZE   (WEB_XIP_SRAM_SIZE - WEB_XIP_WEB_SIZE)

#endif
