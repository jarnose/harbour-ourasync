# Design, and what only a ring can confirm

## Layers

```
 QML pages ── AppController ──┬─ RingLink (BlueZ D-Bus GATT) ── Transport ─┐
                              ├─ RingStore (SQLite: raw events + cursor)    │
                              ├─ KeyVault (Sailfish Secrets)                ▼
                              └─ summarize()  ◄── RingClock ◄── RingSession (protocol, Qt-free)
                                   (Qt-free)
```

* `src/ring/`: AES, framing, request builders, event decoders and the session
  logic (authenticate, set up, drain). **Qt-free**, tested with g++.
* `src/analysis/`: ring clock, nightly and daily summaries, sleep score.
  **Qt-free**, tested with g++.
* `src/ble/`: BlueZ over D-Bus. Qt Bluetooth does not exist on Sailfish 5.1.
* `src/store/`: events are kept **raw and lossless**; decoding happens when a
  summary is built, so a decoder fixed later improves history already stored.
  Events and the sync cursor commit in one transaction, so an interrupted sync
  resumes instead of losing or repeating a stretch.
* `src/secrets/`: the 16-byte key lives in the Secrets vault, never in the
  database. A new key is stored *before* the ring is told about it.

The conversation with the ring is straight-line code that blocks inside nested
event loops (`RingLink::waitFor`, `transact`) rather than a callback state
machine. `AppController` refuses a second operation while one runs.

## One owner

A ring holds one app key. Setting up here means factory-resetting the ring and
installing a key of our own, after which the Oura app cannot use the ring until
it is reset again. The alternative, importing the Oura app's key from a rooted
Android phone, is supported (the "my ring already has a key" switch), but a ring
that is already provisioned may refuse to bond with a new phone.

## What is measured and what is estimated

Measured by the ring: sleep window and stages, HR, HRV, skin temperature, SpO2,
breathing rate, MET per minute. Estimated here: the sleep score (published
norms; `sleepscore.cpp`), temperature deviation (own 14-night mean), active
kcal, hourly HR bands. Not available: Oura's scores and steps.

## Assumptions that only hardware can confirm

These are the places where the reverse-engineering notes leave room, and where a
first sync on a real ring may disagree. Each is isolated so it is a small fix.

1. **Hypnogram placement** (`daysummary.cpp`, "Where does the run begin?").
   `sleep_phase_data` pages carry only the moment the ring emitted them. A run
   one page longer than the bedtime window is anchored at the window's start
   (the last page's padding falls off the end); any other run ends at its
   emission time, as open_health does. If the stage blocks look shifted against
   the heart-rate curve, this is why.
2. **Activity bins** are taken as one MET per minute starting at the event's
   timestamp. open_health does the same; nothing independent confirms it.
3. **Pairing** through `Device1.Pair()` needs BlueZ to complete a Just Works
   bond without a user prompt. If Sailfish's agent insists on one, pair in
   Settings → Bluetooth first; the set-up then skips pairing.
4. **Ring 3/4/5 differences**: stream registration runs on Ring 5 only; the
   legacy fetch is the fallback elsewhere. Only Gen 3 Horizon and Ring 5 were
   observed upstream.
5. **Clock**: with a `time_sync` anchor per sync the dates are exact; an epoch
   with none is placed from the download time and flagged "approximate".

`Settings → Diagnostics` lists stored events by type, which separates "a decoder
is wrong" from "the ring never sent it".

## Not done

* Background or automatic sync (the app syncs when asked).
* Live heart rate (needs `SetFeatureMode CONNECTED_LIVE`; designed, unwired).
* Toggling ring features (real steps, exercise HR). The session API has
  `setFeatureMode`; there is no UI.
* Firmware update, factory reset from the app, and the raw RData channel: all
  deliberately left out. They can brick a ring or erase its history.
