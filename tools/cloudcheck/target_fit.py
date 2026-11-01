#!/usr/bin/env python3
"""Oura's daily activity goal (daily_activity.target_calories) against the day's readiness score, in a
ring.db holding a cloud import. Checks the steps of src/analysis/activity_estimate.cpp.

    python3 target_fit.py ring.db [BASE=450]

Oura sets the goal at a basic value (the person's, 450 here) plus a step that follows the day's readiness:
over 872 days of one person the goal was one of 150, 200, 250, 300, 450, 550, 600, 650 and nothing else, and
sorted by readiness it climbs in bands (readiness 38: 150; 43-46: 200; 50-60: 250; 62-69: 300; 70-84: 450;
85-87: 550; 88-92: 600; 94-95: 650) with an overlap only at the edges and on 37 days of high readiness whose
goal stayed at 450 (why is not found: not the previous day's activity, target or score, nor the
contributors; a depth-3 tree on them adds 0.6 points of accuracy over guessing).
"""
import collections
import json
import sqlite3
import sys


def step(r):
    for floor, s in ((94, 200), (88, 150), (85, 100), (70, 0), (62, -150), (48, -200), (40, -250)):
        if r >= floor:
            return s
    return -300


MEET = [1, 25, 43, 60, 78, 95, 100, 100]


def main():
    c = sqlite3.connect('file:%s?mode=ro' % sys.argv[1], uri=True)
    base = int(sys.argv[2]) if len(sys.argv) > 2 else 450
    load = lambda kind: {json.loads(j)['day']: json.loads(j) for (j,) in c.execute("select json from cloud_docs where kind=?", (kind,))}
    act, rd = load('daily_activity'), load('daily_readiness')
    days = sorted(d for d in act if d in rd)
    exact = off = 0
    kinds = collections.Counter()
    for d in days:
        want, got = act[d]['target_calories'], max(100, base + step(rd[d]['score']))
        if want == got:
            exact += 1
        else:
            kinds[(got, want)] += 1
    print('%d days; the goal from the readiness bands matches Oura\'s on %d (%.1f %%)' % (len(days), exact, 100 * exact / len(days)))
    print('misses (predicted, Oura):', sorted(kinds.items(), key=lambda kv: -kv[1])[:8])
    # What it does to "meet daily targets": days of the last 7 with the active calories at the goal.
    same = total = 0
    allfixed = 0
    for i in range(6, len(days)):
        week = days[i - 6:i + 1]
        real = sum(act[x]['active_calories'] >= act[x]['target_calories'] for x in week)
        pred = sum(act[x]['active_calories'] >= max(100, base + step(rd[x]['score'])) for x in week)
        fixed = sum(act[x]['active_calories'] >= base for x in week)
        total += 1
        same += MEET[real] == MEET[pred]
        allfixed += MEET[real] == MEET[fixed]
    print('"meet daily targets" contributor from the predicted goals equals the one from Oura\'s goals on %.1f %% of days; with the basic goal for every day %.1f %%'
          % (100 * same / total, 100 * allfixed / total))


if __name__ == '__main__':
    main()
