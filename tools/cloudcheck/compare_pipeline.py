#!/usr/bin/env python3
"""Runs the app's own code (mergeCloud + ouraScoreForNight, tools/cloudcheck/pipeline_cli.cpp)
over a ring.db holding an import from Oura's cloud and compares each night's score and
seven contributors with Oura's.

    python3 compare_pipeline.py ring.db /tmp/pipeline_cli AGE [DATE=AGE ...]

AGE is the age on the first night; each DATE=AGE after it says from which day the person is
that age (a birthday), since the limits step with the age.
"""
import sqlite3
import subprocess
import sys
from collections import defaultdict

NAMES = ['total_sleep', 'deep_sleep', 'rem_sleep', 'efficiency', 'latency', 'restfulness', 'timing']


def main():
    db, cli, base = sys.argv[1], sys.argv[2], int(sys.argv[3])
    changes = sorted((a.split('=')[0], int(a.split('=')[1])) for a in sys.argv[4:])
    ages = sorted({base} | {a for _, a in changes})
    lines = []
    for kind, id_, day, js in sqlite3.connect(db).execute(
            "select kind,id,day,json from cloud_docs where kind in ('daily_sleep','sleep')"):
        lines.append(f'{kind}\t{id_}\t{day}\t{js}')
    blob = '\n'.join(lines) + '\n'
    per_age = {}
    for age in ages:
        out = subprocess.run([cli, str(age)], input=blob, capture_output=True, text=True).stdout.split('\n')
        per_age[age] = {l.split()[0]: list(map(int, l.split()[1:])) for l in out if l.strip()}

    def age_on(day):
        a = base
        for d, v in changes:
            if day >= d:
                a = v
        return a

    rows = []
    for day in sorted(per_age[base]):
        rows.append((day, per_age[age_on(day)][day]))
    n = len(rows)
    score_exact = sum(1 for _, r in rows if r[0] == r[1])
    score_err = [r[1] - r[0] for _, r in rows if r[1] >= 0]
    print(f'{n} nights; score exact {score_exact} ({100*score_exact/n:.1f}%), '
          f'within 1: {sum(1 for e in score_err if abs(e) <= 1)} ({100*sum(1 for e in score_err if abs(e)<=1)/n:.1f}%), '
          f'mean abs {sum(abs(e) for e in score_err)/len(score_err):.2f}, max abs {max(abs(e) for e in score_err)}')
    for i, name in enumerate(NAMES):
        errs = [(r[9 + i] - r[2 + i]) for _, r in rows if r[2 + i] >= 0 and r[9 + i] >= 0]
        if not errs:
            continue
        ex = sum(1 for e in errs if e == 0)
        print(f'{name:12s} n={len(errs):4d} exact={ex:4d} ({100*ex/len(errs):5.1f}%)  mean err {sum(errs)/len(errs):+.2f}  '
              f'mean abs {sum(abs(e) for e in errs)/len(errs):.2f}  max abs {max(abs(e) for e in errs)}')
    est = sum(1 for _, r in rows if r[17])
    print(f'restfulness estimated on {est} nights')


if __name__ == '__main__':
    main()
