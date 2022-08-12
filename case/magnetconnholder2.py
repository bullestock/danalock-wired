import cadquery as cq

slot_w = 6
slot_l = 19
w = 9
l = 25
lid_h = 1
h1 = 6
h2 = 11.4

screw_d = 3.5
sh_d = 5.5
sh_h = 3+10

# channel for wire
ch_w = 7
ch_h = 4

res = (cq.Workplane("XY")
       .tag("bot")
       # large block
       .box(l, 3*w, h1+lid_h, centered=(True, True, False))
       .edges("|Z")
       .fillet(w/2)
       .edges("<Z")
       .fillet(2)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, w, h1+lid_h))
       # slot for connector
       .slot2D(slot_l, slot_w, 0)
       .cutBlind(-h1)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, -w, h1))
       .tag("base")
       # small block
       .box(l, w, h2, centered=(True, True, False))
       .edges("|Z")
       .fillet(w/2-0.1)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 0, -h1), rotate=(0, 180, 0))
       # screw holes
       .rarray(l - 2*sh_d, 1, 2, 1)
       .cboreHole(screw_d, sh_d, sh_h)
       .workplaneFromTagged("bot")
       .transformed(rotate=(90, 0, 0))
       .transformed(offset=(0, 3, -w))
       # wire channel part 1: connector to small block
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(2*w)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 0, -h1/2-ch_h/2))
       # wire channel part 2: through small block
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(3*w)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 0, h2+lid_h/2), rotate=(90, 0, 0))
       # wire channel part 3: exit through base of small block
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(w)
      )

show_object(res)
