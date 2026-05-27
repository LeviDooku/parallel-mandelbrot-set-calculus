# Parallel Mandelbrot Set Computation and Rendering

Generates Mandelbrot set images in PGM format. The goal is to compare sequential and parallel implementations using MPI.

## Overview

This is a project meant to learn the basic features of MPI (Message Passing Interface).

MPI is a programming standard for distributed memory systems. It is designed to be used in programs that can exploit the existence of multiple processors.

The calculation and renderization of the Mandelbrot set is a perfect example of this, because the operations are independent from each other. 

The Mandelbrot set is a fractal defined on the complex plane. For each point `c`, the program evaluates the recursive sequence:

$$
z_{n+1} = z_n^2 + c,\quad z_0 = 0
$$

If the sequence remains bounded, the point is considered part of the set. Since this cannot be checked indefinitely in practice, the program uses a maximum number of iterations and an escape threshold, commonly `|z| > 2`, to decide whether the sequence diverges.

Each point of the complex plane is mapped to a pixel in the output image. The final grayscale value depends on how quickly the sequence diverges, allowing the fractal structure to be visualized.

The project also explores the computational cost of generating the Mandelbrot set and compares different execution strategies, including parallel implementations.

## Technologies used

- C/C++
- MPI
- GCC
- Make

## How it works

The first step is solving the problem using a sequential implementation. This first program receives the maximum iterations as an input. Step by step:  

1. It reserves memory for the image.
2. For each pixel:
	a. It is transformed into a point in the complex plane.
	b. It determines if that point belongs to the Mandelbrot set.
3. It measures the time required for that calculation.
4. It saves the PGM image. The grayscale value is chosen depending on the number of iterations required by the calculation.

From this sequential program, we obtain the parallel one, parallelizing the calculation part using MPI. This new parallel program follows these steps:

1. It initizalizes the MPI environment.
2. Then determines the local workload for each process. This specific implementation uses a cyclical allocation strategy. 
This is because the workload is not evenly distributed, since there are regions of the set with more points than others. 
The allocation works as follows: given a process `p`, the rows assigned to it are: `p`, `p + size`, `p + 2*size`, ...
3. It reserves local memory depending on the rows.
4. It performs the local calculation.
5. It sends the data size to process 0 (master).
6. It calculates the offsets, since rows do not arrive in order.
7. It reserves global memory.
8. It collects the local data.
9. Finally, the global image gets reconstructed, using the same concept as in step 2.

## Installation

To download and compile the project:

```bash
git clone https://github.com/LeviDooku/parallel-mandelbrot-set-calculus.git
cd parallel-mandelbrot-set-calculus
make 

