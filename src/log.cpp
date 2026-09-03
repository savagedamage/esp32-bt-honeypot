#include "log.h"
#include "config.h"

void logInit() {
    Serial.begin(LOG_SERIAL_BAUD);
    delay(200);
    Serial.println();
    Serial.println("==============================================");
    Serial.println(" ESP32 Bluetooth Sacrificial Host  (v0.1.0)");
    Serial.println(" All output below is live capture of the");
    Serial.println(" suspect device's traffic. Redirect to file.");
    Serial.println("==============================================");
}

void logHex(const char* tag, const char* id, const uint8_t* data, size_t len) {
    Serial.printf("[%10lu] %s  %s  (%u bytes):\n", (unsigned long)millis(), tag, id, (unsigned)len);
    for (size_t i = 0; i < len; i++) {
        Serial.printf("%02X ", data[i]);
        if ((i & 15) == 15) Serial.println();
    }
    if (len == 0 || (len & 15) != 0) Serial.println();
}
