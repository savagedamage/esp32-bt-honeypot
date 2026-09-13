#include "log.h"
#include "config.h"
#include <honeypot_core.h>

void logInit() {
    Serial.begin(LOG_SERIAL_BAUD);
    delay(200);
    Serial.println();
    Serial.println("==============================================");
    Serial.printf(" ESP32 Bluetooth Sacrificial Host  (v%s)\n", honeypot_core::kVersion);
    Serial.println(" All output below is live capture of the");
    Serial.println(" suspect device's traffic. Redirect to file.");
    Serial.println("==============================================");
}

void logHex(const char* tag, const char* id, const uint8_t* data, size_t len) {
    Serial.printf("[%10lu] %s  %s  (%u bytes):\n", (unsigned long)millis(), tag, id, (unsigned)len);
    for (size_t off = 0; off < len; off += 16) {
        size_t n = len - off;
        if (n > 16) n = 16;
        Serial.println(honeypot_core::hexBytes(data + off, n).c_str());
    }
    if (len == 0) Serial.println();
}