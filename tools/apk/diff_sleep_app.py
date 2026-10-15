#!/usr/bin/env python3
"""Checks the app's score path (oura_sleep_score.cpp: ouraSleepScoreApp) against the library.

Runs `ecore_sleep_score_init_limits` and `ecore_sleep_score_calculate_minutes` of Oura's
libecore.so in the emulator on random inputs and compares the seven contributors with the
C++ port.

    g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_score.cpp diff_app_cli.cpp -o /tmp/diff_app_cli
    python3 diff_sleep_app.py lib/arm64-v8a/libecore.so [cases] [seed]
"""
import random
import subprocess
import sys

from ecore_emu import Ecore

INIT_LIMITS = 0x7f7e4   # ecore_sleep_score_init_limits in 7.27.0
CALC_MINUTES = 0x7fecc  # ecore_sleep_score_calculate_minutes


def main():
    lib = sys.argv[1]
    cases = int(sys.argv[2]) if len(sys.argv) > 2 else 2000
    rnd = random.Random(int(sys.argv[3]) if len(sys.argv) > 3 else 1)
    e = Ecore(lib)
    out = e.malloc(64)
    lines, theirs = [], []
    for _ in range(cases):
        age = rnd.randint(14, 90)
        total = rnd.choice([rnd.randint(0, 700), rnd.randint(240, 600)])
        deep = rnd.randint(0, 160)
        rem = rnd.randint(0, 200)
        eff = rnd.randint(0, 100)
        lat_epochs = rnd.choice([rnd.randint(0, 120), rnd.randint(0, 255)])  # the engine caps it at 255
        wake = rnd.randint(0, 30)
        restless = rnd.randint(0, 700)
        events = rnd.randint(0, 255)
        mid = rnd.randint(-720, 719)
        ctype = rnd.choice([255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 100])
        centre = rnd.randint(0, 400)
        lines.append(' '.join(map(str, [age, total, deep, rem, eff, lat_epochs, wake, restless, events, mid, ctype, centre])))
        e.call(INIT_LIMITS, [age])
        e.write(out, bytes(16))
        w4 = (abs(lat_epochs - 30) + 1) >> 1
        e.call(CALC_MINUTES, [out, total, deep, rem, w4, eff, wake, restless],
               stack=[events, mid & 0xffff, ctype, centre])
        theirs.append(list(e.read(out, 7)))
    mine = subprocess.run(['/tmp/diff_app_cli'], input='\n'.join(lines) + '\n', capture_output=True, text=True).stdout.split('\n')
    bad = 0
    for line, t, m in zip(lines, theirs, mine):
        got = list(map(int, m.split()))
        if got != t:
            bad += 1
            if bad <= 10:
                print('MISMATCH', line, '\n  library', t, '\n  port   ', got)
    print(f'{cases} cases, {bad} mismatches')


if __name__ == '__main__':
    main()
