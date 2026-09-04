#include <NimBLEDevice.h>
#include <string.h>
#include "config.h"
#include "log.h"

static NimBLEClient*   pClient = nullptr;

// ---------------------------------------------------------------------------
// Central probe: notification/indication handler. Every value the device
// pushes at us lands here and gets hex-logged.
// ---------------------------------------------------------------------------
static void onNotify(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t len, bool isNotify) {
    const NimBLERemoteService* svc = chr->getRemoteService();
    char id[96];
    snprintf(id, sizeof(id), "%s / %s",
             svc ? svc->getUUID().toString().c_str() : "?",
             chr->getUUID().toString().c_str());
    logHex(isNotify ? "NOTIFY" : "INDICATE", id, data, len);
}

// ---------------------------------------------------------------------------
// Honeypot peripheral callbacks
// ---------------------------------------------------------------------------
#if BLE_HONEYPOT
class HoneypotCharCb : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* chr, NimBLEConnInfo& connInfo) override {
        LOG_I("HONEYPOT read  %s  by %s", chr->getUUID().toString().c_str(),
              connInfo.getAddress().toString().c_str());
    }
    void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& connInfo) override {
        std::string v = chr->getValue();
        char id[64];
        snprintf(id, sizeof(id), "%s (from %s)", chr->getUUID().toString().c_str(),
                 connInfo.getAddress().toString().c_str());
        logHex("HONEYPOT-WRITE", id, (const uint8_t*)v.data(), v.size());
    }
    void onSubscribe(NimBLECharacteristic* chr, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        LOG_I("HONEYPOT subscribe %s value=%u by %s", chr->getUUID().toString().c_str(),
              subValue, connInfo.getAddress().toString().c_str());
    }
};

class HoneypotServerCb : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* s, NimBLEConnInfo& connInfo) override {
        LOG_I("HONEYPOT CONNECT from %s", connInfo.getAddress().toString().c_str());
    }
    void onDisconnect(NimBLEServer* s, NimBLEConnInfo& connInfo, int reason) override {
        LOG_I("HONEYPOT DISCONNECT from %s reason=%d", connInfo.getAddress().toString().c_str(), reason);
    }
    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        LOG_I("HONEYPOT MTU=%u from %s", MTU, connInfo.getAddress().toString().c_str());
    }
};
#endif // BLE_HONEYPOT

// ---------------------------------------------------------------------------
// Honeypot GATT server: a fake companion/headset service the device will talk
// to. Device Information (0x180A) plus a custom FFF0 "data sink" with a
// writable RX and a notifiable TX characteristic — the two shapes almost every
// companion-app profile uses.
// ---------------------------------------------------------------------------
#if BLE_HONEYPOT
static void startHoneypot() {
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new HoneypotServerCb());

    NimBLEService* dis = pServer->createService("180A");           // Device Information
    NimBLECharacteristic* mfr = dis->createCharacteristic("2A29", NIMBLE_PROPERTY::READ);
    mfr->setValue("ESP32-Sacrificial");
    mfr->setCallbacks(new HoneypotCharCb());

    NimBLEService* cs = pServer->createService("0000FFF0-0000-1000-8000-00805F9B34FB");
    NimBLECharacteristic* rx = cs->createCharacteristic(
        "0000FFF1-0000-1000-8000-00805F9B34FB",
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    rx->setCallbacks(new HoneypotCharCb());
    NimBLECharacteristic* tx = cs->createCharacteristic(
        "0000FFF2-0000-1000-8000-00805F9B34FB",
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    tx->setCallbacks(new HoneypotCharCb());

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(dis->getUUID());
    adv->addServiceUUID(cs->getUUID());
    adv->setName(BLE_HONEYPOT_NAME);
    adv->start();
    LOG_I("Honeypot advertising as '%s' (0x180A + 0xFFF0)", BLE_HONEYPOT_NAME);
}
#endif // BLE_HONEYPOT

// ---------------------------------------------------------------------------
// Central probe: one scan + connect + enumerate cycle.
// ---------------------------------------------------------------------------
static void scanAndConnect() {
    LOG_I("Scanning for %u ms (active)...", (unsigned)BLE_SCAN_MS);
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(99);
    pScan->start(BLE_SCAN_MS, false);
    NimBLEScanResults results = pScan->getResults();

    LOG_I("Found %d advertiser(s):", results.getCount());
    int matchIndex = -1;
    for (int i = 0; i < results.getCount(); i++) {
        const NimBLEAdvertisedDevice* dev = results.getDevice(i);
        if (!dev) continue;
        std::string name = dev->getName();
        std::string addr = dev->getAddress().toString();
        LOG_I("  [%d] %s  '%s'  RSSI=%d", i, addr.c_str(), name.c_str(), dev->getRSSI());
        if (matchIndex < 0) {
            if (strlen(HONEYPOT_TARGET_MAC) && addr == HONEYPOT_TARGET_MAC) {
                matchIndex = i;
            } else if (strlen(HONEYPOT_TARGET_NAME) &&
                       name.find(HONEYPOT_TARGET_NAME) != std::string::npos) {
                matchIndex = i;
            }
        }
    }

    if (!BLE_AUTO_CONNECT) {
        LOG_W("Passive mode (BLE_AUTO_CONNECT=0) — no connection made.");
        return;
    }
    if (matchIndex < 0) {
        LOG_W("No target match. Set HONEYPOT_TARGET_MAC or HONEYPOT_TARGET_NAME in config.h.");
        return;
    }

    const NimBLEAdvertisedDevice* dev = results.getDevice(matchIndex);
    if (!dev) { LOG_W("Target not found in scan results."); return; }
    LOG_I("Connecting to %s ...", dev->getAddress().toString().c_str());

    if (pClient) { NimBLEDevice::deleteClient(pClient); }
    pClient = NimBLEDevice::createClient();
    pClient->setConnectionParams(12, 12, 0, 51);   // 15 ms interval, latency 0, 510 ms timeout
    if (!pClient->connect(dev)) {
        LOG_E("Connect failed.");
        return;
    }
    LOG_I("Connected. MTU=%u", (unsigned)pClient->getMTU());

    const std::vector<NimBLERemoteService*>& services = pClient->getServices(true);
    if (services.empty()) { LOG_W("No services reported."); return; }
    LOG_I("Device exposes %u service(s):", (unsigned)services.size());
    for (auto svc : services) {
        LOG_I("  SERVICE %s", svc->getUUID().toString().c_str());
        const std::vector<NimBLERemoteCharacteristic*>& chars = svc->getCharacteristics(true);
        if (chars.empty()) continue;
        for (auto chr : chars) {
            LOG_I("    CHAR %s  handle=0x%04X",
                  chr->getUUID().toString().c_str(), chr->getHandle());
            if (chr->canRead()) {
                std::string val = chr->readValue();
                logHex("READ", chr->getUUID().toString().c_str(),
                       (const uint8_t*)val.data(), val.size());
            }
            if (chr->canNotify() || chr->canIndicate()) {
                bool ok = chr->subscribe(true, onNotify, true);
                LOG_I("    subscribed (notify/indicate) = %s", ok ? "ok" : "FAIL");
            }
        }
    }
    LOG_I("Enumeration complete. Now streaming notifications until disconnect.");
}

// ---------------------------------------------------------------------------
void bleSetup() {
    NimBLEDevice::init(BLE_HONEYPOT_NAME);
#if BLE_HONEYPOT
    startHoneypot();
#endif
}

void bleLoop() {
    if (!pClient || !pClient->isConnected()) {
        static uint32_t lastScan = 0;
        if (millis() - lastScan > 5000) {
            lastScan = millis();
            scanAndConnect();
        }
    }
    delay(100);
}
