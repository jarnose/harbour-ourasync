#!/usr/bin/env python3
"""Checks whether the "motion events" input of the restfulness contributor can be had from the ring's motion
records (motion.csv of Oura's data export: one row per motion_event, with its seconds of motion).

    g++ ... restful_cli.cpp -o /tmp/restful_cli      (see the file)
    python3 motion_probe.py ring.db motion.csv /tmp/restful_cli BIRTHDATE

Each night's third input is not in the cloud data, but the official restfulness contributor limits it:
the range of values that give that contributor (given the night's wake-ups and restless epochs, which the
cloud data does give) is found, and the count the state machine makes from the motion seconds is checked
against it.
"""
import csv
import datetime
import json
import sqlite3
import subprocess
import sys
import numpy as np


def parse(s):
    return datetime.datetime.fromisoformat(s.replace('Z', '+00:00')).timestamp()


def local_minutes(iso):
    m = int(iso[11:13]) * 60 + int(iso[14:16])
    return m - 1440 if m >= 720 else m


def main():
    db, motion_csv, cli, birth = sys.argv[1:5]
    by = datetime.date.fromisoformat(birth)
    rows = []
    with open(motion_csv) as f:
        r = csv.reader(f, delimiter=';')
        next(r)
        for x in r:
            rows.append((parse(x[0]), int(x[6])))
    t = np.array([a for a, _ in rows])
    secs = np.array([b for _, b in rows])
    c = sqlite3.connect('file:%s?mode=ro' % db, uri=True)
    daily = {d: json.loads(j) for d, j in c.execute("select day,json from cloud_docs where kind='daily_sleep'")}
    nights = []
    for d, j in c.execute("select day,json from cloud_docs where kind='sleep'"):
        s = json.loads(j)
        if s.get('type') == 'long_sleep' and s.get('sleep_phase_30_sec') and s.get('movement_30_sec') and d in daily:
            if d not in [n[0] for n in nights[-1:]] or True:
                nights.append((d, s))
    best = {}
    for d, s in nights:
        if d not in best or s.get('total_sleep_duration', 0) > best[d].get('total_sleep_duration', 0):
            best[d] = s
    lines, keep = [], []
    for d, s in sorted(best.items()):
        if daily[d]['contributors'].get('restfulness') is None:
            continue
        age = int(d[:4]) - by.year - ((int(d[5:7]), int(d[8:10])) < (by.month, by.day))
        st = [int(ch) if ch in '1234' else 0 for ch in s['sleep_phase_30_sec']]
        fl = [1 if ch in '23456789' else 0 for ch in s['movement_30_sec']]
        t0 = datetime.datetime.fromisoformat(s['bedtime_start']).timestamp()
        n = len(st)
        sec = np.zeros(n, int)
        lo, hi = np.searchsorted(t, [t0, t0 + 30 * n])
        for tt, v in zip(t[lo:hi], secs[lo:hi]):
            sec[int((tt - t0) // 30)] += v
        lines.append(' '.join(map(str, [age, n, local_minutes(s['bedtime_start']), n, n, len(fl)] + st + list(np.minimum(sec, 255)) + fl)))
        keep.append((d, daily[d]['contributors']['restfulness']))
    out = subprocess.run([cli], input='\n'.join(lines) + '\n', capture_output=True, text=True).stdout.split('\n')
    inside = low = high = 0
    widths, rows_out = [], []
    for (d, official), line in zip(keep, out):
        v = list(map(int, line.split()))
        wake, rflag, rsec, ev_sec, ev_flag = v[:5]
        table = v[5:]
        ok = [cc for cc in range(256) if table[cc] == official]
        if not ok:
            continue
        a, b = min(ok), max(ok)
        widths.append(b - a + 1)
        rows_out.append((d, official, wake, rflag, rsec, ev_sec, a, b))
        if a <= ev_sec <= b:
            inside += 1
        elif ev_sec < a:
            low += 1
        else:
            high += 1
    n = len(rows_out)
    print('%d nights; count from the motion seconds inside the range the official restfulness allows: %d (%.0f %%), under %d, over %d'
          % (n, inside, 100 * inside / n, low, high))
    print('range widths: median %d, 90th percentile %d' % (np.median(widths), np.percentile(widths, 90)))
    print('restless epochs from motion seconds == cloud restless_periods on %d of %d nights' % (
        sum(1 for r in rows_out if r[3] == r[4]), n))
    for r in rows_out[:12]:
        print(r)
    return rows_out


if __name__ == '__main__':
    main()
