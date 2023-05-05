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
    sq_w = 7-.4
    sq_l = 25
    square = ccube(sq_w, sq_w, sq_l)
    rod_l = 7
    rod = up(sq_l)(cylinder(d = 9.95, h = rod_l))
    disc_th = 2
    disc = up(sq_l + rod_l - e)(cylinder(d = 11, h = disc_th))
    cam = up(sq_l + rod_l + disc_th - e)(ccube(2.5, 10, 5.5))
    return square + cam + disc + rod

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python danalockcam.py"
# End:
