#pragma once

// BT Classic sacrificial sink (original ESP32 only — S3 has no classic BT).
// Runs an SPP server: the suspect device can pair/connect to us instead of a
// real host, and every byte it sends is hex-logged. Classic-BT audio (A2DP)
// and hands-free (HFP/AT-command) capture are the ESP-IDF extension documented
// in docs/pacify-playbook.md — the Arduino core only exposes SPP for classic.
void btClassicSetup();
void btClassicLoop();
