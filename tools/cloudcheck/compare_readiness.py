#!/usr/bin/env python3
"""The readiness estimate (readiness_estimate.cpp) over a cloud import, against Oura's score.

    g++ ... readiness_cli.cpp -o /tmp/readiness_cli      (see the top of readiness_cli.cpp)
    python3 compare_readiness.py ring.db /tmp/readiness_cli
"""
import sqlite3
import statistics
import subprocess
import sys

rows = [f'{k}\t{i}\t{d}\t{j}' for k, i, d, j in sqlite3.connect(sys.argv[1]).execute(
    "select kind,id,day,json from cloud_docs where kind in ('daily_sleep','sleep','daily_readiness','daily_activity')")]
out = subprocess.run([sys.argv[2]], input='\n'.join(rows) + '\n', capture_output=True, text=True).stdout.split('\n')
errs = []
for line in out:
    p = line.split()
    if len(p) >= 3:
        errs.append(int(p[2]) - int(p[1]))
print(len(errs), 'days')
print('mean err %+.2f  mean abs %.2f  within 2: %.0f%%  within 5: %.0f%%  max %d' % (
    statistics.mean(errs), statistics.mean(abs(e) for e in errs),
    100 * sum(1 for e in errs if abs(e) <= 2) / len(errs), 100 * sum(1 for e in errs if abs(e) <= 5) / len(errs),
    max(abs(e) for e in errs)))
