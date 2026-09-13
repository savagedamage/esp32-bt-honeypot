// Host unit tests for honeypot_core (pure, hardware-independent logic).
//
// Build & run (no ESP32 required):
//     g++ -std=c++17 -I lib/honeypot_core/src tests/test_core.cpp -o /tmp/test_core
//     /tmp/test_core
//
// This is wired into the CI workflow (tests step) so the pure logic is
// continuously checked. The firmware itself is built by PlatformIO for the
// two board targets — this test covers the logic that's extractable off-device.
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

#include "honeypot_core.h"

using honeypot_core::hexBytes;
using honeypot_core::targetMatches;

static void test_targetMatches() {
    // MAC exact match
    assert(targetMatches("aa:bb:cc:dd:ee:ff", "AirPods",
                         "aa:bb:cc:dd:ee:ff", ""));
    // Name substring match
    assert(targetMatches("11:22:33", "AirPods Pro", "", "AirPods"));
    assert(targetMatches("11:22:33", "JBL Tune 510BT", "", "Tune"));
    // Non-matches
    assert(!targetMatches("aa:bb", "AirPods", "11:22", "Galaxy"));
    assert(!targetMatches("aa:bb", "AirPods", "", ""));
    assert(!targetMatches("aa:bb", "AirPods", "11:22", ""));
    assert(!targetMatches("", "", "", "AirPods"));
    // MAC match wins even if name doesn't
    assert(targetMatches("aa:bb", "noise", "aa:bb", ""));
}

static void test_hexBytes() {
    uint8_t b[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    assert(hexBytes(b, 4) == "DE AD BE EF");

    uint8_t one[1] = {0x0A};
    assert(hexBytes(one, 1) == "0A");

    uint8_t max[2] = {0xFF, 0x00};
    assert(hexBytes(max, 2) == "FF 00");

    // empty buffer -> empty string (no trailing space)
    assert(hexBytes(nullptr, 0) == "");
}

int main() {
    test_targetMatches();
    test_hexBytes();
    std::printf("honeypot_core: all tests passed\n");
    return 0;
}