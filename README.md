# Parallel Mandelbrot Set Calculus and Render

## Overview

This is a project meant to lern the basic features of MPI (message passing interface). 

MPI is a programming standard for distributed memory systems. Is designed to be used in programs that could explode the existence of multiple processor.

The calculus and renderization of de Mandelbrot set is a perfect example of that, because the operations are independent from each other. 

The Mandelbrot set is a fractal defined on the complex plane. For each point `c`, the program evaluates the recursive sequence:

$$
z_{n+1} = z_n^2 + c,\quad z_0 = 0
$$

If the sequence remains bounded, the point is considered part of the set. Since this cannot be checked indefinitely in practice, the program uses a maximum number of iterations and an escape threshold, commonly `|z| > 2`, to decide whether the sequence diverges.

Each point of the complex plane is mapped to a pixel in the output image. The final grayscale value depends on how quickly the sequence diverges, allowing the fractal structure to be visualized.

The project also explores the computational cost of generating the Mandelbrot set and compares different execution strategies, including parallel implementations.