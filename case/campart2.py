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
    sq_w = 7
    sq_l = 10
    square = ccube(sq_w, sq_w, sq_l+10*e)
    disc_d = 20
    disc = cylinder(d = disc_d, h = sq_l)
    cam_l = 25
    cam = trans(cam_l/2 + disc_d*.4, 0, 0, ccube(cam_l, 7, sq_l))
    return disc - down(e)(square) + cam

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python danalockcampart2.py"
# End:
