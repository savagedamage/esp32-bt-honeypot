# Pacify Playbook — contain & neutralize a suspect Bluetooth device

"Pacify" = make the device stop being a threat to your real gear and to the
people around it, without destroying the evidence. Order matters: contain
first, then capture, then neutralize. Read-only discipline throughout.

---

## 1. Contain (immediate, always)

- Power OFF. Remove battery/SIM/SD if removable. If not removable, tape the
  power button and rely on the pouch.
- **Tested Faraday pouch only.** Mylar/anti-static bags give ~6-9 dB (useless);
  a real pouch (Mission Darkness / EDEC OffGrid) is ~90-110 dB. Test every pouch:
  drop a paired phone or a BLE beacon inside and confirm the signal hits the
  noise floor on your SDR / scanner. Velcro seams and cable passthroughs leak
  above ~3 GHz — re-test at 2.4 GHz too.
- Keep the device in the pouch whenever it is NOT actively wired to the
  sacrificial ESP. Never leave it near your phone, laptop, or any paired device.

## 2. Neutralize (make it inert)

Ranked, least destructive first:

1. **Battery disconnect** — the single most effective pacify. A dead radio is a
   contained radio. If the cell is removable, pull it. If soldered, leave it
   (unless you're willing to desolder — then do it last, after firmware dump).
2. **Never reflash the malicious image.** The implant's firmware is evidence.
   Do not `flashrom -w/-E`, do not `esptool write_flash/erase_flash`, do not
   pair it to anything else. Read-only.
3. **Forget it on every host** it was ever paired to (your phone/laptop BT
   "forget this device"), so it can't re-associate later. Do this BEFORE you
   know what it is, cheap insurance.
4. **Long-term quarantine** — labeled anti-static bag inside the Faraday pouch,
   stored away from mains and people. Log it (date, device, hash, owner,
   disposition) as evidence.

## 3. The "sacrificial" part (why the ESP is safe to sacrifice)

- The ESP32 is a $5-15 disposable. You flash it with our logger, let the
  suspect device do whatever it wants to it, read the log, then **reflash it
  clean** (or bin it). The device can't reach past the ESP because the ESP
  runs no bridge to your network — it only writes to serial.
- Never bridge the ESP's USB to your workstation while a suspect device is
  connected unless you've read §5 and understand the risk. Serial log only,
  redirected to a file.

## 4. What "pacified" looks like (acceptance criteria)

A device counts as pacified when ALL hold:
- [ ] Powered off + battery removed/disconnected (or in a tested pouch if not)
- [ ] Never reflashed; firmware dumps (2x, matching sha256) saved off-device
- [ ] Forgotten on every host it was paired to
- [ ] No new RF/BLE/BT emissions vs. room baseline while it sits "off" (SDR)
- [ ] Documented: hash manifest, MAC/name, IOCs, photos, timeline

## 5. Firmware extraction (the evidence that survives neutralization)

Dump BEFORE you neutralize hardware you can't reopen, because the flash holds
the implant even if the radio is dead.

```
# SPI NOR via CH341A + SOIC8 clip — READ ONLY, double read, compare
flashrom -p ch341a_spi                      # detect chip (must name a W25Qxx etc.)
flashrom -p ch341a_spi -r d1.bin
flashrom -p ch341a_spi -r d2.bin
sha256sum d1.bin d2.bin                     # MUST match
flashrom -p ch341a_spi -v d1.bin            # read-back verify (safe)

# static triage
strings -n 6 d1.bin | grep -aiE 'https?://|[0-9]{1,3}(\.[0-9]{1,3}){3}|\.onion|\.com' > urls.txt
strings -n 6 d1.bin | grep -aiE 'password|admin|telnet|ssh|/bin/sh|nc -|wget|curl|backdoor|at\+'
binwalk d1.bin                              # partition/fs map
```

- SOIC-8 pin 1 = /CS, pin 3 = /WP, pin 7 = /HOLD. If /WP is tied low, hardware
  write-protect is on — leave it. Match chip voltage (1.8 V chips need a
  level-shifter). Power the chip from the programmer only, never the board.
- If the device has no external flash, the MCU holds it internally → SWD/JTAG or
  `esptool` (for ESP-based fakes). `esptool.py image_info` parses the partition
  table (0x8000) and bootloader (0x1000).
- If the MCU has readout protection (RDP), STOP — do not attempt a bypass on a
  device you don't own; on your own device, note that unlocking erases flash.

## 6. Full audio/AT-command capture — the ESP-IDF extension (not yet built)

The Arduino core only exposes SPP for classic BT. For A2DP audio and HFP AT
commands (where a covert mic usually hides), build against ESP-IDF directly:

```
git clone -b v5.4 --recursive https://github.com/espressif/esp-idf.git
./install.sh esp32 && . ./export.sh
# A2DP sink (accept the device's audio stream + log):
#   $IDF_PATH/examples/bluetooth/bluedroid/classic_bt/a2dp_sink
# HFP-AG (accept a hands-free call; capture AT commands + uplink audio):
#   $IDF_PATH/examples/bluetooth/bluedroid/classic_bt/hfp_ag
# HCI snoop (btmon-equivalent raw link capture) — enable Bluedroid log:
#   CONFIG_BT_BLUEDROID_LOG / esp_bt_controller VHCI dump to UART
```

HFP-AG is the highest-value add: a covert mic headphone behaves as a hands-free
with an uplink, and every `AT+...` command plus the uplink audio is logged.

## 7. Pitfalls (don't repeat these)

- A GSM implant is bursty — sweep for minutes, not seconds, and watch for
  periodic bursts. A nearby GSM TX can make audio gear buzz.
- A store-and-forward implant (records, transmits later) defeats a short sweep
  and a short ESP session — leave the ESP logging for hours.
- `usbmon`/SDR/`lsusb` don't work from PRoot/Android containers (no
  `/dev/bus/usb`/debugfs). This capture is UART/serial from the ESP, which is
  fine, but CH341A/SDR bench work needs a real Linux host.
- Bluetooth Classic discovery (inquiry) is not exposed by the Arduino core —
  the ESP can only see classic devices when *they* connect to *us* (or you use
  ESP-IDF's `esp_bt_gap` inquiry). That's fine for a sacrificial sink: put the
  device in pairing mode and let it find the ESP.
