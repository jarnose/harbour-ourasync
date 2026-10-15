#!/usr/bin/env python3
"""Compares this repository's sleep-score arithmetic with Oura's own scores for the nights
of a ring.db that holds an import from Oura's cloud.

    python3 compare_sleep.py ring.db /tmp/score_cli [age] [--motion]

Needs the nights' 30-second stage strings (`sleep_phase_30_sec`) and, with --motion, the
`movement_30_sec` levels. The age is fitted when not given (it is not in the import).
"""
import json
import sqlite3
import subprocess
import sys
from collections import defaultdict

NAMES = ['total_sleep', 'deep_sleep', 'rem_sleep', 'efficiency', 'latency', 'restfulness', 'timing']


def local_minutes(iso):
    # 2026-10-10T23:19:00.000+03:00 -> minutes of local day, wrapped to -720..719
    t = iso[11:16]
    m = int(t[:2]) * 60 + int(t[3:5])
    return m - 1440 if m >= 720 else m


def load(db):
    c = sqlite3.connect(db)
    daily = {}
    for day, j in c.execute("select day,json from cloud_docs where kind='daily_sleep'"):
        daily[day] = json.loads(j)
    best = {}
    for day, j in c.execute("select day,json from cloud_docs where kind='sleep'"):
        o = json.loads(j)
        if o.get('type') != 'long_sleep' or not o.get('sleep_phase_30_sec'):
            continue
        if day not in best or o.get('total_sleep_duration', 0) > best[day].get('total_sleep_duration', 0):
            best[day] = o
    return [(d, daily[d], best[d]) for d in sorted(daily) if d in best]


def run(cli, nights, age, ctype=0, ccenter=0, motion=False):
    lines = []
    for day, ds, sl in nights:
        st = [int(ch) if ch in '1234' else 0 for ch in sl['sleep_phase_30_sec']]
        mo = []
        if motion and sl.get('movement_30_sec'):
            mo = [int(ch) for ch in sl['movement_30_sec']]
        lines.append(' '.join(map(str, [age, len(st), local_minutes(sl['bedtime_start']), ctype, ccenter, len(st), len(mo)] + st + mo)))
    out = subprocess.run([cli], input='\n'.join(lines) + '\n', capture_output=True, text=True).stdout.split('\n')
    return [list(map(int, l.split())) for l in out if l.strip()]


def summarise(nights, res, skip=()):
    errs = defaultdict(list)
    exact = defaultdict(int)
    for (day, ds, sl), r in zip(nights, res):
        mine = dict(zip(NAMES, r[1:8]))
        for k, v in ds['contributors'].items():
            if k in skip or v is None:
                continue
            errs[k].append(mine[k] - v)
            exact[k] += mine[k] == v
    return errs, exact


def main():
    db, cli = sys.argv[1], sys.argv[2]
    nights = load(db)
    print(len(nights), 'nights with stages and an official score')
    args = [a for a in sys.argv[3:] if not a.startswith('--')]
    motion = '--motion' in sys.argv
    if args:
        ages = [int(args[0])]
    else:
        ages = list(range(18, 80))
    best = None
    for age in ages:
        res = run(cli, nights, age, motion=motion)
        errs, exact = summarise(nights, res, skip=('restfulness',))
        total = sum(abs(e) for v in errs.values() for e in v)
        if best is None or total < best[0]:
            best = (total, age, res, errs, exact)
    total, age, res, errs, exact = best
    print('age', age)
    for k in NAMES:
        if k not in errs:
            continue
        e = errs[k]
        print(f'{k:12s} n={len(e):4d} exact={exact[k]:4d} ({100*exact[k]/len(e):.0f}%)  mean err {sum(e)/len(e):+.2f}  mean abs {sum(abs(x) for x in e)/len(e):.2f}  max abs {max(abs(x) for x in e)}')
    return nights, res


if __name__ == '__main__':
    main()
