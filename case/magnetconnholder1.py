import cadquery as cq

slot_w = 6
slot_l = 19
l = 80
x_offset = 12
wire_offset = -20
h = 6

screw_d = 3.5
sh_d = 5.5
sh_h = 3

i_th = 1
bot_clearance = 1
i_x_clearance = 1
i_y_clearance = 7
w = slot_w + 2*i_th + 2*i_x_clearance + 2

inner = (cq.Workplane("XY")
         .transformed(offset=(x_offset, 0, 0))
         .tag("bot")
         .transformed(offset=(0, 0, bot_clearance))
         # body
         .slot2D(slot_l+2*i_th, slot_w+2*i_th, 0)
         .extrude(h - bot_clearance)
         .edges(">Z")
         .workplaneFromTagged("bot")
         # slot for connector
         .slot2D(slot_l, slot_w, 0)
         .cutThruAll()
         )

outer = (cq.Workplane("XY")
         .tag("bot")
         .slot2D(l, w, 0)
         .extrude(h)
         .faces(">Z")
         .fillet(0.75)
         .workplaneFromTagged("bot")
         .transformed(offset=(0, 0, h))
         # screw holes
         .rarray(l - 2*sh_d, 1, 2, 1)
         .cboreHole(screw_d, sh_d, sh_h)
         # slot for inner
         .transformed(offset=(x_offset, 0, 0))
         .slot2D(slot_l + 2*i_th + 2*i_y_clearance, slot_w + 2*i_th + 2*i_x_clearance, 0)
         .cutThruAll()
         .workplaneFromTagged("bot")
         .transformed(offset=(wire_offset, w/4, 0), rotate=(90, 0, 0))
         # wire slot
         .slot2D(5, 3, 0)
         .cutBlind(w*2)
         )

res = outer + inner
#res = inner

s_th = 1 # spring thickness
s_length = i_y_clearance-i_x_clearance
s_width = slot_w/2 + i_th + i_x_clearance
def spring(o, x, y):
    th = 1
    d1 = 2
    d2 = 1.5
    sPnts = [
        (x+1*d1, y+d2),
        (x+2*d1, y+0),
        (x+3*d1, y+-d2),
        (x+4*d1, y+0)
    ]
    s = o.workplaneFromTagged("bot").transformed(offset=(0, 0, 0))
    r = s.spline(sPnts, includeCurrent=True)
    return (cq.Workplane("XZ")
            .rect(th, h).sweep(r)
            )

def springPath(x, y):
    d1 = 2
    d2 = 1.5
    sPnts = [
        (x+1*d1, y+d2),
        (x+2*d1, y+0),
        (x+3*d1, y+-d2),
        (x+4*d1, y+0)
    ]
    r = s.spline(sPnts, includeCurrent=True)
    return (cq.Workplane("XZ")
            .rect(th, h).sweep(r)
            )

res = res + spring(res, 0, 0)
#res = spring(1, 0)
show_object(res)
