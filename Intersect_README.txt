 ___       _                          _
|_ _|_ __ | |_ ___ _ __ ___  ___  ___| |_
 | || '_ \| __/ _ \ '__/ __|/ _ \/ __| __|
 | || | | | ||  __/ |  \__ \  __/ (__| |_
|___|_| |_|\__\___|_|  |___/\___|\___|\__|
===============================================================================
Intersection I have an intersection manager (in the future I can add a KD-Tree
feature to it). Its constructor takes a vector of renderables. I can then
pass it a point and direction and it will determine the closest intersection
if any intersection exists.

I have a class called `IlluminateData` to hold an intersection point and a
pointer to a renderable. This probably should have been called intersection
data now that I think about it.

When I render an image, I shoot a ray through each pixel (or multiple times
through different parts of the pixel if I'm super sampling). I transform the
ray that I shot through each pixel into the object space of every object
in my scene. I check if they intersect and find the nearest intersection
to the point where I shot ray from. I then determine the normal vector
of the surface the light hit, and calculate each lights contribution at
the given pixel. I did all updates in a vector buffer of glm::vec3's
and then copied the buffer over to the canvas, when I finished.

Features:
    Supersampling:
        I take the parameter n, and create n^2 points to sample at. I sample
        at all the different points and then I average them.
    Multi-threading:
        I used c++ threads rather than QThreads because they're easier to use.
        I have a number of threads variable, and I split up the rows of the
        image so each thread is responsible for a given set.
