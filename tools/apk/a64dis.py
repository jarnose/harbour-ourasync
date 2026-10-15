#!/usr/bin/env python3
"""Disassembles functions of an Android arm64 .so by exported symbol (or address).

The native libraries of the Oura app (libecore.so, libappecore.so) are not
stripped of their exported symbol names, so a function can be found by a
fragment of its mangled name. Every `adrp+add` / `adr` / `ldr` reference to the
data segments is annotated with the bytes found there, and `bl` targets are
resolved through both the dynamic symbol table and the PLT, which is how calls
between exported functions of the same library appear.

    pip install pyelftools capstone
    a64dis.py libecore.so ecore_sleep_score_init_limits_v2
    a64dis.py libecore.so @0x7fc0c:704        # an unexported function: @address:size

Needs c++filt (binutils) to print demangled names. Nothing here modifies the
library, and no part of it is included in this repository.
"""
import subprocess
import sys

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
from capstone.arm64 import ARM64_OP_IMM, ARM64_OP_MEM
from elftools.elf.elffile import ELFFile


def demangle(name):
    try:
        return subprocess.run(['c++filt', name], capture_output=True, text=True).stdout.strip()
    except OSError:
        return name


def main(argv):
    lib, wanted = argv[1], argv[2:]
    data = open(lib, 'rb').read()
    elf = ELFFile(open(lib, 'rb'))
    segments = [(s['p_vaddr'], s['p_filesz'], s['p_offset'])
                for s in elf.iter_segments() if s['p_type'] == 'PT_LOAD']

    def offset(va):
        for vaddr, size, off in segments:
            if vaddr <= va < vaddr + size:
                return off + va - vaddr
        return None

    dynsym = elf.get_section_by_name('.dynsym')
    by_address = {s['st_value']: s.name for s in dynsym.iter_symbols() if s['st_value']}
    by_name = {s.name: (s['st_value'], s['st_size']) for s in dynsym.iter_symbols()}
    got_names = {r['r_offset']: dynsym.get_symbol(r['r_info_sym']).name
                 for r in elf.get_section_by_name('.rela.plt').iter_relocations()}

    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    md.detail = True

    def find(fragment):
        if fragment.startswith('@'):
            address, size = fragment[1:].split(':')
            return 'sub_' + address, int(address, 16), int(size)
        for name, (address, size) in by_name.items():
            if fragment in name:
                return name, address, size
        raise SystemExit('no symbol matching ' + fragment)

    for fragment in wanted:
        name, address, size = find(fragment)
        print('=====', demangle(name), hex(address), size)
        start = offset(address)
        regs = {}
        for ins in md.disasm(data[start:start + size], address):
            extra = ''
            if ins.mnemonic == 'adrp':
                regs[ins.operands[0].reg] = ins.operands[1].imm
            elif ins.mnemonic == 'add' and len(ins.operands) == 3 \
                    and ins.operands[2].type == ARM64_OP_IMM and ins.operands[1].reg in regs:
                target = regs[ins.operands[1].reg] + ins.operands[2].imm
                regs[ins.operands[0].reg] = target
                o = offset(target)
                extra = '  ; &%#x %s' % (target, data[o:o + 24].hex() if o is not None else '')
            elif ins.mnemonic.startswith('ldr') and ins.operands[1].type == ARM64_OP_MEM \
                    and ins.operands[1].mem.base in regs:
                target = regs[ins.operands[1].mem.base] + ins.operands[1].mem.disp
                o = offset(target)
                extra = '  ; [%#x] %s' % (target, data[o:o + 16].hex() if o is not None else '')
            elif ins.mnemonic in ('bl', 'b') and ins.operands[0].type == ARM64_OP_IMM:
                target = ins.operands[0].imm
                symbol = by_address.get(target)
                extra = '  ; -> ' + (demangle(symbol) if symbol else hex(target))
                if not symbol:
                    o = offset(target)
                    if o:
                        stub = list(md.disasm(data[o:o + 16], target))
                        try:
                            slot = stub[0].operands[1].imm + stub[1].operands[1].mem.disp
                            extra += ' ' + demangle(got_names.get(slot, '?'))
                        except (IndexError, AttributeError):
                            pass
            print('%x: %-6s %s%s' % (ins.address, ins.mnemonic, ins.op_str, extra))


if __name__ == '__main__':
    main(sys.argv)
