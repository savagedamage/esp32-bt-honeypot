// Pure, hardware-independent helpers for the ESP32 Bluetooth
// Sacrificial Host.
//
// Kept as a header-only library with NO Arduino / NimBLE / ESP-IDF / FreeRTOS
// dependencies so it can be unit-tested on a plain host (see tests/test_core.cpp
// and the CI workflow). Only <string> / <cstdint> / <cstddef> below.
#pragma once
#include <string>
#include <cstdint>
#include <cstddef>

namespace honeypot_core {

// Feature version, shown on the serial banner and used to track releases.
static const char* kVersion = "0.1.1";

// Returns true if an advertised BLE device (addr, name) should be targeted
// given the configured target MAC and/or target-name substring.
//    - non-empty targetMac   -> exact (case-sensitive) MAC match on addr.
//    - non-empty targetName  -> substring (case-sensitive) match on name.
//    - both empty            -> nothing matches; caller should run passive-only.
// This deliberately mirrors the C++ the firmware does so it can be pinned down
// by a host test.
inline bool targetMatches(const std::string& addr, const std::string& name,
                          const std::string& targetMac,
                          const std::string& targetName) {
    if (!targetMac.empty() && addr == targetMac) return true;
    if (!targetName.empty() && name.find(targetName) != std::string::npos) {
        return true;
    }
    return false;
}

// Hex-encoded bytes as an uppercase, space-separated string (e.g. "DE AD BE EF").
// Mirrors the on-serial hex dump so the exact formatting can be unit-tested on
// a host; the firmware prints the result and the tests assert the same string.
inline std::string hexBytes(const uint8_t* data, size_t len) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    out.reserve(len * 3);
    for (size_t i = 0; i < len; i++) {
        if (i) out += ' ';
        out += kHex[(data[i] >> 4) & 0x0F];
        out += kHex[data[i] & 0x0F];
    }
    return out;
}

} // namespace honeypot_core