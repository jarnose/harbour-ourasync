#!/usr/bin/env python3
"""Runs the app's stress estimate (stress_estimate.cpp) over the days of a cloud import and compares it
with Oura's stress_high.

    python3 stress_fit.py ring.db --emit /tmp/stress_days.tsv
    g++ -std=c++17 -I ../../src ../../src/analysis/stress_estimate.cpp stress_cli.cpp -o /tmp/stress_cli
    python3 stress_check.py /tmp/stress_days.tsv /tmp/stress_cli

The weights in the C++ are fitted on all of these days, so the errors here are in sample; the out of
sample figures are stress_fit.py's.
"""
import statistics
import subprocess
import sys

rows = []
out = subprocess.run([sys.argv[2]], stdin=open(sys.argv[1]), capture_output=True, text=True, check=True).stdout
for line in out.splitlines():
    day, truth, est = line.split()
    if est != '-':
        rows.append((day, float(truth) * 5, float(est)))
err = [abs(t - e) for _, t, e in rows]
print('%d days estimated; mean error %.1f minutes, median %.1f; %.0f %% within 30 minutes, %.0f %% within 60' % (
    len(rows), statistics.mean(err), statistics.median(err),
    100 * sum(e <= 30 for e in err) / len(err), 100 * sum(e <= 60 for e in err) / len(err)))
t = [r[1] for r in rows]
e = [r[2] for r in rows]
mt, me = statistics.mean(t), statistics.mean(e)
cov = sum((a - mt) * (b - me) for a, b in zip(t, e))
print('correlation %.3f; mean truth %.0f minutes, mean estimate %.0f' % (
    cov / (sum((a - mt) ** 2 for a in t) * sum((b - me) ** 2 for b in e)) ** 0.5, mt, me))
