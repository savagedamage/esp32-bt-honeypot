#include <Arduino.h>
#include "config.h"
#include "log.h"

#if defined(HONEYPOT_MODE_BLE)
#include "ble_gatt_logger.h"
#elif defined(HONEYPOT_MODE_BTCLASSIC)
#include "bt_classic_sink.h"
#endif

void setup() {
    logInit();
#if defined(HONEYPOT_MODE_BLE)
    LOG_I("Mode: BLE GATT logger + honeypot");
    bleSetup();
#elif defined(HONEYPOT_MODE_BTCLASSIC)
    LOG_I("Mode: BT Classic SPP sink");
    btClassicSetup();
#else
    LOG_E("No mode defined. Pass -DHONEYPOT_MODE_BLE or -DHONEYPOT_MODE_BTCLASSIC.");
#endif
}

void loop() {
#if defined(HONEYPOT_MODE_BLE)
    bleLoop();
#elif defined(HONEYPOT_MODE_BTCLASSIC)
    btClassicLoop();
#endif
}
