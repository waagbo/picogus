// Host tests for MIDI injection from the web UI (mpu401/midi.c, WEBSERVER).
// Injected messages must never split a message of the DOS program's MIDI
// stream, and sysex must be paced.

#include <stdio.h>
#include <string.h>
#include "mpu401/export.h"

void MIDI_Init(bool delaysysex, bool fakeallnotesoff);

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

uint64_t test_now_us = 1000000;
uart_inst_t *uart0;
static uint8_t wire[4096];
static size_t wire_len;
void uart_write_blocking(uart_inst_t *u, const uint8_t *src, size_t len) { (void)u; memcpy(wire + wire_len, src, len); wire_len += len; }
void uart_tx_wait_blocking(uart_inst_t *u) { (void)u; }
bool uart_is_writable(uart_inst_t *u) { (void)u; return true; }

static void host(const uint8_t *b, size_t n) { for (size_t i = 0; i < n; ++i) MIDI_RawOutByte(b[i]); }
// run the core 1 loop for ms milliseconds, at MIDI wire speed (~3 bytes/ms)
static void run(int ms) { for (int i = 0; i < ms * 3; ++i) { test_now_us += 333; send_midi_bytes(1); } }
static void reset(void) { MIDI_Init(false, false); run(50); wire_len = 0; }

static bool wire_is(const uint8_t *want, size_t n) {
    if (wire_len == n && !memcmp(wire, want, n)) return true;
    printf("  wire (%zu):", wire_len);
    for (size_t i = 0; i < wire_len; ++i) printf(" %02X", wire[i]);
    printf("\n");
    return false;
}

int main(void) {
    static const uint8_t note[] = {0x90, 0x3C, 0x7F};
    static const uint8_t sx[] = {0xF0, 0x7D, 0x03, 0x01, 0xF7};

    // queued host data goes first, injected message after it
    reset();
    host(note, 3);
    CHECK(MIDI_Inject(sx, sizeof(sx)), "inject");
    run(10);
    {
        static const uint8_t want[] = {0x90, 0x3C, 0x7F, 0xF0, 0x7D, 0x03, 0x01, 0xF7};
        CHECK(wire_is(want, sizeof(want)), "host message, then injected one");
    }

    // a host sysex in progress is not interrupted
    reset();
    static const uint8_t hsx1[] = {0xF0, 0x41, 0x10, 0x16, 0x12};
    host(hsx1, sizeof(hsx1));
    run(5);
    CHECK(MIDI_Inject(note, 3), "inject");
    run(20);
    CHECK(wire_len == sizeof(hsx1), "injected note waits for the host sysex to end (%zu)", wire_len);
    static const uint8_t hsx2[] = {0x20, 0x00, 0x00, 0x41, 0xF7};
    host(hsx2, sizeof(hsx2));
    run(10);
    {
        static const uint8_t want[] = {0xF0, 0x41, 0x10, 0x16, 0x12, 0x20, 0x00, 0x00, 0x41, 0xF7, 0x90, 0x3C, 0x7F};
        CHECK(wire_is(want, sizeof(want)), "note after the host sysex");
    }

    // an injected sysex is paced: the host stream waits for the synth
    reset();
    CHECK(MIDI_Inject(sx, sizeof(sx)), "inject");
    run(3);
    host(note, 3);
    run(5);
    CHECK(wire_len == sizeof(sx), "host waits after an injected sysex (%zu)", wire_len);
    run(30);
    CHECK(wire_len == sizeof(sx) + 3, "host continues after the sysex delay (%zu)", wire_len);

    // the injected message is sent whole even when host data arrives meanwhile
    reset();
    static const uint8_t lcd[] = {0xF0, 0x41, 0x10, 0x16, 0x12, 0x20, 0, 0, 'H', 'I', 0x7E, 0xF7};
    CHECK(MIDI_Inject(lcd, sizeof(lcd)), "inject");
    for (int i = 0; i < 3; ++i) { test_now_us += 333; send_midi_bytes(1); }
    host(note, 3);
    run(100);
    {
        uint8_t want[sizeof(lcd) + 3];
        memcpy(want, lcd, sizeof(lcd));
        memcpy(want + sizeof(lcd), note, 3);
        CHECK(wire_is(want, sizeof(want)), "injected sysex stays whole");
    }

    // all or nothing when the queue is full
    reset();
    static uint8_t big[1000];
    big[0] = 0xF0; memset(big + 1, 0x11, sizeof(big) - 2); big[sizeof(big) - 1] = 0xF7;
    CHECK(MIDI_Inject(big, sizeof(big)), "big inject");
    CHECK(!MIDI_Inject(big, sizeof(big)), "second big inject refused");
    CHECK(MIDI_InjectFree() == 1023 - sizeof(big), "free space %u", MIDI_InjectFree());

    printf("%s: %d failure(s)\n", failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
