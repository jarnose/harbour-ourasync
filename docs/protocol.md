# The ring protocol, as this app uses it

Provenance: [open_oura](https://github.com/Th0rgal/open_oura)'s reverse
engineering (tested live on a Ring 3 Horizon and a Ring 5), with the
authentication, time-sync and extended-fetch layouts re-verified in the Android
app 7.27.0 (`apk-findings.md`). Code: `src/ring/`.

## Link

* Service `98ed0001-a541-11e4-b6a0-0002a5d5c51b`; write characteristic
  `…0002…`, notify characteristic `…0003…`. MTU about 203.
* A factory-reset ring requires a **bonded, encrypted** link before it accepts a
  single write. The app pairs (`Device1.Pair`) and marks the device trusted.
* The ring serves one connection. The app disconnects when it is done.

## Frames

`tag | length | payload`, little-endian. Several frames can share one
notification, so the parser walks all of them. Extended operations use tag
`0x2f` with the first payload byte as the extended operation.

## Session

1. Connect, resolve services, `StartNotify` (and `StopNotify` on close: BlueZ
   sends a notification once per subscribed client, so a missed `StopNotify`
   doubles every later frame).
2. `2f 01 2b` → `2f 10 2c <15-byte nonce>`; encrypt with the 16-byte key
   (AES-128/ECB, PKCS#7 to one block); `2f 11 2d <16>` → `2f 02 2e <result>`.
   0 = ok, 1 = wrong key, 2 = ring is in factory-reset state, 3 = onboarded by
   another device.
3. Ring 5 only: stream registration and parameter sweep the official app does.
4. `12 09 <u64 seconds> <tz half-hours>`: sets the clock. The ring logs a
   `time_sync` event, which is what dates its history.
5. `28 01 00` (sleep-analysis check) and battery `0c 00`.
6. Drain: `28 01 00` flush, then `2f 0c 41 00 <start ms> <max u16>` until the
   `0x42` confirmation reports no bytes left. Falls back to legacy `10 09` if
   the ring answers that the extended request is unsupported.

## History events

Each is `tag | len | ts u32 | body`. The timestamp counts **deciseconds on the
ring's own counter**, which restarts after some resets and stalls while the ring
is off; `src/analysis/ringclock` ties it to UTC with the `time_sync` (0x42) and
`rtc_beacon` (0x85) events.

Extended fetches deliver bundles: `control | tag | len | varint ts | body`, the
first timestamp absolute milliseconds and the rest deltas.

Decoded and used (`src/ring/events.cpp`):

| tag | event | body |
|---|---|---|
| 0x42 / 0x85 | time_sync / rtc_beacon | u32 unix seconds |
| 0x5d | hrv_event | (hr bpm, rmssd ms) per 5 min |
| 0x80 | green_ibi_quality | 2 bytes per beat: ibi = (b1&7)\|(b0<<3), quality = (b1>>3)&3 |
| 0x6a | sleep_period_information_2 | per window: hr×0.5, breath/8, motion, sleep state |
| 0x76 | bedtime_period | start, end (ring ds) |
| 0x5a | sleep_phase_data | page no., then 2-bit phases (deep, light, REM, awake), 52 per page |
| 0x46 / 0x69 / 0x75 | temperatures | int16 centi-°C |
| 0x6f | spo2_event | header, % per sample |
| 0x8b | spo2_r_pi_event | R ratio (u16 BE / 16384) and perfusion index |
| 0x50 | activity_information | state, then MET per minute (b<128: ×0.1, else 12.8+(b−128)×0.2) |
| 0x61 | debug_data | subtype 0x24: battery % and mV |

What the ring does **not** send: Oura's 0–100 scores, readiness, stress,
resilience, cardiovascular age and step counts. Oura computes those on the phone.
