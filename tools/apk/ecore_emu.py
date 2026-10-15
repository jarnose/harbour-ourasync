#!/usr/bin/env python3
"""Runs functions of Oura's libecore.so (arm64 Android) in an emulator.

Purpose: an oracle. The sleep-score arithmetic in src/analysis/oura_sleep_score.cpp
was read from the library; this loads the library itself into Unicorn, with just
enough libc behind it, so that the port can be checked against what the library
actually returns for arbitrary inputs. Nothing is executed on a phone, nothing is
modified, and no part of the library is kept here: it reads a copy you supply.

    pip install unicorn pyelftools
    from ecore_emu import Ecore
    e = Ecore('lib/arm64-v8a/libecore.so')
    e.call('_ZN5nexus5ecore...', [args])        # by exported symbol (mangled) or address

Only what libecore's sleep and score code needs is provided (heap, string and
memory functions, gmtime/mktime, log2f). Anything else it touches stops the run
with the name of the missing function.
"""
import math
import struct
import time as _time

from elftools.elf.elffile import ELFFile
from unicorn import (UC_ARCH_ARM64, UC_HOOK_CODE, UC_HOOK_MEM_UNMAPPED, UC_MODE_ARM, Uc, UcError,
                     UC_PROT_ALL)
from unicorn.arm64_const import (UC_ARM64_REG_LR, UC_ARM64_REG_PC, UC_ARM64_REG_SP, UC_ARM64_REG_TPIDR_EL0,
                                 UC_ARM64_REG_X0, UC_ARM64_REG_X1, UC_ARM64_REG_X2, UC_ARM64_REG_X3,
                                 UC_ARM64_REG_X4, UC_ARM64_REG_X5, UC_ARM64_REG_X6, UC_ARM64_REG_X7,
                                 UC_ARM64_REG_S0, UC_ARM64_REG_Q0)

BASE = 0x10000000
STACK_TOP = 0x7ff00000
STACK_SIZE = 0x200000
HEAP_BASE = 0x40000000
HEAP_SIZE = 0x4000000
TLS_BASE = 0x50000000
STUB_BASE = 0x60000000
PAGE = 0x1000

ARGS = [UC_ARM64_REG_X0, UC_ARM64_REG_X1, UC_ARM64_REG_X2, UC_ARM64_REG_X3, UC_ARM64_REG_X4,
        UC_ARM64_REG_X5, UC_ARM64_REG_X6, UC_ARM64_REG_X7]


def up(v, a=PAGE):
    return (v + a - 1) // a * a


class Ecore:
    def __init__(self, path, verbose=False):
        self.verbose = verbose
        self.data = open(path, 'rb').read()
        self.elf = ELFFile(open(path, 'rb'))
        self.uc = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
        self.heap_ptr = HEAP_BASE
        self.sizes = {}
        self.stubs = {}          # address -> name
        self.stub_addr = {}      # name -> address
        self.symbols = {}        # defined dynamic symbols
        self.missing = set()
        self._load()
        self._relocate()

    # ---- loading -------------------------------------------------------------
    def _load(self):
        uc = self.uc
        for seg in self.elf.iter_segments():
            if seg['p_type'] != 'PT_LOAD':
                continue
            start = BASE + seg['p_vaddr']
            lo = start & ~(PAGE - 1)
            hi = up(start + seg['p_memsz'])
            try:
                uc.mem_map(lo, hi - lo, UC_PROT_ALL)
            except UcError:
                pass  # overlapping page with the previous segment
            uc.mem_write(start, self.data[seg['p_offset']:seg['p_offset'] + seg['p_filesz']])
        uc.mem_map(STACK_TOP - STACK_SIZE, STACK_SIZE, UC_PROT_ALL)
        uc.mem_map(HEAP_BASE, HEAP_SIZE, UC_PROT_ALL)
        uc.mem_map(TLS_BASE, 0x10000, UC_PROT_ALL)
        uc.mem_map(STUB_BASE, 0x10000, UC_PROT_ALL)
        uc.reg_write(UC_ARM64_REG_TPIDR_EL0, TLS_BASE + 0x100)
        # stack-protector canary slot at tpidr + 0x28
        uc.mem_write(TLS_BASE + 0x100 + 0x28, struct.pack('<Q', 0x1122334455667788))
        dyn = self.elf.get_section_by_name('.dynsym')
        for s in dyn.iter_symbols():
            if s['st_shndx'] != 'SHN_UNDEF' and s['st_value']:
                self.symbols[s.name] = BASE + s['st_value']
        uc.hook_add(UC_HOOK_CODE, self._stub_hook, begin=STUB_BASE, end=STUB_BASE + 0x10000)
        uc.hook_add(UC_HOOK_MEM_UNMAPPED, self._unmapped)

    def _stub_for(self, name):
        if name not in self.stub_addr:
            addr = STUB_BASE + 4 * len(self.stub_addr)
            self.stub_addr[name] = addr
            self.stubs[addr] = name
            self.uc.mem_write(addr, b'\xc0\x03\x5f\xd6')  # ret
        return self.stub_addr[name]

    def _relocate(self):
        dyn = self.elf.get_section_by_name('.dynsym')
        for secname in ('.rela.dyn', '.rela.plt'):
            sec = self.elf.get_section_by_name(secname)
            if not sec:
                continue
            for r in sec.iter_relocations():
                where = BASE + r['r_offset']
                t = r['r_info_type']
                if t == 1027:  # R_AARCH64_RELATIVE
                    self.uc.mem_write(where, struct.pack('<Q', BASE + r['r_addend']))
                    continue
                sym = dyn.get_symbol(r['r_info_sym'])
                name = sym.name
                if t in (1025, 1026, 257):  # GLOB_DAT, JUMP_SLOT, ABS64
                    if sym['st_shndx'] != 'SHN_UNDEF':
                        target = BASE + sym['st_value']
                    else:
                        target = self._stub_for(name.split('@')[0])
                    self.uc.mem_write(where, struct.pack('<Q', target + (r['r_addend'] if t == 257 else 0)))

    # ---- helpers -------------------------------------------------------------
    def malloc(self, n):
        n = up(max(n, 1), 16)
        p = self.heap_ptr
        self.heap_ptr += n
        if self.heap_ptr > HEAP_BASE + HEAP_SIZE:
            raise MemoryError('emulated heap exhausted')
        self.sizes[p] = n
        return p

    def write(self, addr, b):
        self.uc.mem_write(addr, bytes(b))

    def read(self, addr, n):
        return bytes(self.uc.mem_read(addr, n))

    def cstr(self, addr):
        out = b''
        while True:
            c = self.read(addr + len(out), 1)
            if c == b'\0':
                return out
            out += c

    def reg(self, r):
        return self.uc.reg_read(r)

    # ---- libc ----------------------------------------------------------------
    def _stub_hook(self, uc, address, size, user):
        name = self.stubs.get(address)
        if name is None:
            return
        a = [uc.reg_read(r) for r in ARGS]
        ret = 0
        if name in ('malloc', '_Znwm', '_Znam'):
            ret = self.malloc(a[0])
        elif name == 'calloc':
            ret = self.malloc(a[0] * a[1])
        elif name == 'realloc':
            ret = self.malloc(a[1])
            if a[0]:
                old = min(self.sizes.get(a[0], 0), a[1])
                self.write(ret, self.read(a[0], old))
        elif name in ('free', '_ZdlPv', '_ZdaPv'):
            ret = 0
        elif name in ('memset',):
            self.write(a[0], bytes([a[1] & 0xff]) * a[2])
            ret = a[0]
        elif name in ('memcpy', 'memmove'):
            self.write(a[0], self.read(a[1], a[2]))
            ret = a[0]
        elif name in ('strlen', '__strlen_chk'):
            ret = len(self.cstr(a[0]))
        elif name == 'strcmp':
            x, y = self.cstr(a[0]), self.cstr(a[1])
            ret = (x > y) - (x < y)
        elif name == 'strncmp':
            x, y = self.cstr(a[0])[:a[2]], self.cstr(a[1])[:a[2]]
            ret = (x > y) - (x < y)
        elif name == 'memchr':
            blob = self.read(a[0], a[2])
            i = blob.find(bytes([a[1] & 0xff]))
            ret = a[0] + i if i >= 0 else 0
        elif name == 'time':
            ret = int(_time.time())
        elif name in ('gmtime', 'gmtime_r'):
            t = struct.unpack('<q', self.read(a[0], 8))[0]
            tm = _time.gmtime(t)
            buf = self.malloc(64) if name == 'gmtime' else a[1]
            # struct tm: sec min hour mday mon year wday yday isdst (ints)
            self.write(buf, struct.pack('<9i', tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon - 1 + 0,
                                        tm.tm_year - 1900, (tm.tm_wday + 1) % 7, tm.tm_yday - 1, 0))
            ret = buf
        elif name == 'mktime':
            f = struct.unpack('<9i', self.read(a[0], 36))
            import calendar
            ret = calendar.timegm((f[5] + 1900, f[4] + 1, f[3], f[2], f[1], f[0], 0, 0, 0))
        elif name == 'log2f':
            v = struct.unpack('<f', struct.pack('<I', uc.reg_read(UC_ARM64_REG_S0)))[0]
            r = math.log2(v) if v > 0 else float('-inf')
            uc.reg_write(UC_ARM64_REG_S0, struct.unpack('<I', struct.pack('<f', r))[0])
        elif name == 'qsort':
            n, sz, cmp_ = a[1], a[2], a[3]
            items = [self.read(a[0] + i * sz, sz) for i in range(n)]
            # only used on plain ints in libecore: compare as signed 32-bit little endian
            items.sort(key=lambda b: struct.unpack('<i', b[:4])[0] if sz >= 4 else b[0])
            for i, it in enumerate(items):
                self.write(a[0] + i * sz, it)
        elif name == '__errno':
            ret = TLS_BASE + 0x800
        elif name in ('printf', 'puts', 'fprintf', 'fputc', 'fwrite', 'vsnprintf', 'snprintf', '__android_log_print'):
            ret = 0
        elif name == '__stack_chk_fail':
            raise RuntimeError('stack check failed in emulation')
        else:
            self.missing.add(name)
            raise RuntimeError('libc function not provided by the emulator: ' + name)
        uc.reg_write(UC_ARM64_REG_X0, ret & 0xffffffffffffffff)

    def _unmapped(self, uc, access, address, size, value, user):
        raise RuntimeError('unmapped memory access at %#x (pc=%#x)' % (address, uc.reg_read(UC_ARM64_REG_PC)))

    # ---- calling -------------------------------------------------------------
    def addr(self, name_or_addr):
        if isinstance(name_or_addr, int):
            return BASE + name_or_addr
        if name_or_addr in self.symbols:
            return self.symbols[name_or_addr]
        for n, a in self.symbols.items():
            if name_or_addr in n:
                return a
        raise KeyError(name_or_addr)

    def call(self, target, args=(), fargs=(), max_instructions=50_000_000):
        uc = self.uc
        sp = STACK_TOP - 0x1000
        uc.reg_write(UC_ARM64_REG_SP, sp)
        for reg, v in zip(ARGS, args):
            uc.reg_write(reg, v & 0xffffffffffffffff)
        for i, f in enumerate(fargs):
            uc.reg_write(UC_ARM64_REG_S0 + i, struct.unpack('<I', struct.pack('<f', f))[0])
        end = STUB_BASE + 0xfff0
        uc.mem_write(end, b'\xc0\x03\x5f\xd6')
        uc.reg_write(UC_ARM64_REG_LR, end)
        uc.emu_start(self.addr(target), end, count=max_instructions)
        return uc.reg_read(UC_ARM64_REG_X0)
