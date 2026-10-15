# What the Oura Android app (7.27.0) says

Source: `Oura_7.27.0_APKPure.xapk` (package `com.ouraring.oura`, version code
261007085, min SDK 32). The bundle is a base APK plus splits for the ABI,
resources, on-device models (`oura_models.apk`) and ring firmware
(`ring_firmware.apk`).

The interesting code is Oura's own BLE library, `com.ouraring.ourakit` (305
classes, all in `classes9.dex` of the base APK), which is not obfuscated. The
rest of the app is. Bytecode was read with androguard; no decompiler was needed.

## Why this was done

open_oura's protocol notes were written against 7.15.0. This app is 7.27.0, and
authentication is the one place where a silent change would make every sync fail
with an unhelpful error. So the operations the app depends on were re-read.

## Confirmed unchanged in 7.27.0

| thing | where | what the bytecode builds |
|---|---|---|
| GATT service and characteristics | `internal/Constants` | `98ED0001-…` service, `98ED0002` write, `98ED0003` notify; charger service `8BC5888F-…` |
| nonce request | `operations/GetAuthNonce` | `2f 01 2b` |
| authenticate | `operations/Authenticate` | `2f 11 2d <16 bytes>`; reply must be tag `0x2f`, ext `0x2e`, verdict at byte 3 |
| set key | `operations/SetAuthKey` | `24 10 <16 bytes>`; reply tag `0x25`, `0x00` ok, `0x05` special-cased |
| time sync | `operations/SyncTime` | `12 09 <u64 LE seconds> <tz = offset ms / 1 800 000>` (length 10 with a force flag); reply tag `0x13` |
| extended history | `operations/ExtGetEvent` | `2f … 41 <buffer> <start ms> <max events>`; `0x42` confirmation (events u16, sleep progress u8, bytes left u32, buffer, result); `0x43` length-prefixed envelopes; 120 s timeout |

The authentication cipher (AES-128/ECB/PKCS5 over the 15-byte nonce) is
unchanged from the notes.

## Worth knowing

* **Time sync is a u64 of seconds plus a timezone in half hours.** open_oura's
  `sync_time_app` sends a variant with a "counter" and a fixed `f6` timezone
  byte, copied from a capture. It reads as the same 9-byte payload with the low
  byte of the seconds replaced by a token. This app sends the form the 7.27.0
  code builds.
* **The auth key is generated on the phone** (a random UUID written as two
  little-endian longs) and installed with `SetAuthKey`. Nothing about it comes
  from Oura's servers, which is what makes a cloud-free client possible.
* **A ring holds one key.** Whoever installs it last owns the ring. A ring set up
  by the Oura app can only be read by a client that has that app's key (which is
  in the app's Realm database; see open_oura's `android-app-reversing.md`) or
  after a factory reset.
* The app has an `r_data_autosync` preference that is off by default; the bulk
  raw-sensor channel (`0x03`) is not used by a normal sync and not by this app.

## What was not examined

The scoring engine (`libappecore.so`), the encrypted PyTorch models and the
cloud API. They are the reason Oura's own Sleep, Readiness and Activity scores,
stress and step counts cannot be read from a ring directly, and reproducing them
is out of scope. `src/analysis/sleepscore.cpp` is an independent, published-norms
estimate instead.
