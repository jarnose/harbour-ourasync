#!/usr/bin/env python3
"""Fits an estimate of Oura's daily stress minutes from the day's heart rate (a ring.db holding a
cloud import WITH heart rate: Oura cloud page, "everything").

    python3 stress_fit.py ring.db [--emit days.tsv]

What the cloud holds, and what was found (872 days, 2024-05 to 2026-10, one person):

  daily_stress.stress_high    seconds in "high stress", always a multiple of 300: a count of five-minute
                              slots; 19 slots (95 minutes) a day on average, 96 at most. In 99 % of days
                              it is no more than the slots with a heart rate while awake and still, so
                              Oura classifies the same slots as the cloud's heart rate holds.
  daily_stress.day_summary    "stressful" when the day has about 36 or more such slots (3 hours): the
                              threshold classes 824 of 861 days correctly.
  recovery_high               poorly explained by heart rate (test correlation 0.5); not estimated.

Oura's own model (stress_daytime_sensing, per open_oura) works on the daytime HRV of those slots; the ring
does not send that for the day, so this is a heart-rate estimate: the slots' heart rate against the person's
own median over the 14 days before, counted at several margins, plus the night's HRV against its last 30,
combined linearly (ridge). Out of sample (fitted on the first 70 % of days, scored on the last 30 %): mean
error 6.2 slots (31 minutes) against 9.6 for guessing the average, correlation 0.79.

The slots: five minutes from 04:00 local (Oura's activity day), awake (outside every sleep window), with
the day's MET mean at 1.8 or under, and the slot's mean heart rate. The features are the C++'s
(src/analysis/stress_estimate.cpp); the weights it prints go there.
"""
import datetime
import json
import sqlite3
import sys

import numpy as np

RIDGE = 3.0
THRESHOLDS_UP = (4, 8, 12, 16, 20, 26)
THRESHOLDS_DOWN = (-4, -8, -12)
HISTORY_DAYS = 14
HRV_NIGHTS = 30
MIN_HISTORY = 200


def load(path):
    c = sqlite3.connect('file:%s?mode=ro' % path, uri=True)
    ts = lambda s: datetime.datetime.fromisoformat(s).timestamp()
    hr = c.execute("select utc, bpm from cloud_hr order by utc").fetchall()
    utc = np.array([h[0] for h in hr])
    bpm = np.array([h[1] for h in hr], float)
    sleeps = []
    night_hrv = {}
    for (j,) in c.execute("select json from cloud_docs where kind='sleep'"):
        d = json.loads(j)
        if d.get('bedtime_start'):
            sleeps.append((ts(d['bedtime_start']), ts(d['bedtime_end'])))
        if d.get('type') == 'long_sleep' and d.get('average_hrv'):
            night_hrv[d['day']] = d['average_hrv']
    activity = {}
    for (j,) in c.execute("select json from cloud_docs where kind='daily_activity'"):
        d = json.loads(j)
        activity[d['day']] = d
    days = []
    for (j,) in c.execute("select json from cloud_docs where kind='daily_stress'"):
        d = json.loads(j)
        a = activity.get(d['day'])
        if not a or not a.get('met') or len(a['met']['items']) != 1440:
            continue
        t0 = ts(a['met']['timestamp'])
        lo, hi = np.searchsorted(utc, [t0, t0 + 86400])
        bins = []
        sums = np.zeros(288)
        counts = np.zeros(288)
        for t, b in zip(utc[lo:hi], bpm[lo:hi]):
            k = int((t - t0) // 300)
            if 0 <= k < 288:
                sums[k] += b
                counts[k] += 1
        met = a['met']['items']
        for k in range(288):
            if counts[k] == 0:
                continue
            a0, a1 = t0 + 300 * k, t0 + 300 * k + 300
            if any(s0 < a1 and s1 > a0 for s0, s1 in sleeps):
                continue
            if sum(met[k * 5:k * 5 + 5]) / 5 > 1.8:
                continue
            bins.append(sums[k] / counts[k])
        days.append(dict(day=d['day'], bins=np.array(bins), stress=(d.get('stress_high') or 0) / 300,
                         summary=d.get('day_summary')))
    days.sort(key=lambda x: x['day'])
    return days, night_hrv


def features(days, night_hrv, i):
    hist = [days[j]['bins'] for j in range(max(0, i - HISTORY_DAYS), i) if len(days[j]['bins'])]
    hist = np.concatenate(hist) if hist else np.array([])
    today = days[i]['bins']
    if len(hist) < MIN_HISTORY or len(today) == 0:
        return None
    dl = today - np.percentile(hist, 50)
    f = [1.0, len(today) / 100]
    f += [np.sum(dl >= t) / 10 for t in THRESHOLDS_UP]
    f += [np.sum(dl <= t) / 10 for t in THRESHOLDS_DOWN]
    f.append(np.median(dl) / 5)
    day = days[i]['day']
    prior = [night_hrv[days[j]['day']] for j in range(max(0, i - HRV_NIGHTS), i) if days[j]['day'] in night_hrv]
    f.append((night_hrv[day] - np.mean(prior)) / 10 if day in night_hrv and len(prior) >= 5 else 0.0)
    return f


def ridge(X, y):
    P = np.eye(X.shape[1]) * RIDGE
    P[0, 0] = 0
    return np.linalg.solve(X.T @ X + P, X.T @ y)


def main():
    days, night_hrv = load(sys.argv[1])
    rows = [(i, features(days, night_hrv, i)) for i in range(len(days))]
    rows = [(i, f) for i, f in rows if f]
    idx = [i for i, _ in rows]
    X = np.array([f for _, f in rows])
    y = np.array([days[i]['stress'] for i in idx])
    split = int(len(idx) * 0.7)
    w = ridge(X[:split], y[:split])
    p = np.clip(X @ w, 0, None)
    print('%d days with features; fitted on %d, tested on %d' % (len(idx), split, len(idx) - split))
    print('test: mean error %.2f slots (%.0f min), guessing the mean %.2f, correlation %.3f' % (
        np.mean(np.abs(p[split:] - y[split:])), 5 * np.mean(np.abs(p[split:] - y[split:])),
        np.mean(np.abs(y[split:] - y[:split].mean())), np.corrcoef(p[split:], y[split:])[0, 1]))
    stressful = np.array([days[i]['summary'] == 'stressful' for i in idx])
    for t in (30, 33, 36):
        print('test: "stressful" at %d slots: %.1f %% of days right' % (
            t, 100 * np.mean((p[split:] >= t) == stressful[split:])))
    w = ridge(X, y)
    p = np.clip(X @ w, 0, None)
    print('all days (in sample): mean error %.2f slots, correlation %.3f' % (
        np.mean(np.abs(p - y)), np.corrcoef(p, y)[0, 1]))
    print('weights for stress_estimate.cpp:')
    print('{ ' + ', '.join('%.5f' % v for v in w) + ' }')
    if '--emit' in sys.argv:
        # One line per day for the C++ check (stress_cli): date, truth, hrv z inputs and slots.
        out = sys.argv[sys.argv.index('--emit') + 1]
        with open(out, 'w') as f:
            for i, _ in rows:
                d = days[i]
                prior = [night_hrv[days[j]['day']] for j in range(max(0, i - HRV_NIGHTS), i) if days[j]['day'] in night_hrv]
                hist = np.concatenate([days[j]['bins'] for j in range(max(0, i - HISTORY_DAYS), i) if len(days[j]['bins'])])
                f.write('\t'.join([d['day'], '%g' % d['stress'], '%g' % night_hrv.get(d['day'], -1),
                                   ','.join('%g' % v for v in prior), ','.join('%.3f' % v for v in d['bins']),
                                   ','.join('%.3f' % v for v in hist)]) + '\n')


if __name__ == '__main__':
    main()
