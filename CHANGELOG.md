# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-09-05

### Added

- **ESP32 Bluetooth sacrificial host.** Use a disposable ESP32 as a Bluetooth peer
  so a suspect device (headphones, earbuds, "smart" accessory) never reaches your
  phone, laptop, or anything you care about. The ESP takes the attack surface,
  logs everything, and gets reflashed afterward.
- **BLE GATT central logger + honeypot peripheral** (ESP32-S3, NimBLE). Passive
  scan; connect to a target MAC/name and enumerate every GATT service and
  characteristic (read, subscribe, log notifications); and a fake-peripheral
  honeypot mode that logs every read/write/subscribe the device makes against us.
- **Bluetooth Classic SPP sink** (original ESP32, Arduino `BluetoothSerial`).
  Logs every byte a suspect device sends over serial.
- **Runtime config** in `src/config.h` — target MAC/name substring, auto-connect,
  honeypot enable, advertised name, SPP name/PIN.
- **Two PlatformIO build environments** — `s3-ble-logger` (ESP32-S3, BLE) and
  `esp32-bt-sink` (original ESP32, BT Classic).
- **`docs/pacify-playbook.md`** — containment, neutralization, firmware extraction
  (CH341A double-read), and pitfalls.

### Known gaps — not yet built

- **A2DP-sink + HFP-AG audio/AT-command capture** — needs ESP-IDF (the Arduino
  core exposes only SPP for classic BT). See `docs/pacify-playbook.md` §6.
- **Full HCI snoop** (btmon-equivalent raw link-layer capture).
- **SD-card long-capture logger** (serial is the current sink).
