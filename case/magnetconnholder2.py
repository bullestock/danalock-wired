import cadquery as cq

slot_w = 6
slot_l = 19
w = 10
l = 25
l2 = 55
lid_h = 1
h1 = 7
h2 = 11.4

screw_d = 3.5
sh_d = 5.5
sh_h = 12

# channel for wire
ch_w = 7
ch_h = 4

res = (cq.Workplane("XY")
       .tag("bot")
       # large block
       .box(l, 3*w, h1+lid_h, centered=(True, True, False))
       .workplaneFromTagged("bot")
       .transformed(offset=(0, w, h1+lid_h))
       # slot for connector
       .slot2D(slot_l, slot_w, 0)
       .cutBlind(-h1)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, -w, h1))
       .tag("base")
       # small block
       .workplaneFromTagged("base")
       .transformed(offset=(-(l2-l)/2, 0, -h1))
       .box(l2, w, 2*(h1+lid_h), centered=(True, True, False))
       .edges("|Z")
       .fillet(w/2-0.1)
       .edges("<Z")
       .fillet(2)
       .workplaneFromTagged("base")
       .transformed(offset=((l-l2)/2, 0, -h1), rotate=(0, 180, 0))
       # screw holes
       .rarray(l2 - 2*sh_d, 1, 2, 1)
       .cboreHole(screw_d, sh_d, sh_h)
       .workplaneFromTagged("bot")
       .transformed(rotate=(90, 0, 0))
       .transformed(offset=(0, 3, -w))
       # wire channel part 1: connector to small block
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(2*w)
       .workplaneFromTagged("base")
       .transformed(offset=(0, 2, -h1/2-ch_h/2))
       # wire channel part 2: through small block
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(3*w)
       # wire channel part 3: exit through base of small block
       .workplaneFromTagged("base")
       .transformed(offset=(0, 1, h1+ch_h/2-lid_h), rotate=(90, 0, 0))
       .slot2D(ch_w, ch_h, 0)
       .cutBlind(w)
       # reed slot
       .workplaneFromTagged("base")
       .transformed(offset=(-30, 0, h1+1), rotate=(90, 90, 0))
       .rect(7, 4)
       .cutBlind(18)
       # wire slot
       .workplaneFromTagged("base")
       .transformed(offset=(-15, 3, h1+1), rotate=(90, 90, 0))
       .rect(2, 2)
       .cutBlind(15)
      )

show_object(res)
