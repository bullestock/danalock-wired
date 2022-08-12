import cadquery as cq

slot_w = 6
slot_l = 19
w = 9
l = 25
lid_h = 1
h1 = 6
h2 = 10.3

screw_d = 3.5
sh_d = 5.5
sh_h = 3

# channel for wire
ch_w = 5
ch_h = 2

res = (cq.Workplane("XY")
       .tag("bot")
       .box(l, 3*w, h1+lid_h, centered=(True, True, False))
       .edges("|Z")
       .fillet(w/2)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, w, h1+lid_h))
       .slot2D(slot_l, slot_w, 0)
       .cutBlind(-h1)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, -w, h1))
       .tag("base")
       .box(l, w, h2, centered=(True, True, False))
       .edges("|Z")
       .fillet(w/2-0.1)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 0, -h1), rotate=(0, 180, 0))
       .rarray(l - 2*sh_d, 1, 2, 1)
       .cboreHole(screw_d, sh_d, sh_h)
       .workplaneFromTagged("bot")
       .transformed(rotate=(90, 0, 0))
       .transformed(offset=(0, 2, -w))
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(2*w)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 0, -h1/2-ch_h))
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(3*w)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 0, h2+lid_h/2), rotate=(90, 0, 0))
       .slot2D(ch_w, ch_h*2, 0)
       .cutBlind(w)
      )

show_object(res)
