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

e = 0.001
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

# Wemos
wm_l = 34.5
wm_w = 25.5
wm_th = 2
wm_usb_offset = 1 # USB connector is not centered

filler_offset = 10

led_y = -5

pod_l = 15
pod_l2 = 22
pod_extra_w = 5
pod_h = 12
pod_h2 = 7
wt = 2

def pod():
    pod_outer = trans(pod_l2/2, 0, -pod_h2, roundccube(pod_l + pod_l2, wm_w + pod_extra_w, pod_h + pod_h2, 1.5))
    inner_w = pod_h + pod_h2 - 4*wt
    pod_inner = trans(-8, 0, -inner_w-1, roundccube(pod_l2, wm_w, inner_w, 1.5))
    usb_w = 12
    usb_h = 8
    usb_hole = ccube(pod_l + 5, usb_w, usb_h)
    pod = trans(-(wm_l + 2*wt + pod_l)/2, 0, -pod_h/2, down(1)(pod_outer) - trans(1, wm_usb_offset, (pod_h - usb_h)/2, hole()(usb_hole)))

    # Hole for power plug (JST PH)
    pwr_plug_l = 5
    pwr_plug_x = -(wm_l + pod_l)/2 - pwr_plug_l - 1
    pwr_plug_y = 10
    pwr_plug_z = -12
    es = .5 # empirics
    pwr_plug_w = 4.7 + es
    pwr_plug_h = 6 + es
    pwr_wire_sz = 4
    pwr_wirehole = trans(pwr_plug_x + 4, pwr_plug_y, pwr_plug_z + (pwr_plug_h - pwr_wire_sz)/2, ccube(pod_l + 3, pwr_wire_sz, pwr_wire_sz))
    pwr_plughole = trans(pwr_plug_x - 1, pwr_plug_y, pwr_plug_z, ccube(pwr_plug_l + 1, pwr_plug_w, pwr_plug_h))
    pwr_plugtube = trans(pwr_plug_x, pwr_plug_y, pwr_plug_z-1.5, ccube(pwr_plug_l, pwr_plug_w+3, pwr_plug_h+3))

    # Hole for door switches (2.54 mm header)
    sw_plug_x = -(wm_l + 2*wt + pod_l - 2)/2
    sw_plug_y = -10
    sw_plug_l = pod_l - 2
    sw_plug_z = -12
    es = 0.6 # empirics
    n = 2
    sw_hdr_w = n*2.5 + es
    sw_hdr_h = n*2.5 + es
    es = 0.5 # empirics
    sw_housing_w = n*2.6 + es
    sw_housing_h = n*2.6 + es
    sw_housing_l = 10
    sw_hdrhole = trans(sw_plug_x - 1, sw_plug_y, sw_plug_z, ccube(sw_plug_l + 10, sw_hdr_w, sw_hdr_h))
    sw_housinghole = trans(sw_plug_x - (sw_plug_l - sw_housing_l) - 1, sw_plug_y, sw_plug_z - (sw_housing_h - sw_hdr_h)/2, ccube(sw_housing_l, sw_housing_w, sw_housing_h))
    sw_hdrtube = trans(sw_plug_x, sw_plug_y, sw_plug_z-1.5, ccube(sw_plug_l, sw_hdr_w+3, sw_hdr_h+3))
    holes = hole()(sw_hdrhole + sw_housinghole + pwr_wirehole + pwr_plughole + pod_inner)
    return pod - holes

def wemos_holder():
    es = .1 # empirics
    frame_inner = ccube(wm_l+es, wm_w+es, wm_th+e)
    frame_outer = roundccube(wm_l + 2*wt, wm_w + 2*wt, 2*wm_th, 1)
    frame_cut = trans(-wm_l/2, wm_usb_offset, -wm_th, ccube(5, 12, wm_th+2))
    return trans(-1, 0, 0, frame_outer - hole()(frame_inner + frame_cut))

# Depression for switch mode power supply
def smps():
    w = 17
    l = 22
    h = 2
    wt = 1
    inner = ccube(w, l, h + e)
    outer = ccube(w + 2*wt, l + 2*wt, h)
    return trans(8, filler_offset - dia/2 - h, lid_w/2, rot(90, 0, 180, outer - hole()(inner)))

def shell_outer():
    # Hollow cylinder
    outer = cylinder(d = dia, h = lid_w, segments = 256)
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
    # LED hole
    led_x = dia/2 - 5
    ledhole = trans(dia/2 - 5, led_y, 30, rot(90, 0, 90, cylinder(d = 5, h = 10)))
    ledtube = trans(dia/2 - 4, led_y, 30, rot(90, 0, 90, cylinder(d = 7, h = 4)))
    return outer - cutter - rounder1 - rounder2 - slit1 - slit2 + ledtube - ledhole

def shell_block():
    # Block for screw hole
    filler_h = wm_w+4
    filler = up((lid_w - filler_h)/2)(cylinder(d = dia - 2*lid_th, h = filler_h) -
                                      trans(-dia/2, -dia/2 + filler_offset, -2, cube([dia, dia, lid_w+4])))
    screwhole = trans(0, -dia/2 + 4.8, -lid_w/2, cylinder(d = 4.5, h = 2*lid_w))
    cutter = trans(0, dia/2, -1, ccube(dia + 2, dia, lid_w + 2))
    return filler - screwhole

def shell_inner():
    inner = down(1)(cylinder(d = dia - 2*lid_th, h = lid_w+2, segments = 256))
    return inner

def assembly():
    sh = shell_outer()
    wmh = wemos_holder()
    part1 = sh + smps() - shell_inner() + shell_block()
    wemos_offset_x = -15
    wemos_offset_y = -15
    part2 = trans(wemos_offset_x, wemos_offset_y, lid_w/2, rot(180+90, 0, 90, wmh))
    pod_t = trans(wemos_offset_x, wemos_offset_y, lid_w/2, rot(180+90, 0, 90, pod())) - shell_inner()
    #return pod_t + part2
    return part1 + part2 + pod_t

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python lid.py"
# End:
