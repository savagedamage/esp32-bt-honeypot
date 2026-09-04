# ESP32 Bluetooth Sacrificial Host

[![CI](https://github.com/savagedamage/esp32-bt-honeypot/actions/workflows/ci.yml/badge.svg)](https://github.com/savagedamage/esp32-bt-honeypot/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Use your ESP32 boards as a **sacrificial Bluetooth peer** so suspect Bluetooth
devices (headphones, earbuds, "smart" accessories) never touch your phone,
laptop, or anything you care about. The ESP32 presents the attack surface,
logs everything the device does, and gets reflashed afterward. This is the
"divert the danger to the ESP" machine, plus the workflow to extract and then
**pacify** (contain/neutralize) whatever malware the device is carrying.

```
 suspect BT device ──(Bluetooth)──► ESP32 sacrificial host ──► UART log (evidence)
        (headphones)                    │
                                        └── you reflash the ESP afterwards, it's disposable
 your phone/laptop  ✗  never in the path
```

---

## 0. The one fact that decides everything

Bluetooth comes in two flavours and they live on **different silicon**:

| Board you own | Radio | Use it for |
|---|---|---|
| **original ESP32** (micro-USB, CP2102/CH340 bridge) | BT Classic (BR/EDR) **+** BLE | Classic headsets — SPP, A2DP, HFP |
| **ESP32-S3** (USB-C) | **BLE 5 only** — no classic BT | BLE buds / companion-app traffic |
| **ESP32-S2** (USB-C) | Wi-Fi only — no BT at all | not usable here |

So: **classic Bluetooth headphones → original ESP32. BLE-only buds → S3.**
If you don't know which a device is yet, assume classic and start with the
original board — most full-size headphones are BR/EDR.

---

## 1. Threat model — what a "malicious headphone" actually is

A hostile BT device is usually one of these (all observable without ever
letting it near your real hardware):

1. **Covert microphone** — records ambient audio, exfils over the HFP/A2DP
   uplink or a hidden SPP/BLE channel.
2. **Data exfil channel** — a hidden SPP server or BLE GATT service that pushes
   captured data (or receives C2) over a profile that shouldn't exist on a
   "headphone".
3. **C2 implant** — the device phones home to a C2 over its own radio
   (Wi-Fi/2.4 GHz/GSM) on schedule. Detected by RF sweep, not by pairing.
4. **Companion-app abuse** — a "headphone" whose app demands broad permissions
   and the device drives the app via BLE GATT. The honeypot catches the GATT
   traffic.

---

## 2. The sacrificial-machine architecture

Two complementary capture paths, both "the ESP takes the hit, your devices
never do":

**A. Behaviour capture (the ESP is the peer).** Flash the sacrificial firmware
below. The ESP becomes the device's Bluetooth partner and logs the full
conversation to serial. This catches live exfil and GATT/HFP/SPP behaviour.

- BLE: two modes in one firmware —
  - **Central probe** (scan → connect → enumerate every GATT service/char →
    read → subscribe → log notifications). Good once you know the address.
  - **Honeypot peripheral** (advertise a fake companion/headset service and log
    every read/write/subscribe the device makes against us). This is the
    "device attacks the ESP" mode — the device thinks we're its victim/phone.
- BT Classic: SPP server logs every byte. A2DP-sink + HFP-AG (to capture the
  audio uplink and AT commands) is the ESP-IDF extension in
  `docs/pacify-playbook.md`.

**B. Firmware capture (read the device's own flash).** CH341A clip → `flashrom`
double-read → `binwalk`/`strings`/Ghidra for C2 domains, keys, backdoors. This
finds the implant even when it never transmits during a session. See
`docs/pacify-playbook.md` §5 and the `implant-firmware-extraction` skill.

**C. RF sweep (catches the radio you can't see by pairing).** Baseline the room
with an SDR, power the device in a Faraday enclosure, sweep 1 MHz–6 GHz, diff.
Catches BLE/Wi-Fi/GSM exfil and hidden 2.4 GHz radios. See `rf-emission-scanning`.

---

## 3. Build & flash

```
cd esp32-bt-honeypot

# BLE logger + honeypot  -> ESP32-S3 (or any BLE board)
pio run -e s3-ble-logger
pio run -e s3-ble-logger -t upload
pio device monitor -e s3-ble-logger > ble-capture.log

# BT Classic SPP sink -> original ESP32 (micro-USB)
pio run -e esp32-bt-sink
pio run -e esp32-bt-sink -t upload
pio device monitor -e esp32-bt-sink > spp-capture.log
```

The serial log is your evidence — always redirect to a file and hash it.

---

## 4. Operating procedure (per suspect device — scales to any count)

1. **CONTAIN** — photograph + label the device. Power it OFF. Remove the
   battery/SIM if removable. Place it in a **tested Faraday pouch** (mylar/ESD
   bags are ~6-9 dB and useless; a real pouch is ~90-110 dB). Keep it away from
   your phone/laptop.
2. **CLASSIFY** — wired vs wireless; classic vs BLE. This picks the board
   (§0) and the firmware.
3. **BASELINE RF** — sweep the room *without* the device (SDR). Save it.
4. **CAPTURE (sacrificial ESP)** — flash the right firmware. Put the device in
   pairing mode and complete pairing **on the ESP only**. Exercise the device
   (play, tap buttons, connect to its app's BLE side) while the ESP logs.
5. **CAPTURE (firmware)** — open the device (if you're willing to lose the
   housing), CH341A double-read the SPI flash. `sha256sum` the two dumps; they
   must match.
6. **ANALYZE** — `strings`/`binwalk`/Ghidra the dump for C2 domains/IPs, hidden
   profiles, exfil paths. Diff the ESP log for unexpected GATT/SPP/HFP traffic.
7. **PACIFY** — see `docs/pacify-playbook.md`. Minimum: back in the Faraday
   pouch, battery disconnected, never reflashed, documented. Optionally
   physically disable (battery removal) for long-term quarantine.
8. **DOCUMENT** — hash manifest, MAC/name, firmware hashes, IOCs, timeline →
   threat brief.

---

## 5. What is / isn't built yet (honest status)

- [x] BLE GATT central logger + honeypot peripheral (Arduino/NimBLE 2.x, S3) — **compiles clean, verified**
- [x] BT Classic SPP sink (Arduino core, original ESP32) — **compiles clean, verified**
- [ ] A2DP-sink + HFP-AG audio/AT capture → needs ESP-IDF directly (Arduino core
      exposes only SPP for classic). Example paths + build steps in
      `docs/pacify-playbook.md` §6.
- [ ] Full HCI snoop (raw link-layer capture, btmon-equivalent) → same ESP-IDF path.
- [ ] SD-card logger for long captures (serial is the current sink).

---

## 6. Scope boundary

This is **defensive analysis of devices you own or are authorized to examine.**
It covers containment, traffic capture, firmware extraction, static/dynamic
analysis in a lab, and neutralization (the "pacify" step). It does **not**
cover reflashing a device to exfiltrate to your own C2, or extending whatever
malware is found into exploit chains against other targets. The publishable
result is lab characterization of the original malware with dummy data, plus
mitigations — that's also the part that has career/credit value.
