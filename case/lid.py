#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re

# Assumes SolidPython is in site-packages or elsewhere in sys.path
from solid import *
from solid.utils import *
from utils import *

SEGMENTS = 256

e = 0.0001
dia = 59
lid_w = 36
corner_r = 3
lid_th = 2

pod_iw = 28
pod_ow = 32
pod_h = 22
pod_x = 2

cutout_h = 7
cutout_w = 12
cutout_offset = -1 # Offset from center

def assembly():
    outer = cylinder(d = dia, h = lid_w)
    inner = down(1)(cylinder(d = dia - 2*lid_th, h = lid_w+2))
    cutter = trans(0, dia/2, -1, ccube(dia + 2, dia, lid_w + 2))
    rounder_cube = cube([lid_th + 2, corner_r, corner_r])
    rounder_cyl = trans(-1, 0, 0, rot(0, 90, 0, cylinder(r = corner_r, h = lid_th + 4)))
    rounder = rounder_cube - rounder_cyl
    adj = 0
    rounder1 = trans(-dia/2 - 1, -corner_r + adj + 0.5, lid_w - corner_r, rounder)
    rounder2 = trans(dia/2 - lid_th - 1, -corner_r + adj + 0.5, lid_w - corner_r, rounder)
    pod_outer = trans(-pod_x, -pod_h, (lid_w - pod_ow)/2, cube([dia/2, pod_h, pod_ow]))
    pod_th = (pod_ow - pod_iw)/2
    pod_inner = trans(-pod_x-pod_th, -pod_h+pod_th, (lid_w - pod_iw)/2, cube([dia/2, pod_h, pod_iw]))
    cutout = trans(dia/2,
                   cutout_h/2 - (pod_h - 2),
                   (lid_w - cutout_w)/2 + cutout_offset,
                   ccube(10, cutout_h, cutout_w))
    slit1 = trans(dia/2 - 2, adj, -1, ccube(2, 2.5, lid_w + 2))
    slit2 = trans(-(dia/2 - 2), adj, -1, ccube(2, 2.5, lid_w + 2))
    wm_l = 34.5
    wm_w = 25.5
    wm_th = 2
    filler_h = wm_w+2*wm_th
    filler = up((lid_w - filler_h)/2)(cylinder(d = dia - 2*lid_th, h = filler_h) - trans(-dia/2, -dia/2 + (32 - pod_h), -2, cube([dia, dia, lid_w+4])))
    frame_inner = trans(-wm_l/2 + 8, -(pod_h - 2.5), (lid_w-wm_w)/2, cube([wm_l, 5, wm_w]))
    frame_outer = trans(-wm_l/2 + 6, -(pod_h - 2.5), (lid_w-(wm_w+2*wm_th))/2, cube([wm_l+wm_th, 3, wm_w+2*wm_th]))
    screwhole = trans(0, -dia/2 + 4.8, -lid_w/2, cylinder(d = 4.5, h = 2*lid_w))
    led_x = dia/2 - 5
    led_y = -5
    ledhole = trans(dia/2 - 5, led_y, 30, rot(90, 0, 90, cylinder(d = 5, h = 10)))
    ledtube = trans(dia/2 - 4, led_y, 30, rot(90, 0, 90, cylinder(d = 7, h = 4)))
    plughole = trans(dia/2, led_y, 10, ccube(5, 4.7, 6))
    plugtube = trans(dia/2 - 4, led_y, 10-1.5, ccube(5, 4.7+3, 6+3))
    pinshole = trans(dia/2 - 5, led_y, 10+1, ccube(10, 3, 4))
    sd_filler_h = 23
    sd_filler = up((lid_w - sd_filler_h)/2)(cylinder(d = dia - 2*lid_th, h = sd_filler_h) - rot(0, 0, -55, trans(-dia/2, -dia/2 + 5, -2, cube([dia, dia, lid_w+4]))))
    return outer + pod_outer - inner - cutter - rounder1 - rounder2 - cutout - slit1 - slit2 + filler - pod_inner + frame_outer - frame_inner - screwhole + ledtube - ledhole + plugtube - plughole - pinshole + sd_filler

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python lid.py"
# End:
