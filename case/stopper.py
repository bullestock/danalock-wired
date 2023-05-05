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
    w = 10 - .5
    rod = roundccube(w, w, 40, 2)
    head = roundccube(20, 20, 5, 2)
    return rod + trans(0, 0, 40 - 2, head)

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python danalockstopper.py"
# End:
