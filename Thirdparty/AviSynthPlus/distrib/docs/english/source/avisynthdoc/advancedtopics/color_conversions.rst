
Color conversions
=================

work under construction.

to do:
`<http://forum.doom9.org/showthread.php?p=1084293#post1084293>`_

Should cover RGB->YUV conversions and lumarange scaling/preservation and when
to use which conversion.


Color conversions
-----------------

+--------------------------------+---------+---------+------+
| coefficients                   | Rec.601 | Rec.709 | FCC  |
+================================+=========+=========+======+
| Kr : Red channel coefficient   | 0.299   | 0.2125  | 0.3  |
+--------------------------------+---------+---------+------+
| Kg : Green channel coefficient | 0.587   | 0.7154  | 0.59 |
+--------------------------------+---------+---------+------+
| Kb : Blue channel coefficient  | 0.114   | 0.0721  | 0.11 |
+--------------------------------+---------+---------+------+

(0.0 <= [Y,R,G,B] <= 1.0) ; (-1.0 < [U,V] < 1.0)

Kg = 1 - Kr - Kb

| Y = Kr*R + Kg*G + Kb*B
| V = (R - Y)/(1-Kr) = R - G * Kg/(1-Kr) - B * Kb/(1-Kr)
| U = (B - Y)/(1-Kb) = - R * Kr/(1-Kb) - G * Kg/(1-Kb) + B

R = Y + V*(1-Kr)
G = Y - U*(1-Kb)*Kb/Kg - V*(1-Kr)*Kr/Kg
B = Y + U*(1-Kb)


Converting to programming values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**YUV [0,255] <-> RGB [0,255]** (0 <= [r,g,b] <= 255, 0 <= y <= 255, 0 <
[u,v] < 255)

| y = Y * 255
| v = V * 127.5 + 128
| u = U * 127.5 + 128
| r = R * 255
| g = G * 255
| b = B * 255

Substituting (Y,V,U,R,G,B) in the equations above and multiplying with 127.5
and respectively 255 gives

| y = Kr*r + Kg*g + Kb*b
| v - 128 = 0.5*(r - y)/(1-Kr) = 0.5 * r - 0.5 * g * Kg/(1-Kr) - 0.5 * b * Kb/(1-Kr)
| u - 128 = 0.5*(b - y)/(1-Kb) = - 0.5 * r * Kr/(1-Kb) - 0.5 * g * Kg/(1-Kb) + 0.5 * b

| r = y + 2*(v-128)*(1-Kr)
| g = y - 2*(u-128)*(1-Kb)*Kb/Kg - 2*(v-128)*(1-Kr)*Kr/Kg
| b = y + 2*(u-128)*(1-Kb)

**YUV [16,235] <-> RGB [0,255]** (0 <= [r,g,b] <= 255, 16 <= y <= 235, 16 <=
[u,v] <= 240)

| y = Y * 219 + 16
| u = U * 112 + 128
| v = V * 112 + 128
| r = R * 255
| g = G * 255
| b = B * 255


References
----------

| http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
| ITU BT.601 ...
| ITU BT.709 ...

$Date: 2010/02/27 14:45:12 $
