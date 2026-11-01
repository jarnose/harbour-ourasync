# Changelog

## 1.8.1

- The day page shows the day's activity goal under the active calories (Oura's own where the cloud has it,
  else the basic goal stepped by the day's readiness, marked "~").

## 1.8.0

- The activity estimate measures each day against Oura's own kind of goal: the basic goal from Settings
  raised or lowered by the day's readiness (up to +200 kcal at a readiness of 94, down to -300 under 40),
  which is how Oura sets it. On 872 real days the rule gave Oura's goal on 93 % of them, and the
  "meet daily targets" part of the activity score came out as Oura's on 96 % of days, against 51 % with
  one fixed goal. Days from the cloud keep Oura's own goal.
- The basic goal in Settings now starts at 450 kcal.

## 1.7.0

- The main page shows three gauges for last night and the day: sleep, readiness and activity,
  Oura's own where the cloud has them and the app's estimate (marked "~") where not.
- Settings has a Cover section: choose what the cover shows from the sleep, readiness and activity
  scores, time asleep, the latest and the night's lowest heart rate, HRV and the ring's battery.

## 1.6.0

- Sleep debt for each day with sleep, as Oura's app shows it: the last 14 days' shortfall against your sleep
  need, newest days counting most (Oura's own arithmetic, read from its library), with the need estimated
  from your last 90 days. Shown with a "~" on the day page, with Oura's none / low / moderate / high.

## 1.5.0

- Estimated time in high stress for each day, from the day's heart rate while awake and still, your own
  usual over the 14 days before and the night's HRV. Fitted to 868 real days (mean error 31 minutes out
  of sample, correlation 0.79); shown with a "~" on the day page, and Oura's own figure where the cloud
  has one. Recovery time is not estimated. Needs about four days of the ring's data.

## 1.4.2

- Heart rate from Oura's cloud: the times asked for are sent with a "Z" instead of "+00:00", whose
  plus sign reached Oura as a space and made it answer HTTP 422. A failed heart rate request now
  also shows what Oura said.

## 1.4.1

- The Oura cloud page says how many heart rate samples are held, and an import tells when Oura
  refused or failed the heart rate series (it used to skip it without a word).
- The scan, alert and connection events of the ring have names in the diagnostics.

## 1.4.0

- Breathing rate for rings that do not send it (Ring 4 and 5): worked out from the night's
  beat-to-beat intervals the way Oura's app does, and shown with the night's other numbers. On a
  real night it lands within 0.11 breaths a minute of Oura's own library on average (0.33 at worst).

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
