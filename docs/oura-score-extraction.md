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

### The night's state machine: ported

The mapper's inputs (latency, wake-ups, restless epochs, motion events,
midpoint) are produced by a state machine over the night's 30-second epochs, in
the function that calls `ecore_sleep_score_calculate`. `src/analysis/oura_sleep_state.cpp`
is a port of it:

* inputs: one stage code per 30 s epoch (1 deep, 2 light, 3 REM, 4 awake, 0 awake),
  optionally one motion value per epoch, and the local start time of day
  (wrapped to -720..719 minutes). With fewer motion values than epochs the engine
  ignores motion altogether. The loop runs `count + 1` times, `count` capped at 1799.
* latency: start of the first run of ten non-awake epochs (1 if that is within
  the first ten epochs, 255 if it is after epoch 264);
* wake-ups: awake streaks of two or more epochs while asleep, minus the last;
* restless epochs: non-awake epochs where the motion summed over a four-epoch
  window is non-zero;
* motion events: a jump of 24 or more over `max(M[i-3], M[i-2])` where that
  reference is itself 24 or more, re-armed after `min(i+1, 20)` calm epochs;
* midpoint: the start time of day plus half the mean position of the non-awake
  epochs.

Motion per epoch is the sum of the `motion_event` (0x47) seconds (low five bits
of the first body byte) that fall in the epoch. **That mapping is an assumption**
about how the app feeds the engine: it is consistent with the field's range
(0-31 s per 30 s) but has not been checked against a real night, so the
restfulness contributor is the first thing to compare in *Score check* once a
ring night and the matching cloud score are both present.

Nights imported from Oura's cloud have staging (a 30 s string or the 5-minute
one, repeated ten times) but no motion, so their restful counts are unknown and
the score is renormalised and marked `complete = false`.

### How the state machine was checked

`tools/apk/ecore_sleep.py` runs `libecore.so` itself in the Unicorn emulator
(libc calls stubbed) and builds the `s_ecore_sleep_score_input_t` the way the
app does; `diff_sleep_state.py` feeds the same random nights to it and to
`tools/apk/diff_state_cli.cpp` (a thin driver around the port): 750 nights, no
mismatches. `diff_sleep_score.py` does the same for the mapper (3000 cases),
including the seven displayed contributor values (restfulness shown is
`sum(((2c+3)&0xff)*0xab>>10)` of its three parts).

```
python3 -m venv venv && venv/bin/pip install unicorn pyelftools capstone
g++ -std=c++17 -O2 -I src tools/apk/diff_state_cli.cpp src/analysis/oura_sleep_state.cpp -o /tmp/diff_state_cli
venv/bin/python tools/apk/diff_sleep_state.py lib/arm64-v8a/libecore.so 250 1   # cases, seed
```

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

### What reading `get_activity_score_raw` showed (why it is not ported yet)

Read in full (1,700 instructions, `0x64404`-`0x65ee0`), not ported:

* it is **not a pure function of its input struct**. Besides the day's
  `activity_score_input_t` it reads a nine-slot history of earlier days, 0x18
  bytes per slot, from one of two global stores chosen by
  `actinfo_get_stateless_mode()`, plus two feature flags
  (`ecore_is_feature_enabled(3)` and `(0x17)`) that switch whole branches
  (including a vectorised variant of the same counting). The third argument is
  the number of history days available (compared with 9 and 10);
* each day is five 16-bit minute counts plus a byte; an all-zero day is skipped,
  and most contributors are *counts of days* (of the last six, nine) that pass a
  threshold, turned into a class 0-3 and interpolated on the same
  `0, 25, 95, 100` curve as sleep (`0x3d220`) with breakpoints from
  `0x3d170`, `0x3d180`, `0x3d470`, `0x3d600`, `0x3d7e0`, `0x3d870`, `0x3d880`;
* the target thresholds come from `actinfo_get_default_met_values_for_categories`
  (age and sex), scaled by 10 and 60.

A faithful port therefore has to model that history store and its feature
flags, and the mapping from the ring's MET minutes to the five counts is not
visible in this function. The honest next step is to verify the sleep
restfulness against a real night first (it fixes how the app feeds the engine),
then do this with the same emulator oracle: seed the two globals, vary the input,
compare. Until then activity shows the ring-measured MET minutes and, for
imported days, Oura's own score.

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

## Checked against a real account (871 nights of one person, 2024-2026)

`tools/cloudcheck/` runs the app's own code over a `ring.db` holding a cloud import and
compares it with Oura's scores (`compare_pipeline.py`, `compare_sleep.py`). Findings:

* The scores in Oura's cloud use the library's **first** set of limits
  (`ecore_sleep_score_init_limits`: deep/REM top out at 150 min) and the minutes entry
  `ecore_sleep_score_calculate_minutes`, not the numerical path the first port covered.
  `OuraLimitSet::V1` and `ouraSleepScoreApp()` are that path (12,000 fuzz cases against the
  library: `tools/apk/diff_sleep_app.py`).
* Inputs: minutes are epochs/2 rounded half up; latency is `round(|minutes - 15|)`;
  efficiency is asleep over time in bed; the age steps on the birthday.
* Real nights: latency 871/871, total sleep 99 %, deep 99.5 %, REM 99.9 %, efficiency 96 %,
  timing 88 % (type 255 until the app learns a chronotype, then a centre; unknown here),
  restfulness not exact (third input is not recoverable from cloud data; an estimate).
* The state machine reproduces Oura's `restless_periods` on 871/871 nights and the API's
  latency on 871/871, from `sleep_phase_30_sec` and `movement_30_sec` (level >= 2 = motion).
* A day's score counts its other periods too: naps of >= 15 min asleep add to total, deep, REM.

### The ring's own motion source is `0x6b`

Oura's cloud `movement_30_sec` uses the levels 1-4 of the ring's `motion_period` event
(`0x6b`: 0 none, 1 restless, 2 tossing and turning, 3 active), and that string reproduces
Oura's `restless_periods` exactly through the state machine. So the restless count the app sends
the engine most likely comes from `0x6b` and not from the `0x47` seconds. `0x6b` is therefore
decoded (`decodeMotionPeriod`; layout from NOOP's notes, which corrected the ringverse reading:
a header byte with the period type, the number of codes in the last byte and a counter, then
2-bit codes, four to a byte) and used for a night's motion whenever the ring sent it; `0x47`
remains the fallback.

Not verified on a real Ring 4: the time axis. The event carries no start, so its codes are taken
to end at its timestamp, 30 s each. The restless count is little affected by a shift (it counts
epochs with movement, not where they are), but a capture compared with the cloud's
`movement_30_sec` of the same night would settle it.

## Activity score, fitted from real days (not the library)

`get_activity_score_raw` reads global history state, so it was not ported. Instead the 878 days of
one cloud import were used as ground truth (`tools/cloudcheck/activity_fit.py`). The score is
`25 meet_daily_targets + 10 move_every_hour + 25 recovery_time + 15 stay_active +
10 training_frequency + 15 training_volume`, over 100 (757 of 878 days exact from Oura's own
contributors; the rest are one point off from rounding). The contributors:

| contributor | rule | fit |
| --- | --- | --- |
| move_every_hour | table of the day's inactivity alerts: 100, 95, 78, 60, 43, 25, 1 | 100 % exact |
| meet_daily_targets | days of the last 7 (today included) with active calories at or over the target: 1, 25, 43, 60, 78, 95, 100, 100 | 99 % exact |
| training_frequency | days of the last 7 with medium + high activity of 150 MET-minutes or more: 1, 40, 71, 96, 100 | 99 % exact |
| stay_active | falling curve of sedentary hours | about +-3 of noise, 53 % exact |
| training_volume | rising curve of the week's medium + high MET-minutes | rough, 35 % exact |
| recovery_time | 100 on 81 % of days; what lowers it is not found | 81 % exact as "100" |

With these rules and the cloud's own daily figures as input the score is within 2 points on 81 % of
days (mean error 1.2). What is not done: producing those daily figures (inactivity alerts, the
sedentary, medium and high minutes, the MET-minutes) from a ring's own data. The cloud's per-minute
MET series reproduces the high minutes (74 % exact) but not the medium ones, so the ring-side
classification is something else, probably the activity class the ring writes with its `0x50`
events. That mapping, the user's calorie target and the missing last contributors are what stands
between this and an activity score in the app.
