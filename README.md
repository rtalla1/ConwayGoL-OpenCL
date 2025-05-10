# Conway's Game of Life

An OpenCL implementation of Conway's Game of Life, harnessing GPU acceleration for fast simulations of cellular automata.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Usage](#usage)
- [Maps](#maps)
- [Examples](#examples)
- [Contributing](#contributing)

## Overview
Conway's Game of Life is a zero-player game that simulates the life and death of cells on a two-dimensional grid based on a set of simple rules. This project implements the game using OpenCL to leverage the parallel processing capabilities of modern GPUs, CPUs, FPGAs, and more to deliver high-performance simulations.

## Features
- **GPU-Accelerated**: Utilizes OpenCL for parallel cell updates.
- **Configurable Grid**: Supports custom map sizes and initial configurations.
- **Multiple Maps**: Includes example maps in the `maps/` directory.
- **Real-Time Visualization**: Render the simulation visually (via optional viewer).
- **Cross-Platform**: Compatible with Linux, macOS, and Windows (with an OpenCL SDK).

## Prerequisites
- A C++ compiler with C++11 support (e.g., `g++`, `clang++`).
- OpenCL SDK and drivers installed for your platform.
- CMake (optional).

## Building
```bash
clang++ main.cpp -framework OpenCL -o conway
```
This produces `conway`. You can then run it directly:
```bash
./conway <map_file> <num_generations> <delay_microseconds>
```

## Usage
Run the executable with the required arguments:

```bash
./conway <map_file> <num_generations> <delay_microseconds>
# or, if you used the default output name:
./a.out <map_file> <num_generations> <delay_microseconds>
```

Example:

```bash
./conway maps/m1.txt 500 25000
# or
./a.out maps/m3.txt 250 50000
```

## Maps
Sample map files are located in the `maps/` directory:
- `m1.txt`: Random pattern.
- `m2.txt`: Columns.
- `m3.txt`: Smaller map with diagonal patterns.

Map files are simple text grids where `.` denotes a dead cell and `X` denotes a live cell. Feel free to contribute with some interesting maps!

## Example
![Conway’s Game of Life demo](https://i.imgur.com/qEvWrk1.gif)

## Contributing
Contributions are welcome! Please open issues or submit pull requests for bug fixes, new features, or improvements.