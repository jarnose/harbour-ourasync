# Importing history from Oura's cloud

A ring keeps a few weeks of events. Oura's cloud keeps everything the official
app ever uploaded, together with the scores the ring itself cannot report (sleep,
readiness, activity: Oura computes those on the phone). This app can read both of
the ways Oura offers.

| | data export | Cloud API |
|---|---|---|
| needs | an Oura account | an account **and** an active Oura Membership |
| setup | none | register your own application at cloud.ouraring.com/oauth/applications |
| network | none in the app (you download the ZIP in a browser) | the app talks to api.ouraring.com |
| history | all of it | the span you choose |
| permission | Downloads / Documents | Internet |

## The data export (recommended)

membership.ouraring.com/data-export lets any owner request a ZIP of their data
(sign in with the emailed code, request, wait for the email). Save it on the phone
and open it in *Oura cloud* in Settings. The reader accepts the ZIP, a folder of
the unpacked CSV files, or a single CSV.

The files are semicolon-separated and not quite regular, which Cracked-Oura's
ingestion documents and `src/cloud/csv.cpp` follows: a whole line may be wrapped in
quotes with doubled quotes inside; JSON cells (contributors, heart-rate series)
contain commas and quotes; and in `dailyactivity.csv` and `sleepmodel.csv` a row
may hold fewer cells than the header, the missing ones being the *leading*
columns. Files used: `dailysleep`, `dailyreadiness`, `dailyactivity`,
`sleepmodel` (the sleep periods with five-minute stages, heart rate and HRV),
`dailyspo2`, `dailystress`, `dailyresilience`, `dailycardiovascularage`,
`heartrate`, `workout`, `session`, `enhancedtag`, `sleeptime`.

## The Cloud API

Oura retired personal access tokens at the end of 2025. Every client now uses
OAuth 2.0 with an application it registers itself (a client id, a client secret and
a redirect URI), and Gen 3 and later rings need an active Membership for the API to
serve their data. So the sign-in here is:

1. Register an application; set its redirect URI to `http://localhost:8421/callback`
   (the port is editable).
2. Enter the client id and secret in the app and tap *Sign in with Oura*. The
   phone's browser opens Oura's consent page.
3. Oura redirects to `localhost`, where a small listener in the app catches the
   code (and checks the `state`). If the browser cannot reach it, paste the address
   from the browser's address bar into the app instead.
4. The tokens go into the Sailfish Secrets vault at once: a refresh token is
   single use, so the new pair is stored before anything else happens.

Scopes asked: `personal daily heartrate workout session tag spo2Daily`. A
collection that answers 403 (no membership, or its scope was declined) is skipped
with a note; the rest are still imported. Heart rate is fetched a week at a time.

## What it changes in the app

* Oura's score and contributors are shown beside this app's estimate for each
  night, and Oura's readiness and activity scores and steps appear on the overview
  and in the day view.
* Nights and days **before the first sync** are added from the cloud (stages in
  five-minute steps, heart rate, HRV), marked as coming from the cloud. Where the
  ring has the same night, the ring's data is kept.
* *Score check* sets this app's arithmetic against Oura's numbers night by night.
  With an export it is also the validation set for the Oura-formula port in
  `docs/oura-score-extraction.md`: the official per-contributor values are in the
  export, so a mismatch is a measurable error.

## Privacy

The export route sends nothing anywhere. The API route only fetches; nothing is
uploaded. The `Internet` permission is in the app because Sailjail permissions are
static, not because the app uses the network otherwise. The client secret and the
tokens live in the Secrets vault, not in the database.
