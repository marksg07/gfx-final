 ____                 _     
| __ ) _ __ _   _ ___| |__  
|  _ \| '__| | | / __| '_ \ 
| |_) | |  | |_| \__ \ | | |
|____/|_|   \__,_|___/_| |_|
===============================================================================

Speical Brushes:
    I implemented a brush like the game snake. It kind of pushes the definition
    of brush a little, but I think it still counts, because it's basically a
    stamp brush with extra features. I really liked the ripple brush animation,
    but I wanted to do something unique.

    1. To add snake food, click (or click and drag) with the "Add Snake Food"
    brush.
    2. The add a snake, click with the "Add Snake" brush. The snakes will
    eat the food and grow as they eat. The color of the snake we be the color
    selected via the sliders.

Design Decisions:
    1. I added a `BrushManager` class to the Canvas2D this keeps track of
    the current brush, and will create new brushes when settings change, or
    return the current brush when settings are non-changing.

    2. The "fix alpha blending" behavior was meant to mirror the demo.
    The goal being to create a blended layer of paint, that would never
    exceede the maximum brush-to-canvas paint ratio as well as appear
    uniform with no ugly lines of intensity around the path of the user's mouse.
    I decided to calculate the `ideal` pixel (what would the pixel color be if
    I clicked on it, a.k.a. blended with radius 0). I then take that `ideal`
    pixel and blend it with what's on the canvas (except in the case of the
    ConstantBrush, in which case the pixel is simply replaced as the mask is
    either 0 or 1). This has the effect of limiting the max
    brush-to-old-canvas ratio and prevent the issues of alpha blending
    highlighted in the handout.
    
    3. I added a `SnakeManager` which tracks the location of the food on the
    canvas, as well as the a copy of the underlying canvas, which makes it
    easier to animate the snakes.
    
    4. The Snakes were the trickiest brush, as the snakes can move
    non-destructively over the canvas. It was implemented a QTimer in the 
    SnakeManager, which allows multiple snakes to move at the "same" time.
