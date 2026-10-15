#!/usr/bin/env python3
"""Checks src/analysis/oura_sleep_state.cpp against Oura's own library.

Random nights (stage runs, awake bursts, several motion styles, window lengths up
to and past the engine's 1799-epoch limit, several time zones) go through the
library in the emulator and through the C++ port; every number the engine hands
to the sleep score must agree.

    g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_state.cpp diff_state_cli.cpp -o /tmp/diff_state_cli
    python3 diff_sleep_state.py lib/arm64-v8a/libecore.so [cases] [seed]
"""
import random
import subprocess
import sys

from ecore_sleep import SleepOracle


def night(rnd):
    n = rnd.randint(40, 1900)
    stages = []
    mode = rnd.choice([1, 2, 3, 4])
    switch = rnd.choice([0.01, 0.03, 0.1])
    for _ in range(n):
        if rnd.random() < switch:
            mode = rnd.choice([1, 2, 2, 3, 4, 4])
        stages.append(mode)
    if rnd.random() < 0.3:
        k = rnd.randint(0, min(60, n))
        stages[:k] = [4] * k
    if rnd.random() < 0.2:
        k = rnd.randint(0, min(60, n))
        stages[n - k:] = [4] * k
    motion = [0] * n
    style = rnd.choice(['quiet', 'sparse', 'bursty', 'dense', 'heavy'])
    for i in range(n):
        if style == 'sparse' and rnd.random() < 0.05:
            motion[i] = rnd.randint(1, 30)
        elif style == 'bursty' and rnd.random() < 0.03:
            for k in range(rnd.randint(1, 6)):
                if i + k < n:
                    motion[i + k] = rnd.randint(5, 30)
        elif style == 'dense' and rnd.random() < 0.4:
            motion[i] = rnd.randint(1, 12)
        elif style == 'heavy' and rnd.random() < 0.2:
            motion[i] = rnd.randint(0, 255)
    if rnd.random() < 0.15:
        motion = motion[:max(1, n - rnd.randint(1, 5))]    # too few motion values: ignored
    start = rnd.randint(1700000000, 1790000000)
    tz = rnd.choice([0, 60, 120, 180, 330, 540, 600])
    tod = ((start + tz * 60) % 86400) // 60
    if tod >= 720:
        tod -= 1440
    return stages, motion, start, tz, tod


def main():
    lib = sys.argv[1]
    cases = int(sys.argv[2]) if len(sys.argv) > 2 else 300
    rnd = random.Random(int(sys.argv[3]) if len(sys.argv) > 3 else 1)
    oracle = SleepOracle(lib)
    nights = [night(rnd) for _ in range(cases)]

    lines = []
    for stages, motion, start, tz, tod in nights:
        lines.append('%d %d %d %d' % (len(stages), tod, len(stages), len(motion)))
        lines.append(' '.join(map(str, stages)))
        lines.append(' '.join(map(str, motion)) if motion else '')
    mine = subprocess.run(['/tmp/diff_state_cli'], input='\n'.join(lines) + '\n', capture_output=True,
                          text=True).stdout.splitlines()

    bad = 0
    for (stages, motion, start, tz, tod), line in zip(nights, mine):
        native = oracle.run(stages, motion, start=start, tz=tz)
        got = list(map(int, line.split()))
        want = [native['w1'] - 1, native['w2'], native['w3'], native['w4'], native['w5'], native['w6'],
                native['w7'], native['a8'], native['a9']]
        if got != want:
            bad += 1
            if bad <= 5:
                print('MISMATCH n=%d start=%d tz=%d\n  library %s\n  port    %s' % (len(stages), start, tz, want, got))
    print('%d nights, %d mismatches' % (cases, bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
