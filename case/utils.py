from solid import *
from solid.utils import *

# Cube, rounded in x/y, centered around (0, 0)
def roundxycube(length, width, height, radius):
    c = cylinder(r = radius, h = height)
    xo = length/2 - radius
    yo = width/2 - radius
    c1 = trans(xo, yo, 0, c)
    c2 = trans(-xo, yo, 0, c)
    c3 = trans(xo, -yo, 0, c)
    c4 = trans(-xo, -yo, 0, c)
    return hull()(c1 + c2 + c3 + c4)

def roundccube_s(length, width, height, radius):
    s = sphere(r = radius)
    xo = length/2 - radius
    yo = width/2 - radius
    zo = radius
    s1 = trans(xo, yo, zo, s)
    s2 = trans(-xo, yo, zo, s)
    s3 = trans(xo, -yo, zo, s)
    s4 = trans(-xo, -yo, zo, s)
    zo = height - radius
    s5 = trans(xo, yo, zo, s)
    s6 = trans(-xo, yo, zo, s)
    s7 = trans(xo, -yo, zo, s)
    s8 = trans(-xo, -yo, zo, s)
    return [s1, s2, s3, s4, s5, s6, s7, s8]

# Cube, rounded in x/y/z, centered in x/y
def roundccube(length, width, height, radius):
    ss = roundccube_s(length, width, height, radius)
    return hull()(sum(ss))

# Cube, rounded in x/y/z, at (0, 0)
def roundcube(length, width, height, radius):
    return translate([length/2, width/2, 0])(roundccube(length, width, height, radius))

# Cube centered around origin in x/y
def ccube(w, l, h):
    return translate([-w/2, -l/2, 0])(cube([w, l, h]))

# Cube centered around y axis
def yccube(w, l, h):
    return translate([-w/2, 0, 0])(cube([w, l, h]))

# Shortcut for translate()
def trans(x, y, z, s):
    return translate([x, y, z])(s)

# Shortcut for rotate()
def rot(x, y, z, s):
    return rotate([x, y, z])(s)

def half_cylinder(r, h):
    return cylinder(r=r, h=h) - trans(r, 0, 0, cube(2*r+0.01, center=True))

def tslotfoot(length, height):
    width = 20
    inset = 8
    body = roundxycube(length, width, height, 2)
    screwhole = hole()(translate([0, width/2, 0])(down(1)(cylinder(d = 4, h = height + 2))))
    return body + right(inset)(screwhole) + right(length - inset)(screwhole)
