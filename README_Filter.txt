Design Decisions:

All my filters inherit from Filter. The scaling and edge detection filters are pretty much standard. The blur filter uses the O(1) box blur method mentioned in the README. I also have a rotation filter and a Gaussian blur filter in the frequency domain. For the Gaussian blur, I run FFT on each channnel of the image, multiply the image in the frequency domain by a Gaussian (which is equivalent to convoluting in the spatial domain by a Gaussian with a kernel over the whole image), and then run IFFT on each channel of the image.
