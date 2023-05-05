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

SEGMENTS = 64
SEGMENTS_SHELL = SEGMENTS*4

e = 0.001
dia = 59     # outer diameter
lid_w = 36   # overall width
corner_r = 3 # rounding radius
lid_th = 2

cutout_h = 7
cutout_w = 12
cutout_offset = -1 # Offset from center

# Esp
esp_l = 39.5
esp_w = 31.5
esp_th = 2
esp_usb_offset = 0 # USB connector is centered on ESP32mini

filler_offset = 10
filler_indent = 3

# Pod
pod_l = 17
pod_l2 = 22 + 8
pod_l3 = 5
pod_h = 14
pod_h2 = 11
avpod_l = 21 # z
avpod_h = 14.5 # x
avpod_w = 24 # y
wt = 2
outer_w = lid_w - filler_indent

def pod():
    r = 1.5
    pod_w = lid_w
    pod_outer = (trans(-8, 0, pod_h2, roundcube(pod_l + pod_l2, pod_w, pod_h, r)) +
                 trans(0, 0, 0, roundcube(pod_l + pod_l3, pod_w, pod_h2 + 2*r, r)))
    inner_w = pod_h + pod_h2 - 4*wt
    pod_inner = trans(-r-8, 2, 3.5-2, roundcube(pod_l2, esp_w - 3, inner_w, r))
    usb_w = 12
    usb_h = 8
    esp_offset_x = esp_l/2 - 16 + 2
    esp_offset_y = pod_w/2 - .3
    esp_offset_z = 9-1
    usb_hole = trans(esp_l - esp_offset_x - 4, pod_w/2, 12, ccube(pod_l2, usb_w, usb_h))
    pod = pod_outer - trans(1, -esp_usb_offset, (pod_h - usb_h)/2, hole()(usb_hole))

    # LED hole
    led_x = pod_l
    led_y = 18
    led_z = 6.5
    led_l = 50
    ledhole = trans(led_x, led_y, led_z, rot(90, 0, 90, cylinder(d = 5, h = 50)))

    holes = pod_inner + ledhole# + ledtube

    esph = rot(0, 0, 180, esp_holder())
    part2 = trans(esp_offset_x, esp_offset_y, esp_offset_z, esph)

    return pod - hole()(holes) + part2

def esp_holder():
    es = .1 # empirics
    frame_inner = trans(0, 0, -5, ccube(esp_l+es, esp_w+es, esp_th+e+5))
    frame_outer = roundccube(esp_l + 2*wt, esp_w + wt, 2*esp_th, 1)
    frame_cut = trans(-esp_l/2, esp_usb_offset, -esp_th, ccube(5, 12, esp_th+2))
    groove_d = 3
    groove_c = rot(90, 0, 90, cylinder(d = groove_d, h = esp_l))
    groove_w = 10
    dd = (groove_w-groove_d)/2
    groove = trans(-esp_l/2, 0, groove_d/2, hull()(trans(0, dd, 0, groove_c) +
                                                   trans(0, -dd, 0, groove_c)))
    grooves = trans(0, esp_w/2 - groove_w/2, 0, groove) + trans(0, -(esp_w/2 - groove_w/2), 0, groove)
    a = frame_outer - hole()(frame_inner + frame_cut + grooves)
    return trans(-1, 0, pod_h2 + 1, a)

def shell_outer():
    # Hollow cylinder
    outer = cylinder(d = dia, h = lid_w, segments = SEGMENTS_SHELL)
    # Remove part of the cylinder that we don't need
    cutter = trans(0, dia/2, -1, ccube(dia + 2, dia, lid_w + 2))
    # Make round corners
    rounder_cube = cube([lid_th + 2, corner_r, corner_r])
    rounder_cyl = trans(-1, 0, 0, rot(0, 90, 0, cylinder(r = corner_r, h = lid_th + 4)))
    rounder = rounder_cube - rounder_cyl
    adj = 0
    rounder1 = trans(-dia/2 - 1, -corner_r + adj + 0.5, lid_w - corner_r, rounder)
    rounder2 = trans(dia/2 - lid_th - 1, -corner_r + adj + 0.5, lid_w - corner_r, rounder)
    # Inner slit at edges
    slit1 = trans(dia/2 - 2, adj, -1, ccube(2, 2.5, lid_w + 2))
    slit2 = trans(-(dia/2 - 2), adj, -1, ccube(2, 2.5, lid_w + 2))
    return outer - cutter - rounder1 - rounder2 - slit1 - slit2

def shell_block():
    # Block for screw hole
    filler_h = lid_w -  filler_indent
    filler = up(filler_indent)(cylinder(d = dia - 2*lid_th, h = filler_h) -
                                      trans(-dia/2, -dia/2 + filler_offset, -2, cube([dia, dia, lid_w+4])))
    screwhole = trans(0, -dia/2 + 4.8, -lid_w/2, cylinder(d = 4.5, h = 2*lid_w))
    cutter = trans(0, dia/2, -1, ccube(dia + 2, dia, lid_w + 2))
    return filler - screwhole

def shell_inner():
    inner = down(1)(cylinder(d = dia - 2*lid_th, h = lid_w+2, segments = SEGMENTS_SHELL))
    return inner

avpod_x = dia - 2.6*avpod_h

def avpod_outer():
    r = 1.5
    pod_outer = trans(avpod_x, -(avpod_w + 1.5),
                      lid_w - outer_w, roundcube(avpod_h, avpod_w, avpod_l, r))
    inner = cylinder(d = dia, h = lid_w)
    outer = cylinder(d = dia+35, h = lid_w)
    cutter = outer - inner
    return pod_outer - cutter

def avpod_inner():
    r = 1.5
    th = 2
    inner = trans(avpod_x + th, -(avpod_w + filler_indent),
                  lid_w - outer_w + th, roundcube(avpod_h+5, avpod_w - th, avpod_l - 2*th, r))
    y = -avpod_h + 4.5
    z = avpod_l/2 + filler_indent
    hole = trans(15, y, z, rot(0, 90, 0, cylinder(d = 12, h = 20)))
    cutout = trans(23.4, y, z, rot(0, 90, 0, cylinder(d = 15, h = 20)))
    return inner + hole + cutout

def upper_cutter():
    h = 10
    return down(h - filler_indent)(cylinder(d = dia - 2*lid_th, h = h))

def assembly():
    sh = shell_outer()
    part1 = sh - shell_inner() + shell_block()
    pod_offset_x = -3.2
    esp_offset_y = -12
    pod_t = trans(pod_offset_x, esp_offset_y - 8, 0,
                  rot(180+90, 180, 90, pod()))
    avpod_t = avpod_outer()
    return part1 + pod_t + avpod_t - avpod_inner() - upper_cutter()

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python lid.py"
# End:
