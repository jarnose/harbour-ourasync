# Oura's score algorithms, read from the app

Question: Oura's 0-100 Sleep, Readiness and Activity scores are not sent by the
ring (they are computed on the phone). Can the algorithm be recovered from the
Android app, so that this app can compute them from the ring's own data?

Short answer: **partly, and further than open_oura got.** The sleep score's
mapper is recovered exactly; the activity combiner is recovered exactly; the
inputs that need the app's per-epoch state machines, and all of readiness, are
not yet.

## What open_oura had concluded (docs/algorithms)

`open_oura/docs/algorithms/` ports what it could from `libappecore.so` of app
7.15 and, for the three scores, stops at the **constant tables**: "the tables do
not read back cleanly from this APK's `libappecore.so` build ... so an exact
bit-match isn't feasible". It then fits the weights by regression against an
Oura account's trends export instead (Sleep R² 0.999 on the combiner, 0.964
end to end; Readiness 0.49 and Activity 0.06 from single-day inputs).

## What 7.27.0 offers

The app bundle (`config.arm64_v8a.apk`) carries, among others:

| library | size | note |
|---|---|---|
| `libecore.so` | 1.1 MB | the C++ `nexus::ecore` engine. **1660 exported symbols with their names**: `ecore_sleep_score_calculate`, `ecore_sleep_score_init_limits_v2`, `readiness_calculate`, `get_activity_score_raw`, `get_activity_score_from_raw_100`, `actinfo_calculate_scores`, `sleep_timing_score_calculate`, `recovery_run`, ... |
| `libappecore.so` | 2.1 MB | the older C-style ecore (same functions, no namespace), self-contained |
| `libnexusengine.so` | 16 MB | the engine that drives it |

The Java side confirms what the engine consumes: `EcoreWrapper.nativeCalculateSleepScore`
takes the **raw ring event bytes** (`BedtimePeriod`, `SleepPhasesV2`, `MotionSeconds`,
`SkinTemperatures`, `HrHrv5MinAverages`) plus a `ChronotypeInput`. So the sleep
score is a function of the events this app already stores.

### Why the tables looked unreadable

They are not missing; they are computed. The age-dependent limits are written at
**run time** into a struct in `.data` by `ecore_sleep_score_init_limits_v2`, so a
dump of the file shows zeros. That function is 60 instructions of integer and
single-precision arithmetic and is reproduced exactly in
`src/analysis/oura_sleep_score.cpp`. The static breakpoints sit in an initialised
`.data` block and the response curve and weights in `.rodata`, all readable.

## Sleep score: recovered

Nine contributors, each mapped by the same piecewise-linear curve and combined as
`(sum(weight x contributor) + 50) / 100`.

| # | input | breakpoints (-> 0, 25, 95, 100) | weight |
|---|---|---|---|
| 0 | total sleep, minutes | `0, (128n+128)>>8, (302n+128)>>8, 540` with `n = 486 - (453 x age >> 8)`; the last is `(5n+2)>>2` under age 31 | 35 |
| 1 | deep sleep, minutes | `0, t/4, t, 1.25t` with `t = max(round(115.64 - 1.1334 x age), 20)` | 10 |
| 2 | REM sleep, minutes | `0, r/4, r, 1.25r` with `r = max(114 - (341 x age >> 9), 60)` | 10 |
| 3 | efficiency, percent | `30, 60, 90, 95` | 10 |
| 4 | latency: distance from 15 minutes | `60, 28, 2, 0` (descending) | 10 |
| 5, 6, 7 | restfulness: three motion-derived counts | `240,128,16,0` / `80,50,25,15` / `96,48,16,0` | 5 + 5 + 5 |
| 8 | timing: sleep midpoint | trapezoid, below | 10 |

Input definitions as the engine counts them (30-second epochs): total = (deep + 1
+ light + REM) / 2 minutes; efficiency = total x 100 / (in-bed epochs / 2);
latency = (epochs to first sleep + 1) / 2 minutes. The restfulness inputs are
`count x 6720 / T`, `count x 50 / T` and `count x 6720 / T` with `T = max(total, 60)`.

**Timing** is a trapezoid around an ideal midpoint (03:00 by default, the
chronotype's own centre when it is known): 100 within 45 minutes of it (30 with a
chronotype), 25 at 150 minutes beyond that, 0 at 280. This matches open_oura's
empirical finding of a timing curve peaking near 03:00.

The four weights it recovered by regression (35 / 15 / 10 / 10 / 10 / 10 / 10)
are exactly these once the three 5-point restfulness inputs are added.

### How it was checked

* `test_oura_sleep_score.cpp` against the one published night in
  `score-weights.md`: 471 minutes asleep at 91 % efficiency gave **Efficiency 96**
  (reproduced for every age) and **Total 83** (reproduced at age 24-25; the
  author's age is not published, and is plausible).
* Sleep weights 35/10/10/10/10/15/10 agree with open_oura's independent
  regression to within its stated error.
* **Not validated**: the timing trapezoid (read from the binary, no published
  night exercises it) and the deep/REM rows (no published minutes to test them).

### What is still missing for an exact sleep score

The three restfulness inputs and the exact midpoint are produced by a state
machine over the night's epochs (about 2.3 kB of arm64 at `0x7e7dc`, inside the
function that calls `ecore_sleep_score_calculate`). From a first read: it
tracks a per-epoch motion value, counts "restless" epochs in non-awake stages,
counts long awakenings, and takes the midpoint as the mean position of the sleep
epochs added to the start time of day. Porting it needs the per-epoch motion
signal (`motion_event`, 0x47) laid out exactly as the engine reads it. Until
then `ouraSleepScore()` leaves those contributors out and renormalises, and says
so (`complete = false`).

## Activity score: combiner recovered, inputs not

`get_activity_score_from_raw_100` is fully readable: seven raw contributors in
[0, 1], weights `25, 15, 10, 10, 15, 10, 15` (sum 100), score = round(sum). The
curves that produce the seven contributors are in `get_activity_score_raw` (6.9
kB) and read float tables at `.rodata` `0x3d170`, `0x3d220`, `0x3d600`, `0x3d7e0`
plus the app's activity history state. **Not ported.** The contributors that
open_oura could not reproduce from a single day (Meet Daily Targets, Training
Volume/Frequency) depend on a personalised target (`actinfo_get_activity_target`,
from age and sex) and on days of history; with the engine readable, both are
reachable, and the ring's per-minute MET is the engine's own input.

## Readiness: not analysed

`readiness_calculate` (1.6 kB) and `recovery_run` read baselines (HRV, resting
HR, temperature; open_oura ported `baseline_update_*` exactly) and the previous
night's and day's outputs. Only its structure was looked at. It is the largest
piece and needs ~2 weeks of accumulated history to mean anything.

## Reproducing this

```
unzip Oura_*.xapk config.arm64_v8a.apk
unzip config.arm64_v8a.apk 'lib/arm64-v8a/libecore.so'
pip install pyelftools capstone
python3 tools/apk/a64dis.py lib/arm64-v8a/libecore.so ecore_sleep_score_init_limits_v2
python3 tools/apk/a64dis.py lib/arm64-v8a/libecore.so @0x7fc0c:704   # the mapper
```

The mapper is the unexported function at `0x7fc0c` (called from
`ecore_sleep_score_calculate` at `0x7f9c8`); the weights are the 9 bytes at
`.rodata 0x3db35` and the curve is 4 signed bytes at `0x3db2d`.

## Status of this as a source of truth, and licensing

* This is a **clean reading of arithmetic**, not a copy: the repository contains
  constants and formulas written out in the app's own code and style, no binary,
  no disassembly of the library, and nothing decrypted. The library itself is
  Oura's and is not redistributed; `tools/apk/` only reads a copy you supply.
* It is reverse engineering of a proprietary program for interoperability with
  hardware the user owns. open_oura publishes similar ports under MIT. Whether
  that is acceptable for something you distribute is for you to decide; the
  disassembler and the notes are separate from the `src/` code that uses the
  numbers, so the choice can be made independently.
* Oura changes its algorithms: the app records an `algorithm_version` with each
  score. A tuned-to-7.27.0 formula can drift from what a newer app shows.
