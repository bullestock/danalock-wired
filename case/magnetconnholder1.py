import cadquery as cq

slot_w = 5.55
slot_l = 18.5
l = 70
x_offset = 8
wire_offset = -12
h = 7

screw_d = 3.5
sh_d = 5.5
sh_h = 3

i_th = 1
bot_clearance = 1
i_x_clearance = 1
i_y_clearance = 7
wall_th = 2
w = slot_w + 2*i_th + 2*i_x_clearance + 2*wall_th

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
         .transformed(offset=(-12, 2.5, 0), rotate=(90, 90, 0))
         # wire slot
         .rect(5, 3)
         .cutBlind(6)
         )

outer = (cq.Workplane("XY")
         .tag("bot")
         .slot2D(l, w, 0)
         .extrude(h)
         .faces(">Z")
         .fillet(1)
         .workplaneFromTagged("bot")
         .transformed(offset=(0, 0, h))
         # screw holes
         .rarray(l - 2*sh_d, 1, 2, 1)
         .cboreHole(screw_d, sh_d, sh_h)
         # slot for inner
         .transformed(offset=(x_offset, 0, 0))
         .slot2D(slot_l + 2*i_th + 2*i_y_clearance, slot_w + 2*i_th + 2*i_x_clearance, 0)
         .cutThruAll()
         # wire slot
         .workplaneFromTagged("bot")
         .transformed(offset=(wire_offset-2.5, 3, 0), rotate=(90, 90, 0))
         .rect(2, 4)
         .cutBlind(10)
         # wire exit
         .workplaneFromTagged("bot")
         .transformed(offset=(wire_offset, +2, 0), rotate=(90, 0, 0))
         .slot2D(4, 3, 0)
         .cutBlind(w)
         # magnet hole
         .workplaneFromTagged("bot")
         .transformed(offset=(wire_offset-8, 6, 0), rotate=(90, 0, 0))
         .slot2D(12.5, 6.25, 90)
         .cutBlind(6)
         )

res = outer + inner

s_th = 0.5 # spring thickness - 0.6 ok
s_length = i_y_clearance + s_th
s_width = slot_w/2 + i_th + i_x_clearance
def spring(sign):
    d1 = s_length/5*sign
    d2 = 3
    sPnts = [
        (1*d1, d2/2),
        (2*d1, 0),
        (3*d1, -d2),
        (4*d1, 0),
        (5*d1, d2/2)
    ]
    s = cq.Workplane("XY")
    r = s.spline(sPnts, includeCurrent=True)
    return (cq.Workplane("XZ")
            .rect(s_th, h).sweep(r)
            )

s_spacing = slot_l + s_length + 2*i_th - s_th
spring1 = (res
           .workplaneFromTagged("bot")
           .transformed(offset=(x_offset - 0.5*s_length + s_spacing/2, 0, h/2))
           .eachpoint(lambda loc: spring(1).val().moved(loc), True))
spring2 = (res
           .workplaneFromTagged("bot")
           .transformed(offset=(x_offset + 0.5*s_length - s_spacing/2, 0, h/2))
           .eachpoint(lambda loc: spring(-1).val().moved(loc), True)
       )
res = res + spring1 + spring2

show_object(res)
