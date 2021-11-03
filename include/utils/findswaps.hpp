#ifndef FIND_SWAPS_HPP
#define FIND_SWAPS_HPP

#include <vector>
#include <stack>
#include "Matrix.hpp"
#include "Gate.hpp"
#include "Lattice.hpp"

// returns the number of SWAP gates scheduled
// takes grid by reference and alters its logical->physical qubit mapping
// frontLayer should only consist of CX gates/two-qubit gates
// int& res is assigned the number of vertices used by scheduled swaps (for resource utilization)
// TODO: make a better interface for this?
int findSwaps(std::vector<Gate> frontLayer, Lattice& grid, const Matrix& world, int& res);

#endif