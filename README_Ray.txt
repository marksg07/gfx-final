In addition to supersampling, multithreading, kd-trees, and adaptive supersampling, which I did for Intersect, and the required features (texture mapping, reflection, shadows, specular highlights, directional lights, attenuation), I implemented:
- Transparency: Refraction and reflection with transparent objects.
I used glm::refract to figure out the angles for entering and leaving an object. I have a check for if I'm entering or if I'm leaving, and if I am entering an object with refraction, I only check the object I'm currently inside for the intersection test. I do the same if I am inside an object with reflection. I also use Schlick's approximation if there is both refraction and reflection enabled.

TESTS: ray/refract.xml, ray/refract2.xml

- Bump mapping
I get the (bicubic) interpolated height values of the place I hit on the texture with the ray (me), as well as the interpolated height values of one pixel to the right, left, up, and down of that pixel. Then, I use a precalculated tangent and bitangent that point towards the +u and -v directions, respectively, to find where all of those height values land in world space. Then for each triangle touching the me point (RMD, DML, LMU, UMR for U=up, R=right, D=down, L=left, M=me), I calculate its surface normal with the cross product. Then I average those surface normals to get the new normal for the point.

TESTS: mytests/brick_wall.xml (Note: both texture and bump mapping are used), mytests/refract-bump.xml (Refraction + bump mapping = cool, use a low depth), shared/test_bumpmap.xml, mytests/cube_bump.xml, mytests/cone_bump.xml, mytests/cyl_bump.xml, ray/sphere_texture_test.xml, mytests/test-bumpy.xml

- Parallax mapping
If it is enabled, before bump mapping, I alter my initial (me) UV coordinates by finding the interpolated height value at me, and multiplying my normalized point-to-eye vector by that to get an approximation of where I actually hit the surface. Then, I use the tangent and bitangent vectors to find the intersection point in tangent space, and then I convert that to texture coordinates by taking u += x, v -= y, because the tangent vector and the bitangent vector are lined up with the +u and -v directions, as previously stated. Those new texture coordinates are where I sample the bump map.

TESTS: mytests/brick_wall.xml, shared/test_bumpmap.xml, mytests/cube_bump.xml, mytests/cone_bump.xml, mytests/cyl_bump.xml, mytests/sphere_texture_test.xml, mytests/test-bumpy.xml

- Spotlights
Pretty simple, just spotlights. Note: I ignored the radius parameter.

TESTS: ray/spotlight_on_cheese.xml

- Bicubic texture filtering
Samples from a texture using bicubic interpolation, so that low-quality textures look smoother.

TESTS: Any texture tests
