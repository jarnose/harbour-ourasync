# Changelog

## 1.1.0

- Ring 4 and 5 do not stage sleep themselves: their nights now get stages estimated from heart
  rate, HRV and movement (a port of open_health's model-free stager), labelled as estimates.
  Awake and asleep are reliable; deep sleep and REM are rough.

## 1.0.2

- The ring's own motion states (`0x6b`, the source of Oura's restless periods) are decoded and
  used for a night's restlessness; the `0x47` motion events remain the fallback.

## 1.0.1

- Factory reset: its progress and result are shown on the Settings page, not only as a banner on
  the main page; the ring dropping the link as it erases itself counts as success.

## 1.0.0

First release.

- Sleep score computed the way Oura's own app does it (first limit set, minutes entry), checked
  against 871 real nights from the cloud and 12,000 fuzz cases against the library: total sleep,
  deep, REM, efficiency and latency match; restfulness is an estimate; timing follows the
  chronotype once Oura has learnt one.
- Oura cloud import: data export (ZIP/CSV) and the Cloud API with OAuth2 (PKCE); age and weight
  from Oura's profile; the sign-in is revoked on disconnect.
- Birthday in Settings instead of an age; the score's limits follow it night by night.
- Factory reset of the ring from Settings, with the charger procedure for a lost key.
- Export as a FHIR R4 Bundle and CSV; backup and restore of the whole database.
- Charts and gauges repaint when a page is shown again; BlueZ calls as plain D-Bus messages;
  numbers in JSON are read with the classic locale (a Finnish phone broke Oura's token reply).
- Finnish translation.

## 0.1.0

- First version: reads an Oura ring over BLE (BlueZ), stores its history and shows nights, days,
  heart rate and trends.
