#!/usr/bin/env python3
"""Runs the app's activity score (activity_score.cpp) over the daily_activity documents of a cloud
import, taking the cloud's per-minute MET series as the ring's, and compares with Oura's.

    g++ -std=c++17 -I ../../src ../../src/analysis/activity_score.cpp activity_cli.cpp -o /tmp/activity_cli
    python3 compare_activity.py ring.db /tmp/activity_cli
"""
import datetime
import json
import sqlite3
import statistics
import subprocess
import sys

NAMES = ['meet_daily_targets', 'move_every_hour', 'recovery_time', 'stay_active', 'training_frequency', 'training_volume']


def main():
    c = sqlite3.connect(sys.argv[1])
    docs = {day: json.loads(j) for day, j in c.execute("select day,json from cloud_docs where kind='daily_activity' order by day")}
    days = sorted(docs)
    lines, used = [], []
    prev = None
    for d in days:
        r = docs[d]
        met = r.get('met', {}).get('items', [])
        if len(met) != 1440 or r.get('target_calories') is None:
            continue
        # A gap in the days restarts the week.
        day = datetime.date.fromisoformat(d)
        if prev and (day - prev).days != 1:
            lines.append(None)
        prev = day
        lines.append(f"{r['active_calories']} {r['target_calories']} " + ' '.join('nan' if m is None else repr(m) for m in met))
        used.append(d)
    # Run each unbroken stretch on its own.
    out_rows = {}
    stretch, start = [], 0
    chunks = []
    cur = []
    for item, d in zip([l for l in lines if l is not None] if None not in lines else lines, used):
        pass
    # Simple: split on None markers.
    cur, curdays, di = [], [], 0
    for item in lines:
        if item is None:
            chunks.append((cur, curdays)); cur, curdays = [], []
            continue
        cur.append(item); curdays.append(used[di]); di += 1
    chunks.append((cur, curdays))
    for cur, curdays in chunks:
        if not cur:
            continue
        res = subprocess.run([sys.argv[2]], input='\n'.join(cur) + '\n', capture_output=True, text=True).stdout.split('\n')
        for d, line in zip(curdays, res):
            v = list(map(int, line.split()))
            if v and v[0] >= 0:
                out_rows[d] = v
    errs = {n: [] for n in NAMES}
    score_err = []
    for d, v in out_rows.items():
        r = docs[d]
        if r.get('score') is None or not r.get('contributors'):
            continue
        score_err.append(v[0] - r['score'])
        for n, mine in zip(NAMES, v[1:]):
            theirs = r['contributors'].get(n)
            if theirs is not None:
                errs[n].append(mine - theirs)
    print(len(score_err), 'days with a full week')
    for n in NAMES:
        e = errs[n]
        print(f'{n:20s} mean err {statistics.mean(e):+6.2f}  mean abs {statistics.mean(abs(x) for x in e):5.2f}  exact {sum(1 for x in e if x == 0)/len(e):4.0%}')
    print('score: mean err %+.2f  mean abs %.2f  within 2: %.0f%%  within 5: %.0f%%  max %d' % (
        statistics.mean(score_err), statistics.mean(abs(e) for e in score_err),
        100 * sum(1 for e in score_err if abs(e) <= 2) / len(score_err),
        100 * sum(1 for e in score_err if abs(e) <= 5) / len(score_err), max(abs(e) for e in score_err)))


if __name__ == '__main__':
    main()
