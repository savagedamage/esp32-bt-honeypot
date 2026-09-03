#pragma once
// ============================================================================
// ESP32 Bluetooth Sacrificial Host — configuration
// ============================================================================
// Build mode is selected in platformio.ini via -DHONEYPOT_MODE_BLE or
// -DHONEYPOT_MODE_BTCLASSIC. This file is the runtime knob board.

// ---- BLE central probe: target selection --------------------------------
// Leave both empty + BLE_AUTO_CONNECT=0 to just scan and log every advertiser
// without connecting (passive recon). Set a MAC or a name substring to make
// the firmware connect and enumerate the device automatically.
#define HONEYPOT_TARGET_MAC   ""               // e.g. "aa:bb:cc:dd:ee:ff"
#define HONEYPOT_TARGET_NAME  ""               // substring, e.g. "AirPods"

// ---- BLE behaviour --------------------------------------------------------
#define BLE_SCAN_MS           8000             // scan window per cycle
#define BLE_AUTO_CONNECT      1                // 1 = connect to first match
#define BLE_HONEYPOT          1                // 1 = also run a honeypot GATT server
#define BLE_HONEYPOT_NAME     "esp32-honeypot" // advertised name (<= ~20 chars)

// ---- BT Classic (SPP) ----------------------------------------------------
#define SPP_NAME              "ESP32-Sacrificial"
#define SPP_PIN               "1234"           // legacy PIN fallback for old gear

// ---- Logging ---------------------------------------------------------------
#define LOG_SERIAL_BAUD       115200
