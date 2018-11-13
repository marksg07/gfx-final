 ____  _                           
/ ___|| |__   __ _ _ __   ___  ___ 
\___ \| '_ \ / _` | '_ \ / _ \/ __|
 ___) | | | | (_| | |_) |  __/\__ \
|____/|_| |_|\__,_| .__/ \___||___/
                  |_|              
===============================================================================
Note: The camera starts in a weird place. You usually have to
reset the camera after starting (by using the cameras menu).

Extras: I added basic Torus support (the normals don't work).

My general idea for creating shapes, was to create a strip of the shape, which,
when rotated around the Y axis would construct the full shape. So for instance
in cylinder, I created the caps by constructing on wedge and rotating it around
the Y axis. I then created the side paneling and rotated that around the Y axis
to form the body. In the case of the cube I created one face and then rotated
that single face into the 6 positions necessary to form a cube.

In Shapes I decided to use some of the support code from lab02, like
OpenGLShape,but I also created my own classes to represent the data while I was
generating each shape. I created a GLVertex class, which could held data like
position, normal vector, and texture coordinates (and which I can extend for
use in further projects). I then created an GLTriangle class composed of 3
vertices. When I generated my shapes I generated vectors of GLTriangles, which
I then converted into a single GLfloat vector when I constructed the shape VAO
and VBO.


Composition:
    I used a GLCap class to account for the fact that the cone and cylinder
    both require a cap. By using composition the code is simpler to read and
    easier to extend.

    I also added a GLTransformable virtual class that I can implement (which
    provides prototypes for `rotate`, `translate`, and `reflect`). The class
    also forwards calls of `function(GLfloat x, GLfloat y, GLfloat z)` ->
    `function(glm::vec3(x, y, z)` which cuts down on boilerplate. This makes
    for cleaner and clearer code. So far, the GLTriangle and GLCap extend
    GLTransformable, but in the future I'll add it to EVERY shape!

    That way I can do things like:

        GLCube cube(t1, t2);
        cube.translate(glm::vec3(x, y, z)
            .rotate(M_PI / 4, glm::vec(1, 1, 1));


Bugs: No bugs (except the torus issue).
