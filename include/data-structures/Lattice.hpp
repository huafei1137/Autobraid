#ifndef LATTICE_HPP
#define LATTICE_HPP

#include "Gate.hpp"
#include "Point.hpp"
#include <vector>

// represents grid of cells and how logical qubits are arranged in that grid
class Lattice {
public:
    // assigns a default mapping (identity map)
    Lattice(int length);

    // sets a logical to physical qubit mapping
    void setMapping(std::vector<int> mp);

    int latticeLength() const { return length; }
    Cell getLatticePosition(int logQubit) const; // convert 1d -> 2d coordinate
    int getPhysQubitNumber(const Cell& c) const; // convert 2d -> 1d coordinate
    int getArea(const Gate& g) const;
    bool checkOverlap(const Gate& g1, const Gate& g2) const;
    void swapLogicalQubit(int log1, int log2); // alter log->phys mapping

private:
    int length;
    std::vector<int> log2phys;
};

#endif