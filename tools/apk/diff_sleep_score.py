#!/usr/bin/env python3
"""Checks src/analysis/oura_sleep_score.cpp against Oura's own library.

Runs ecore_sleep_score_calculate inside the emulator (ecore_emu.py) on random
inputs and compares what it returns with the C++ port (diff_cli). Any
disagreement is printed with its inputs.

    g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_score.cpp diff_cli.cpp -o /tmp/diff_cli
    python3 diff_sleep_score.py lib/arm64-v8a/libecore.so [cases]
"""
import random
import struct
import subprocess
import sys

from ecore_emu import Ecore, STACK_TOP

SYM = '_ZN5nexus5ecore27ecore_sleep_score_calculateEPNS0_19s_sleep_summary_4_tEtttthhthshi'


def native(e, out, case):
    age, w1, w2, w3, w4, w5, w6, w7, a8, a9, a10, a11 = case
    e.call('ecore_sleep_score_init_limits_v2', [age])
    sp = STACK_TOP - 0x1000
    e.write(sp, struct.pack('<Q', a8) + struct.pack('<q', a9) + struct.pack('<Q', a10) + struct.pack('<q', a11))
    score = e.call(SYM, [out, w1, w2, w3, w4, w5, w6, w7])
    return score, list(e.read(out, 7))


def main():
    lib = sys.argv[1]
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 2000
    e = Ecore(lib)
    out = e.malloc(64)
    rnd = random.Random(1)
    cases = []
    for _ in range(n):
        inbed = rnd.randint(200, 1799)
        asleep = rnd.randint(int(inbed * 0.3), inbed)
        deep = rnd.randint(0, asleep // 3)
        rem = rnd.randint(0, asleep // 3)
        light = max(0, asleep - deep - rem)
        a10 = rnd.choice([255, 0, 1, 2, 3, 4, 5, 6, 7, 200])
        cases.append((rnd.randint(14, 90), inbed + 1, deep, light, rem, rnd.randint(0, 120), rnd.randint(0, 40),
                      rnd.randint(0, 60), rnd.randint(0, 40), rnd.randint(-720, 719), a10, rnd.randint(-300, 300)))
    mine = subprocess.run(['/tmp/diff_cli'], input='\n'.join(' '.join(map(str, c)) for c in cases) + '\n',
                          capture_output=True, text=True).stdout.splitlines()
    bad = 0
    for case, line in zip(cases, mine):
        m = list(map(int, line.split()))
        score, nat = native(e, out, case)
        mc = m[1:10]
        shown = m[10:17]
        # native contributor bytes: 0-4 direct, 6 timing; 5 is the three restfulness parts rescaled
        parts = [((2 * min(100, max(0, c)) + 3) & 0xff) * 0xab >> 10 for c in mc[5:8]]
        nat_rest = sum(parts)
        problems = []
        if score != m[0]:
            problems.append('score native %d mine %d' % (score, m[0]))
        for i in range(5):
            if nat[i] != max(1, min(100, mc[i])) and not (mc[i] == 0 and nat[i] == 1):
                problems.append('c%d native %d mine %d' % (i, nat[i], mc[i]))
        if nat[6] != max(1, min(100, mc[8])):
            problems.append('timing native %d mine %d' % (nat[6], mc[8]))
        if nat[5] != max(1, min(100, nat_rest)):
            problems.append('restful native %d mine %d' % (nat[5], nat_rest))
        # the port's own "displayed" contributors must equal the library's bytes
        if shown != nat:
            problems.append('displayed native %s mine %s' % (nat, shown))
        if problems:
            bad += 1
            if bad <= 15:
                print('MISMATCH', case, '|', '; '.join(problems))
    print('%d cases, %d mismatches' % (len(cases), bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
