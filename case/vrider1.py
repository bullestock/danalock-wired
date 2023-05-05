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

e = 0.001

def assembly():
    free_h = 9
    cyl1 = cylinder(d = 8.5, h = free_h)
    cut = ccube(10, 10, 4)
    wi = 5
    disc_th = 3
    p1 = cyl1 - trans(0, -(5 + wi/2), -1, cut) - trans(0, (5 + wi/2), -1, cut)
    disc = cylinder(d = 25, h = disc_th)
    center_hole = cylinder(d = 3.5, h = free_h + disc_th + 2)
    insert_hole = cylinder(d = 4.2, h = 6)
    mount_hole = cylinder(d = 3.5, h = 5) + cylinder(d = 7, h = 1)
    d = 15
    mount_holes = trans(-d/2, 0, 0, mount_hole) + trans(d/2, 0, 0, mount_hole)
    return p1 + up(free_h)(disc) - down(1)(center_hole) - up(free_h - 2.5)(insert_hole) - up(free_h - e)(mount_holes)

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python danalockvrider1.py"
# End:
