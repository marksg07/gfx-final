 ____                            _               
/ ___|  ___ ___ _ __   _____   _(_) _____      __
\___ \ / __/ _ \ '_ \ / _ \ \ / / |/ _ \ \ /\ / /
 ___) | (_|  __/ | | |  __/\ V /| |  __/\ V  V / 
|____/ \___\___|_| |_|\___| \_/ |_|\___| \_/\_/  
===============================================================================
I created a `CS123Renderable` class that contains a transformation matrix
along with a CS123ScenePrimative. When I transform the tree into a list, I
make a list of CS123Renderables.

I have a recursive function that creates a CS123Renderable for ever primative
in the node and then calls itself on every child node contained in the current
node. The recursive function also has a parameter to hold the current
transformation matrix. I accumulate all the transformation matrices by
multiplying the previous matrix passed to the function with all the
transformations contained within the the node. I attach this matrix to all the
primatives in the current node, and pass it on to the next recursive call.



Extra Features:

I always only use one instance of each shape, which prevents me from having to
re-tesselate.

I have a simple system that calculates the tesslation parameter. The more
instances of the shape in the scene the lower the tesselation parameters. For
instance if there is only one sphere was can afford to tesselate it with 100,
100 as our parameters but if there are 1000, we might only be able to afford
20, 20. The system is very basic, but seems to work fairly well but could
easily be improved with more empirical testing.

I have textures working, but none of my shapes have proper texture coordinates.
So I've set the texture coordinates to random values between 0 and 1. So the
functionality for textures is there, I just need to add the correct u and v
values to the vertices in my shape.
