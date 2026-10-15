# Oura Sync (harbour-ourasync)

A Sailfish OS app that reads an **Oura ring (Gen 3, 4, 5) directly over
Bluetooth LE** and shows what it recorded: sleep stages and a sleep score, heart
rate through the day, HRV, skin temperature, blood oxygen, breathing and
activity. No Oura account, no subscription, no network permission: the history
stays on the phone, and the ring's key in the Sailfish Secrets vault.

Native C++/QML (Silica). Unofficial: not made, endorsed or supported by Oura
Health, and not a medical device.

## How it came about

* The ring's protocol was reverse engineered by
  [open_oura](https://github.com/Th0rgal/open_oura); the sleep-score method and
  several decoders come from [open_health](https://github.com/Th0rgal/open_health)
  (both MIT, Thomas Marchand). The views follow
  [Cracked Oura](https://github.com/EIrno/Cracked-Oura).
* The authentication, time-sync and history-fetch layouts were re-read in the
  bytecode of Oura's Android app **7.27.0** and found unchanged:
  [`docs/apk-findings.md`](docs/apk-findings.md).

## Read this before you set a ring up

A ring has **one owner**. This app sets up a *factory-reset* ring and installs a
key of its own, after which the Oura app cannot use that ring until it is reset
again. The key lives only in this phone's vault: the app offers a backup, and
you should take it, because losing the key means a factory reset, which erases
the history stored on the ring. Details: [`docs/design.md`](docs/design.md).

## What you get and what you do not

Measured by the ring and shown as such: sleep window and stages, heart rate,
HRV, skin temperature, SpO2, breathing rate while asleep, movement (MET per
minute). Estimated by the app and labelled: the sleep score, temperature change
against your own usual, active calories. **Not available:** Oura's own Sleep,
Readiness and Activity scores and step counts. Oura computes those on the phone
with closed software; a ring read directly does not carry them.

## Build

```
~/SailfishOS/bin/sfdk -c target=SailfishOS-5.1.0.11-aarch64 build
```

Or open `CMakeLists.txt` in Qt Creator with the Sailfish SDK. The RPM lands in
`RPMS/`. Install with `devel-su pkcon install-local`.

## Test

```
tests/run.sh          # protocol, session, analysis: plain g++, no phone
python3 tools/check_qml.py
```

See [`tests/README.md`](tests/README.md). The pages were also rendered offscreen
against a stub of Silica for layout; the first sync on a real ring is the part no
test here replaces, and [`docs/design.md`](docs/design.md) lists exactly what it
would confirm.

## Translations

Source strings are English. After changing any `qsTr()`:

```
lupdate -locations absolute -no-obsolete src qml -ts translations/harbour-ourasync.ts
python3 tools/fill_fi.py      # reports any string with no Finnish text
```

## Licence

MIT. Third-party sources are credited in the About page and `docs/`.
