# Contributing to the ESP32 Bluetooth Sacrificial Host

Thanks for your interest — this is a defensive-research tool, and every solid
contribution helps. Read this first so your PR lands cleanly.

## What to work on

High-value, well-scoped starter items:

- **ESP-IDF A2DP-sink + HFP-AG extension** — the single highest-value add. The
  Arduino core exposes only SPP for classic BT; Bluedroid classic profiles need
  ESP-IDF directly. See [`docs/pacify-playbook.md`](docs/pacify-playbook.md) §6.
  Mark any API symbol you can't confirm with `VERIFY against IDF v5.4` rather
  than guessing.
- **Honeypot baiting** — advertise a victim-like name/UUID and add a minimal reply
  state machine so a device keeps talking instead of disconnecting after one
  exchange.
- **Bonding / encryption** — the central probe can't currently read encrypted GATT;
  add `secureConnection()` + client callbacks.
- **More unit tests** for [`lib/honeypot_core`](lib/honeypot_core).

## Ground rules

- **Defensive & evidence-preserving only.** This repo analyzes devices you own or
  are authorized to examine. No reflashing to exfiltrate to your own C2, no
  extending found malware into exploit chains. PRs that cross into that are out
  of scope.
- **No fabricated claims.** Don't claim a build "passes" unless you ran it. Don't
  invent an API that doesn't exist — mark it `VERIFY` and say so.
- **Keep it honest.** The README explicitly lists known gaps. Don't strip them to
  make the tool look more finished than it is.

## Before you open a PR

1. Fetch the latest `main` and branch from it.
2. Make a focused change — one PR, one concern.
3. Run the checks:
   ```sh
   # unit tests (no board needed)
   g++ -std=c++17 -I lib/honeypot_core/src tests/test_core.cpp -o /tmp/test_core && /tmp/test_core

   # build both firmware targets
   pio run -e s3-ble-logger
   pio run -e esp32-bt-sink
   ```
4. Confirm both builds succeed and the tests pass, then reference that state in your
   PR description.

## Style

- Plain, honest code with comments explaining *why*, not just *what*.
- No new license headers on existing files; add one only if you add a new file.
- Keep the serial log simple and machine-parseable.

Open your PR against `main`. Thanks for building this with us.