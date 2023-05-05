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

SEGMENTS = 32

e = 0.001

def assembly():
    block = roundcube(40, 30, 20, 2)
    cut = trans(-1, 2-5, -3, cube([45, 5, 25]))
    hole1 = trans(30 - 5, 2 + 13, -5, cube([10, 10, 30]))
    hole2 = trans(0 + 5, 2 + 13, -5, cube([10, 10, 30]))
    return block - cut - hole1 - hole2

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python danalockblock.py"
# End:
