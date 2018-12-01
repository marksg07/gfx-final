Design Decisions:

- Shared pointers to store each of the brushes

- Quadratic brush just does linear brush computation for each pixel in the mask but then squares the value

- My brushes:
  - Swirl: Rotates the circular area under the brush. Things farthest away from the brush get rotated the most, but the mask is lowest, so it has the least effect. Uses a quadratic mask along with the rotation angle being (dist from center / radius^2) radians to make this effect possible. Also uses bilinear interpolation to decide the color after rotation.
  - Bezier: Operates differently from most brushes: Click to add a bezier point, keep clicking to add more. When you're done with the curve, you have to deselect and reselect the Bezier brush (think of it like a done/start new button.) Computes a Bezier curve with given points, which is a smooth curve starting and ending at two fixed points parametrized by a polynomial in the "time" dimension, which ranges from 0 to 1. Depending on the radius of the brush, it will choose some fixed number of points ranging between 0 and 1, evaluate the polynomial for each point, and then draw lines of width radius between each consecutive point (see drawLineToImg and corresponding line1, line2, line7, line8).

- Fixed alpha blending

Bugs:

Fixed alpha blending doesn't work the same as the demo, but it works pretty well. Both ways I tried to do it were off. First I tried to just linearly interpolate the RGBA values in the temporary layer directly, but that caused dark rings/borders around the linear and especially the quadratic brushes. Then I tried what I have now, which basically does a more correct alpha blending procedure. However, this one has the problem that the quadratic and linear brushes are a bit too dark (at least, compared to the demo). However, I think that this variation looks better.
