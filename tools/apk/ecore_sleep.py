"""Drives libecore's numerical sleep analysis in the emulator and reports what it hands to the score.

`calculate_sleep_score_numerical(out, in)` is the entry point the app's engine uses
with already-decoded inputs: stage codes and motion seconds per 30-second epoch.
This builds that input structure (layout read from the function), runs it, and
captures the arguments of the call it makes to `ecore_sleep_score_calculate`, which
are exactly the numbers the port in src/analysis/oura_sleep_state.cpp has to
reproduce.
"""
import struct

from ecore_emu import BASE, Ecore
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import (UC_ARM64_REG_SP, UC_ARM64_REG_X0, UC_ARM64_REG_X1, UC_ARM64_REG_X2,
                                 UC_ARM64_REG_X3, UC_ARM64_REG_X4, UC_ARM64_REG_X5, UC_ARM64_REG_X6,
                                 UC_ARM64_REG_X7)

SCORE_CALCULATE = 0x7f9c8       # ecore_sleep_score_calculate in 7.27.0's libecore.so
NUMERICAL = '_ZN5nexus5ecore31calculate_sleep_score_numericalEPNS0_16s_sleep_output_tEPKNS0_27s_ecore_sleep_score_input_tE'


class SleepOracle:
    def __init__(self, lib):
        self.e = Ecore(lib)
        self.captured = []
        self.e.uc.hook_add(UC_HOOK_CODE, self._on_score, begin=BASE + SCORE_CALCULATE, end=BASE + SCORE_CALCULATE)

    def _on_score(self, uc, addr, size, user):
        regs = [uc.reg_read(r) for r in (UC_ARM64_REG_X0, UC_ARM64_REG_X1, UC_ARM64_REG_X2, UC_ARM64_REG_X3,
                                         UC_ARM64_REG_X4, UC_ARM64_REG_X5, UC_ARM64_REG_X6, UC_ARM64_REG_X7)]
        stack = self.e.read(uc.reg_read(UC_ARM64_REG_SP), 0x40)
        a8 = stack[0]
        a9 = struct.unpack('<h', stack[8:10])[0]
        a10 = stack[16]
        a11 = struct.unpack('<i', stack[24:28])[0]
        self.captured.append(dict(w1=regs[1] & 0xffff, w2=regs[2] & 0xffff, w3=regs[3] & 0xffff,
                                  w4=regs[4] & 0xffff, w5=regs[5] & 0xff, w6=regs[6] & 0xff,
                                  w7=regs[7] & 0xffff, a8=a8, a9=a9, a10=a10, a11=a11))

    def run(self, stages, motion, age=30, start=1760000000, tz=0, chrono=(255, 0)):
        """stages: code per epoch (1 deep, 2 light, 3 REM, 4 awake); motion: seconds per epoch.
        tz: minutes east of UTC (not negative: the library reads it as unsigned)."""
        e = self.e
        n = len(stages)
        nibbles = (n + 1) // 2
        packed = bytearray(nibbles)
        for i, s in enumerate(stages):
            packed[i // 2] |= (s & 0xf) << (4 * (i % 2))
        stages_p = e.malloc(nibbles + 16)
        e.write(stages_p, bytes(packed))
        motion_p = e.malloc(len(motion) + 16)
        e.write(motion_p, bytes(motion))
        temps_p = e.malloc(64)
        user = e.malloc(64)
        e.write(user, struct.pack('<IIIIII', 0, age, 0, 0, 0, 0))
        chrono_p = e.malloc(64)
        e.write(chrono_p, struct.pack('<ii', chrono[0], chrono[1] * 60))
        records = e.malloc(24 * (n + 8))
        inp = e.malloc(0x100)
        buf = bytearray(0x100)
        struct.pack_into('<Q', buf, 0x00, start * 1000)
        struct.pack_into('<Q', buf, 0x10, stages_p)
        struct.pack_into('<I', buf, 0x18, nibbles)
        struct.pack_into('<Q', buf, 0x20, records)
        struct.pack_into('<Q', buf, 0x30, start * 1000)
        struct.pack_into('<i', buf, 0x38, 0)
        struct.pack_into('<Q', buf, 0x40, motion_p)
        struct.pack_into('<I', buf, 0x48, len(motion))
        struct.pack_into('<Q', buf, 0x50, start)
        struct.pack_into('<i', buf, 0x58, tz)
        struct.pack_into('<Q', buf, 0x60, start + 30 * n)
        struct.pack_into('<i', buf, 0x68, tz)
        struct.pack_into('<Q', buf, 0x90, temps_p)
        struct.pack_into('<Q', buf, 0xc8, user)
        struct.pack_into('<Q', buf, 0xd0, chrono_p)
        e.write(inp, bytes(buf))
        out = e.malloc(0x200)
        outbuf = bytearray(0x200)
        for off in (0x40, 0x48, 0x50, 0x80, 0x88, 0xb8, 0x68, 0x70):
            struct.pack_into('<Q', outbuf, off, e.malloc(0x800))
        e.write(out, bytes(outbuf))
        self.captured.clear()
        e.call(NUMERICAL, [out, inp])
        return self.captured[0] if self.captured else None
