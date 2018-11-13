 _____ _ _ _
|  ___(_) | |_ ___ _ __
| |_  | | | __/ _ \ '__|
|  _| | | | ||  __/ |
|_|   |_|_|\__\___|_|
===============================================================================
Design Choices:
    I have a Kernel Context class. This stores image data to which we can
    apply Kernels. The Kernels are in a kernel class, which stores the mask
    and the dimensions.
    I have a FilterUtil's class with lots of helpers.

Filters:
    FilterEdge:
        This uses a seperable kernel.

    FilterBlur:
        I use a gaussian blur function. I calculate the gaussian in one dimension
        and the use it for the other dimension. This is because the kernels are
        seperable.

        Note: The blur is gaussian with a mean of 0 and std. dev of 1 / 3.
        This works for everything, but radius 1 doesn't really blur that much.

        This is because the kernel for r = 1 is: [0.0108675 0.978265 0.0108675], so
        you get 97% of the pixel you started with. This means you'd have to blur hundreds
        of times to get any significant difference.
        Note that for r = 2 the kernel is: [0.00664604 0.194226 0.598257 0.194226 0.00664604],
        which will blur immediately.

    FilterScale: This uses the back mapping from the algo.

    FilterRotate: This uses glm to rotate a vector to each corner around the z axis.
                  The maximum difference between x and y form the height and width of
                  the new image. You then iterate over every pixel in the new image.
                  You take the vector from the center to the pixel and unrotate it
                  to map to a point in the old image (like scaling). Unfortunately this
                  is unlikely to be an integer. If the value is in bounds you interpolate it
                  between the the nearest 4 pixels (just like the interpolation in the terrain lab).
                  If it's out of bounds it's black.
                  Bugs: Repeated rotations can blur. Rotations at small angles lead to
                  jagged edges.

    SpecialFilter#1 (FilterSharpen):
        This is a sharpen filter. It uses a separable kernel and a 1x1 kernel.
        Note: this uses the blur radius parameter. This filter is from the textbook section 19.6

    SpecialFilter#2 (FilterUnsharp):
        This is an unsharp mask filter. It sharpens but it uses a gaussian distribution.
        Note: this uses the blur radius parameter.
