 ____
|  _ \ __ _ _   _
| |_) / _` | | | |
|  _ < (_| | |_| |
|_| \_\__,_|\__, |
            |___/
===============================================================================
Almost all the code is just a simple extension ontop of my intersect code.
I refactored my code to have an evaluate ray method. I can use this
to bounce the recursive rays without updating the image buffer.

The texture mapping is done by checking which type of primative we are
rendering, finding the face were are rending, and applying the correct
equation.

Note: I have a constant called MAX_RECURSION_DEPTH defined to be 6 in
the RayScene.h file.

Features:
    I added a KD Tree to my intersection manager. It seems like it works,
    but it's kind of slow. I think it's the fact that my bounding
    box intersection is slow. Or possibly a slight SAH bug

    I upgraded my multithreading to use a thread pool.
