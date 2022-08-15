import cadquery as cq

slot_w = 5.6
slot_l = 18.6
l = 80
x_offset = 12
wire_offset = -30
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
         # body
         .slot2D(slot_l+2*i_th, slot_w+2*i_th, 0)
         .extrude(h)
         .edges(">Z")
         .workplaneFromTagged("bot")
         # slot for connector
         .slot2D(slot_l, slot_w, 0)
         .cutThruAll()
         .workplaneFromTagged("bot")
         .transformed(offset=(-12, 2, 0), rotate=(90, 90, 0))
         # wire slot
         .slot2D(5, 3, 0)
         .cutBlind(5)
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

s_th = 0.6 # spring thickness - was 1
s_length = i_y_clearance + s_th
s_width = slot_w/2 + i_th + i_x_clearance
def spring():
    d1 = s_length/4
    d2 = 3
    sPnts = [
        (1*d1, d2),
        (2*d1, 0),
        (3*d1, -d2),
        (4*d1, 0)
    ]
    s = cq.Workplane("XY")
    r = s.spline(sPnts, includeCurrent=True)
    return (cq.Workplane("XZ")
            .rect(s_th, h).sweep(r)
            )

springs = (res
           .workplaneFromTagged("bot")
           .transformed(offset=(x_offset - 0.5*s_length, 0, h/2))
           .rarray(slot_l + s_length + 2*i_th - s_th, 1, 2, 1)
       .eachpoint(lambda loc: spring().val().moved(loc), True)
       )
res = res + springs

show_object(res)
