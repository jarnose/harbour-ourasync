# Changelog

## 1.3.1

- HRV and heart-rate buckets of the ring (`0x5d`) are placed correctly in time: the first pair is
  the oldest five minutes and the event's time ends the last one. Before, they were laid forwards
  from the event, up to half an hour off (correlation with the beats' own heart rate 0.16, now 0.90).
  This moves the night's heart-rate and HRV curves, resting heart rate and the estimates built on them.

## 1.3.0

- An estimated readiness score, with its nine contributors, for days Oura's cloud has none for,
  from the nights and days the app holds. Fitted to 859 real days (mean error 2.4 points, 70 % of
  days within 2, 93 % within 5, for the person it was fitted to); shown with a "~" on the day page.

## 1.2.0

- An estimated activity score for every day the ring has the six before it, with its contributors,
  worked out from the ring's movement and a daily calorie goal set in Settings. Fitted to 878 real
  days (mean error 2 points; recovery time is taken as full); labelled as an estimate with a "~".
- Active calories now count minutes of 2 MET and over, which matches Oura's much better.

## 1.1.2

- The main page's "Synced … ago" updates while the page is open instead of only at start-up.

## 1.1.1

- Blood oxygen (and daytime heart rate) are switched on in the ring at sync if they are off,
  as the Oura app does at set-up; a freshly reset Ring 4 recorded no SpO2 until then.
- The main page shows Oura's scores from the last cloud import with their date, and hides
  them once they are more than three days old.

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
