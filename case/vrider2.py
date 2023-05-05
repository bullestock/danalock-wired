#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re

# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *
from utils import *

SEGMENTS = 128

e = 0.01

def assembly():
    insert_hole = cylinder(d = 4.2, h = 15, segments = 16)
    d = 15
    insert_holes = trans(-d/2, 0, 0, insert_hole) + trans(d/2, 0, 0, insert_hole)
    rr = 4
    block1 = roundccube(35, 12, 13+rr, rr)
    hcutter = ccube(40, 20, rr+1)
    indent = rot(0, 90, 0, cylinder(d = 30, h = 40))
    pinch = 19
    indents = trans(-20, pinch, 10, indent) + trans(-20, -pinch, 10, indent)
    main = down(rr+1)(block1 - hcutter) - indents
    body = main
    return body - down(e)(insert_holes)

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python danalockvrider2.py"
# End:
