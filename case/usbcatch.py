import cadquery as cq

th = 3.5
w = 11
oh = 6 # overhang
extra = 2
h = 45

res = (cq.Workplane("XY")
       .tag("bot")
       .transformed(offset=(-extra, 0, 0))
       .box(oh+th+extra, w, th, centered=False)
       .workplaneFromTagged("bot")
       .transformed(offset=(0, w/2, 0))
       .circle(8/2)
       .cutThruAll()
       .workplaneFromTagged("bot")
       .transformed(offset=(oh, 0, th))
       .box(th, w, h, centered=False)
       .edges("|Z")
       .fillet(0.5)
      )

spring_th = 1.8
cutout_w = 5
res = (res
       .workplaneFromTagged("bot")
       .transformed(offset=(oh+spring_th+0.5*cutout_w, 0, h*0.5), rotate=(90, 0, 0))
       .slot2D(25, cutout_w, 90)
       #.extrude(10)
       .cutThruAll()
          )

show_object(res)
