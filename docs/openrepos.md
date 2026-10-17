# OpenRepos listing

**Title:** Oura Sync

**Short description (one line):** Read your Oura ring over Bluetooth, without an account or subscription, and keep the data on your phone.

**Categories:** Health & Fitness, Utilities
**Licence:** MIT
**Source / bug reports:** https://github.com/jarnose/harbour-ourasync
**Needs:** Sailfish OS 5.1 or newer (aarch64 and armv7hl builds)

## Description

Oura Sync reads an Oura ring (Gen 3 and 4 tested, Gen 5 expected to work) straight from the ring over Bluetooth LE and shows
what it has recorded: nights of sleep, heart rate, HRV, skin temperature, blood oxygen and
activity. There is no Oura account to sign in to and no subscription; the data stays on your
phone and nothing is sent anywhere. The app is unofficial and not affiliated with Oura.

**What you get**

* Nights with a hypnogram, sleep stages, resting heart rate, HRV, skin temperature and SpO2,
  days with heart rate by the hour and activity, and trends over weeks.
* A sleep score worked out the way Oura's own app does it. The arithmetic was read from the
  Android app and checked against 871 real nights from Oura's cloud: total sleep, deep sleep,
  REM, efficiency and latency match, restfulness and (before Oura has learnt your chronotype)
  timing are estimates. A Score check page compares the app's numbers with Oura's.
* Ring 4 and 5 do not stage sleep themselves; their nights get stages estimated from heart rate,
  HRV and movement. Awake and asleep are reliable, deep sleep and REM are rough, and the app says so.
  Their breathing rate is worked out from the beat-to-beat intervals the way Oura's app does.
* Activity and readiness scores are estimated for days Oura's cloud has none for (fitted to one
  person's history, shown with a "~").
* Your history from before: import Oura's data export (a ZIP or CSV files, no account needed), or,
  if you have a membership, pull it from the Oura Cloud API with an application of your own
  (OAuth2; sign-in kept in the Sailfish Secrets vault). Oura's own scores are shown beside the
  app's, never mixed with them.
* Export everything as a FHIR R4 bundle or CSV, back up and restore the whole database, and
  factory reset the ring from the app (for moving it to another phone or the Oura app).
* English and Finnish.

**Read this before setting a ring up**

A ring has one owner. To use it here it must be factory reset first; the app then installs a key
of its own, and the Oura app can no longer use the ring until it is reset again (the Oura app syncs
to Oura's cloud, this app does not). The key lives in the Sailfish Secrets vault and the app offers
a backup of it: take it, because losing the key means another factory reset, which erases what the
ring holds. Rings 3 and 4 have been tried here; Ring 5 has not, though it speaks the same protocol. The notes
in the repository say what has and has not been verified on real hardware.

**What it does not do**

Oura's own activity and readiness scores, steps and stress are produced by Oura's closed
software and are shown only for days imported from Oura's cloud; the app's activity and readiness
numbers are estimates, and stress is not estimated. This is not a medical device.

**Permissions**

Bluetooth (the ring), Secrets (the ring's key and the cloud sign-in), Internet (only when you
import from the Oura Cloud API; reading the ring needs no network), Downloads and Documents
(importing an export and writing exports and backups).

**Credits**

The ring's protocol was reverse engineered by the open_oura project and the sleep-score method
and several decoders come from open_health (both by Thomas Marchand, MIT); the dashboards follow
the ideas of Cracked Oura.
