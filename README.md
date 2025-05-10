#o get some experience with OpenCL, I decided to implement this game because of its interesting visual and mathematical nature.
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
- [License](#license)

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
Build the project using CMake:

```bash
git clone https://github.com/rtalla1/conwayGoL-OpenCL.git
cd conwayGoL-OpenCL
mkdir build && cd build
cmake ..
make
```

Or, without CMake:

```bash
clang++ main.cpp -framework OpenCL -o c
```

## Usage
Run the executable with optional arguments:

```bash
./conway [options]
```

### Options
- `-m, --map <file>`: Path to a map file (default: `maps/m1.txt`).
- `-s, --steps <n>`: Number of generations to simulate (default: infinite loop).
- `-d, --delay <ms>`: Delay between frames in milliseconds (default: 100).

Example:

```bash
./conway --map maps/m2.txt --steps 1000 --delay 50
```

## Maps
Sample map files are located in the `maps/` directory:
- `m1.txt`: Glider pattern.
- `m2.txt`: Gosper glider gun.
- `m3.txt`: Random initial state.

Map files are simple text grids where `.` denotes a dead cell and `*` denotes a live cell.

## Examples
Include screenshots or link to demo videos here.

## Contributing
Contributions are welcome! Please open issues or submit pull requests for bug fixes, new features, or improvements.

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.