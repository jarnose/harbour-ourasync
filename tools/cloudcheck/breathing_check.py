#!/usr/bin/env python3
"""Checks the app's breathing rate (breathing.cpp) against Oura's library on a real night.

Decodes the 0x60 interval events of a ring.db, takes its longest bedtime window and compares, over
stretches of it, the library's `breathing_rate_calculate_averages` (run in the emulator of
tools/apk/ecore_emu.py) with the C++.

    g++ -std=c++17 -I ../../src ../../src/analysis/breathing.cpp breathing_cli.cpp -o /tmp/breathing_cli
    python3 breathing_check.py ring.db lib/arm64-v8a/libecore.so /tmp/breathing_cli
"""
import os
import sqlite3
import statistics
import struct
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'apk'))
from ecore_emu import Ecore  # noqa: E402


def decode(p):
    mid = [(p[12] >> 5) & 6, (p[12] >> 3) & 6, (p[12] >> 1) & 6, (p[12] << 1) & 6, (p[13] >> 5) & 6, (p[13] >> 3) & 6]
    return [(p[i] << 3) | mid[i] | (p[6 + i] & 1) for i in range(6)]


def main():
    c = sqlite3.connect(sys.argv[1])
    syncs = [(ds, struct.unpack('<I', b[:4])[0]) for ds, b in c.execute("select ds,body from events where tag=66 order by ds")]

    def utc(ds):
        best = min(syncs, key=lambda s: abs(s[0] - ds))
        return best[1] + (ds - best[0]) / 10.0

    beats = []
    for ds, b in c.execute("select ds,body from events where tag=96 order by ds"):
        if len(b) != 14:
            continue
        ibis = decode(b)
        t = utc(ds) * 1000.0
        stamps = []
        for v in reversed(ibis):          # the newest beat ends at the event's time
            stamps.append(t)
            t -= v
        for v, tm in zip(ibis, reversed(stamps)):
            beats.append((int(tm), v))
    beats.sort()
    windows = [(utc(struct.unpack('<II', b[:8])[0]), utc(struct.unpack('<II', b[:8])[1]))
               for (b,) in c.execute("select body from events where tag=118 order by ds")]
    a, b = max(windows, key=lambda w: w[1] - w[0])
    print(f'night {(b - a) / 3600:.1f} h, {sum(1 for t, _ in beats if a * 1000 <= t <= b * 1000)} beats')

    e = Ecore(sys.argv[2])
    rows = []
    for off_h, len_h in ((0, 4), (1, 4), (2, 4), (3, 4), (4, 4), (0, 6), (2, 6), (0, 9)):
        s = a + off_h * 3600
        en = min(b, s + len_h * 3600)
        sub = [(t, v) for t, v in beats if s * 1000 <= t <= en * 1000]
        n = len(sub)
        ibi_p = e.malloc(2 * n + 16); e.write(ibi_p, struct.pack('<%dH' % n, *[v for _, v in sub]))
        ts_p = e.malloc(8 * n + 16); e.write(ts_p, struct.pack('<%dQ' % n, *[t for t, _ in sub]))
        epochs = int((en - s) / 30)
        st_p = e.malloc(epochs + 16); e.write(st_p, bytes([2]) * epochs)
        out = e.malloc(0x4000); e.write(out, bytes(0x4000))
        e.call('breathing_rate_calculate_averages', [out, int(s * 1000), ibi_p, ts_p, n, st_p, epochs], max_instructions=900_000_000)
        theirs = struct.unpack('<3f', e.read(out, 12))[0]
        text = f'{s} {en}\n' + ''.join(f'{t / 1000.0:.3f} {v}\n' for t, v in sub)
        mine = subprocess.run([sys.argv[3]], input=text, capture_output=True, text=True).stdout.strip()
        rows.append((off_h, len_h, theirs, float(mine) if mine != 'none' else None))
        print(rows[-1], flush=True)
    d = [abs(r[2] - r[3]) for r in rows if r[3] is not None]
    print(f'{len(d)} stretches: mean abs difference {statistics.mean(d):.3f} breaths/min, max {max(d):.3f}')


if __name__ == '__main__':
    main()
