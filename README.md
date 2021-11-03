# autobraid

## Dependencies

Building this software requires `CMake 3.x.x` and `metis 5.1.0` (or a compatible version).

The software also uses the third party header-only library `cxxopts` to enable command line argument parsing
([link to repository](https://github.com/jarro2783/cxxopts)),
as well as a `.qasm` file parser published by the *JKU Institute for Integrated Circuits*
([link to website](https://iic.jku.at/eda/research/quantum_simulation/)).
The required files for these dependencies are already included in the repository,
so no installation is needed for them.

## Building and Usage

In the root directory (containing `CMakeLists.txt`), type the following commands:

```
mkdir build && cd build
cmake ../
make
```

If `metis` or the header file `metis.h` is installed in a non-standard location,
you may need to edit the following lines of `CMakeLists.txt`
so that `cmake` can find the library:

```cmake
find_library(LIBMETIS metis)
set(METIS_INCLUDE_DIR /usr/local/include)
```

This will compile the two executables `autobraid` and `critpath` in the `build` directory.
`autobraid` is the main executable which implements stack-based pathfinding and additional optimizations,
while `critpath` is used to determine the critical path (ie best possible execution time) of a quantum circuit
using surface code error correction.

Type `/path/to/autobraid` or `/path/to/critpath` without additional arguments to see their usage.
Both executables take the same options, however `critpath` will ignore some `autobraid` algorithm-specific options
(such as `--init-place` or `--swap-opt`).

### Special Note on `--qft`

The `--qft` option should be used when the input is a qft or qft-type circuit.
Currently we do not support automatic detection of such circuits.
Since the `.qasm` file parser used does not recognize general controlled rotations,
any controlled rotations should be specified as standard `cx` gates in the `.qasm` file,
and they will be assigned the correct cycle cost for general controlled rotations by the algorithm.

Enabling the `--qft` option will also cause `autobraid` to compute the cycle cost of using
Maslov's swap-based solution for qft circuits on LNN-type architectures.
