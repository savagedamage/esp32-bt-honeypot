#include <BluetoothSerial.h>
#include "config.h"
#include "log.h"

static BluetoothSerial SerialBT;
static bool lastClientState = false;

void btClassicSetup() {
    // Basic controller init happens inside BluetoothSerial.begin().
    SerialBT.begin(SPP_NAME);
    SerialBT.enableSSP();          // Secure Simple Pairing (Just Works in lab mode)
    SerialBT.setPin(SPP_PIN);      // legacy PIN fallback for pre-2.1 headsets

    LOG_I("BT Classic SPP server up as '%s' (PIN %s).", SPP_NAME, SPP_PIN);
    LOG_I("Put the suspect device in pairing mode and accept the link HERE,");
    LOG_I("never on your phone/laptop. All RX bytes are hex-logged below.");
}

void btClassicLoop() {
    bool hasClient = SerialBT.hasClient();
    if (hasClient != lastClientState) {
        lastClientState = hasClient;
        LOG_I("%s", hasClient ? "SPP CLIENT CONNECTED" : "SPP CLIENT DISCONNECTED");
    }

    if (hasClient) {
        while (SerialBT.available()) {
            uint8_t buf[128];
            size_t n = 0;
            while (n < sizeof(buf) && SerialBT.available()) {
                int b = SerialBT.read();
                if (b < 0) break;
                buf[n++] = (uint8_t)b;
            }
            if (n > 0) {
                logHex("SPP-RX", "classic", buf, n);
            }
        }
    }
    delay(10);
}
