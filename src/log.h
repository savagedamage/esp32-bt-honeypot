#pragma once
#include <Arduino.h>

// Tagged, timestamped serial logging. Logs are the whole point of this tool —
// everything the suspect device does gets written here (and you redirect the
// monitor to a file for evidence:  pio device monitor > capture.log).

#define LOG_I(fmt, ...) Serial.printf("[%10lu] I " fmt "\n", (unsigned long)millis(), ##__VA_ARGS__)
#define LOG_W(fmt, ...) Serial.printf("[%10lu] W " fmt "\n", (unsigned long)millis(), ##__VA_ARGS__)
#define LOG_E(fmt, ...) Serial.printf("[%10lu] E " fmt "\n", (unsigned long)millis(), ##__VA_ARGS__)

void logInit();
// Dump a byte buffer as hex, tagged, with a length header.
void logHex(const char* tag, const char* id, const uint8_t* data, size_t len);
