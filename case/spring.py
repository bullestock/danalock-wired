import cadquery as cq

# th = 1
# d1 = 2
# d2 = 5
# d3 = 2
# d4 = d2/2

# result = (
#     cq.Sketch()
#     .segment((th/2, 0), (th/2, d1))
#     .arc((th/2, d1), (th/2 - d2/2, d1 + d2/2), (th/2 - d2, d1))
#     .segment((th/2 - d2, d1), (th/2 - d2, d1 - d3))
#     .arc((th/2 - d2, d1 - d3), (th/2 - d2 - 0.7*(d4 -th), d1 - d3 - 0.7*(d4 -th)), (th/2 - d2 - (d4 - th), d1 - d3 - (d4 - th)))
#     .assemble(tag='face')
# )

th = 1
d1 = 2
d2 = 5
d3 = 2
d4 = 5

s = cq.Workplane("XY")
sPnts = [
    (0, d1),
    (d2/2, d1+d2/2),
    (d2, d1),
    (d2, d1-d3),
    (d2+d4, d1-d3-d4)
]
#r = s.spline(sPnts, includeCurrent=True)

th = 1
d1 = 2
d2 = 1.5
x = 0.0

sPnts = [
    (x, 0),
    (x+1*d1, d2),
    (x+2*d1, 0),
    (x+3*d1, -d2),
    (x+4*d1, 0)
]
r = s.move(1, 1).spline(sPnts, includeCurrent=True)

result = cq.Workplane("XZ").rect(th, 5*th).sweep(r)

show_object(result)
