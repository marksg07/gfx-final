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

Note: I have a constant called MAX_RECURSION_DEPTH defined to be 3 in
the RayScene.h file.

Features:
    I added a KD Tree to my intersection manager. The SAH is a little iffy,
    but everything else seems to work (but without a good SAH it doesn't
    really provide any speed up).

    I upgraded my multithreading to use a thread pool.
