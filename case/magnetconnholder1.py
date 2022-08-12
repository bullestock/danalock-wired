import cadquery as cq

slot_w = 6
slot_l = 19
w = 9
l = 40
h = 6

screw_d = 3.5
sh_d = 5.5
sh_h = 3

res = (cq.Workplane("XY")
       .tag("bot")
       # body
       .slot2D(l, w, 0)
       .extrude(h)
       .edges(">Z")
       .fillet(0.75)
       .workplaneFromTagged("bot")
       # slot for connector
       .slot2D(slot_l, slot_w, 0)
       .cutThruAll()
       .workplaneFromTagged("bot")
       .transformed(offset=(0, 0, h))
       # screw holes
       .rarray(l - 2*sh_d, 1, 2, 1)
       .cboreHole(screw_d, sh_d, sh_h)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, 0, 0), rotate=(90, 0, 0))
       # wire slot
       .slot2D(5, 3, 0)
       .cutBlind(20)
      )

show_object(res)
