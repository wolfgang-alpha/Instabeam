# Instabeam

**Unstable development channel** — this is the actively developed successor of [TrussCalculator](https://github.com/wolfgang-alpha/TrussCalculator). Expect breaking changes, incomplete features, and rough edges. For the stable version, use TrussCalculator.

A 2D structural analysis application built with C++ and Qt. It provides an interactive graphical interface for defining structural systems and solves them using the Finite Element Method (FEM) with the Eigen linear algebra library.

Originally written by [bernhard1536](https://github.com/bernhard1536) as TrussCalculator (ca. 2018-2019). Forked and extended as Instabeam.

## What's new compared to TrussCalculator

- Curved beam elements (Hermite arc elements derived from complementary energy)
- Deformation visualization for curved beams
- Mixed systems: straight rods and curved beams can share nodes

## Features

- Interactive graphical editor for placing nodes, rods, curved beams, bearings, and forces
- FEM solver: assembles element/global stiffness matrices and solves K·U = F
- Visualization of inner forces with color-mapped elements
- Deformed system overlay
- Support types: locating bearing, floating bearing, fixed clamping
- Rod, rope (cable), and curved beam elements
- Dimension and label annotations
- Save/load projects as JSON
- Print support

## Element stiffness matrix derivations

Symbolic derivations for all new element types are in `new-element-derivations/`:

- **`hermite_curved_beam.py`** — Hermite curved beam element. Derives the 6x6 element stiffness matrix from complementary energy (Parkus p.247). DOFs: radial translation, rotation, and tangential translation at each node.
- **`cst_continuum_element.py`** — Constant Strain Triangle (CST) continuum element. Derives the 6x6 element stiffness matrix from strain energy for a triangular 2D element with two translational DOFs per node.

## Building

### Prerequisites

- C++14 compiler (GCC, Clang, or MSVC)
- Qt 5 (tested with Qt 5.15; Qt 5.6+ should work)
  - Modules: Core, GUI, Widgets, PrintSupport
- qmake
- Eigen is bundled in `libs/Eigen/` (no separate install needed)

### Linux

Install the Qt 5 development packages:

```bash
# Debian / Ubuntu
sudo apt install build-essential qt5-qmake qtbase5-dev libqt5printsupport5

# Fedora
sudo dnf install gcc-c++ make qt5-qtbase-devel

# Arch
sudo pacman -S base-devel qt5-base
```

Build:

```bash
mkdir -p build && cd build
qmake ../TrussCalculator.pro
make -j$(nproc)
```

Run:

```bash
./build/TrussCalculator
```

### Windows

#### Option A: Qt Creator (recommended)

1. Install [Qt](https://www.qt.io/download-qt-installer) with Qt 5.15 and MinGW or MSVC kit
2. Open `TrussCalculator.pro` in Qt Creator
3. Configure the project with your kit and click Build/Run

#### Option B: Command line with MSVC

1. Install Qt 5 and add `qmake` to your PATH
2. Open a Developer Command Prompt for Visual Studio:

```cmd
mkdir build && cd build
qmake ..\TrussCalculator.pro -spec win32-msvc
nmake
```

#### Option C: Command line with MinGW

1. Install Qt 5 with MinGW and add both `qmake` and MinGW `bin/` to your PATH:

```cmd
mkdir build && cd build
qmake ..\TrussCalculator.pro -spec win32-g++
mingw32-make -j%NUMBER_OF_PROCESSORS%
```

## License

See the Eigen library license files in `libs/Eigen/` for Eigen's licensing (MPL2 / BSD / LGPL).
