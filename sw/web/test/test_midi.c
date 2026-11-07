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

// simulated execution context and MPU-401 mode
static unsigned core = 1, exception = 0;
static bool intelligent;
unsigned get_core_num(void) { return core; }
unsigned __get_current_exception(void) { return exception; }
bool MPU401_IsIntelligent(void) { return intelligent; }

static void host(const uint8_t *b, size_t n) { for (size_t i = 0; i < n; ++i) MIDI_RawOutByte(b[i]); }
// run the core 1 loop for ms milliseconds, at MIDI wire speed (~3 bytes/ms)
static void run(int ms) { for (int i = 0; i < ms * 3; ++i) { test_now_us += 333; send_midi_bytes(1); } }
static void reset(void) { MIDI_Init(false, false); run(1000); wire_len = 0; }

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

    // MPU-401 intelligent mode: injected messages wait
    reset();
    intelligent = true;
    CHECK(MIDI_Inject(note, 3), "inject");
    run(20);
    CHECK(wire_len == 0 && MIDI_InjectHeld(), "held in intelligent mode (%zu)", wire_len);
    intelligent = false;
    run(5);
    CHECK(wire_is(note, 3), "sent after leaving intelligent mode");

    // intelligent mode flushing MIDI out from core 0 (or an interrupt) while an
    // injected sysex is being sent: the rest of it is dropped, nobody waits
    reset();
    static const uint8_t longsx[] = {0xF0, 0x41, 0x10, 0x16, 0x12, 1, 2, 3, 4, 5, 6, 7, 8, 0xF7};
    CHECK(MIDI_Inject(longsx, sizeof(longsx)), "inject");
    CHECK(MIDI_Inject(note, 3), "inject note after it");
    for (int i = 0; i < 4; ++i) { test_now_us += 333; send_midi_bytes(1); }
    CHECK(wire_len == 4, "4 sysex bytes out (%zu)", wire_len);
    static const uint8_t hnote[] = {0x80, 0x3C, 0x00};
    host(hnote, 3);
    core = 0;
    for (int i = 0; i < 3; ++i) { test_now_us += 333; send_midi_bytes(1); }
    core = 1;
    {
        static const uint8_t want[] = {0xF0, 0x41, 0x10, 0x16, 0x80, 0x3C, 0x00};
        CHECK(wire_is(want, sizeof(want)), "core 0 sends host data right away");
    }
    run(50);
    {
        static const uint8_t want[] = {0xF0, 0x41, 0x10, 0x16, 0x80, 0x3C, 0x00, 0x90, 0x3C, 0x7F};
        CHECK(wire_is(want, sizeof(want)), "rest of the cut sysex dropped, next message intact");
    }

    printf("%s: %d failure(s)\n", failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
