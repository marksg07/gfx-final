Design decisions:
Implicit shape intersections and othe implicit shape utils are separate from Shape.h and .cpp, rather they are in intersect/implicitshape.cpp. This is because the logic is completely different. For the rendering itself, I basically followed the algorithm in the slides exactly. I also implemented supersampling, multithreading, kd-trees, and adaptive supersampling, as described below. I implemented point and directional lights. Finally, I copy the scene graph (including primitives, lights, and globals) from the sceneview scene when I create a new rayscene.

Supersampling:
NOTE: Supersampling will only happen if the supersampling checkbox is enabled AND the antialiasing checkbox is DISABLED. This is intended to make it easier to set up adaptive supersampling, as described below.
Does what it says on the box; shoots multiple rays per pixel. I made the grid of samples to be centered around the center of a pixel. I made the grid so that the border between it and the edges of the pixel is 1/(2*samples) so that the grid is continuous over the pixels (same sized gaps between every sample over all the pixels).

Multithreading:
Also pretty simple. If checkbox is enabled, creates 16 threads, each one of which processes a certain set of rows. These row sets are calculated in two different ways, but the second way only comes into play with antialiasing enabled, so I'll ignore it for now. The first way is to split up the rows evenly by dividing the height by the number of threads, and then giving the remainder of the rows (m_height % nthreads) to the first (1 + m_height % nthreads) threads.

kd-trees:
If checkbox is enabled, use kd-tree. On render, builds a kd-tree in O(n(logn)^2) (i.e. the fast method) using AABBs for each shape. Optimally, this would happen only once per scene, i.e. only on the first render of a new scene, but I couldn't figure out a way to do that without building a kd-tree every time you loaded a scene (even when you didn't want to raytrace). Then, for each ray, the kd-tree is traversed with early termination to find the closest intersection.

Adaptive supersampling:
If antialiasing checkbox is enabled, enabled. Steps:
1. For each channel, edge detect scene, but don't bound end result to max 255. Then, for each pixel, take R^2 + G^2 + B^2 to get the final edge detection.
2. Sort this list of edge weights. Ignore anything with weight<=200 for following calculations, as it should never be resampled. Note: 200 weight is very low, considering we use a sensitivity of 1 and we don't bound the end result, and also that everything is squared.
3. With n = numSuperSamples, find top (1-n)th, (1-n/4)th, (1-n/16)th, and (1-n/64)th percentile of edge weights. Note that num samples is not actually used for num samples here. In fact, a rough equivalency is that num samples = 100 ~ supersampling with num samples = 2. num samples = 400 ~ supersampling with num samples = 4. num samples = 1600 ~ supersampling with num samples = 8. num samples = 6400 ~ supersampling with num samples = 16. I do it this way to allow more choice.
4. Rerender pixels conditionally: If pixel's edge weight is not in the top (1-n)th percentile, don't rerender. If pixel's edge weight is between (1-n)th and (1-n/4)th percentiles, rerender with supersampling = 2. If pixel's edge weight is between (1-n/4)th and (1-n/16)th percentiles, rerender with supersampling = 4. If weight between (1-n/16)th and (1-n/64)th, rerender with SS = 8. Finally, if weight above (1-n/64)th percentile, rerender with SS = 16. For each of these renders, if multithreading is enabled, we split up the rows by, rather than pure number of rows, number of rerenders necessary per row. This means each thread will be doing approximately the same number of rerenders, which is good because that means they will be close in the amount of time they take.

With num samples = 100, all pixels with edge weight != 0 are rerendered: approx 75% with SS=2, 19% with SS=4, 4.5% with SS=8, 1.5% with SS=16. The amount of time this takes is approx 3*(cost of SS=2)*(num pixels with edge weight != 0). In a scene full of intersecting objects, this is likely to take around 3x the time of SS=2, while in a more empty scene, this would probably take much less time than SS=2. Overall, this method generally does much better than SS for quality/time. There are occasionally weird inconsistencies in which, for example, the edge of a shape is rendered partially with one level of SS and partially with a differrent level, which sometimes looks bad, but this can usually be solved by increasing the num samples.

Bugs: Rendering with the eye inside an object sometimes produces different results than the demo. I was somewhat unsure of what to do in this case, so not 100% sure it's a bug because the behavior is unspecified.

Note: With multithreading, kd-trees, and no supersampling or adapt. SS, with default sized window, on some dept machine,
- rec. spheres 7 builds kd-tree in 0.4 secs, renders in 1.0 secs
- rec. spheres 8 builds kd-tree in 1.9 secs, renders in 1.5 secs
- rec. spheres 9 builds kd-tree in 9.9 secs, renders in 2.7 secs
on my home computer:
- rec. spheres 10 builds kd-tree in 87.7 secs, renders in 4.5 secs
on dept machine:
- rec. spheres 10 builds kd-tree in >15 min, I killed it. I assume this is due to an OOM causing a page file to be used as memory or something like that, slowing everything to a crawl.

Making the window larger slows this down, but only the rendering time.
