#!/usr/bin/env python3
"""Renders harbour-ourasync.svg into the launcher icons of icons/<size>x<size>/.

    python3 render.py        (needs cairosvg)
"""
import os
import cairosvg

here = os.path.dirname(os.path.abspath(__file__))
src = os.path.join(here, 'harbour-ourasync.svg')
for size in (86, 108, 128, 172):
    out = os.path.join(here, '..', '..', 'icons', '%dx%d' % (size, size), 'harbour-ourasync.png')
    cairosvg.svg2png(url=src, write_to=out, output_width=size, output_height=size)
    print(out)
