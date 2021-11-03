#ifndef ACTIVE_GATE_HPP
#define ACTIVE_GATE_HPP

#include "Gate.hpp"
#include "Point.hpp"
#include "Matrix.hpp"
#include "config.hpp"

#include <vector>

struct ActiveGate : public Gate {
    std::vector<Point> braidPath; // lattice resources occupied by this gate
    int cycleCost; // how long gate will need to execute

    int lifetime = 0; // how long gate has been active so far
};

inline bool isDone(const ActiveGate& g) { return g.lifetime >= g.cycleCost; }

// occupy lattice resources and active gate (modifies world)
// d is the surface code distance
ActiveGate activateGate(const Gate& g, std::vector<Point> resources, Matrix& world, const Environment& env);

// free up resources taken by a gate that has completed
void deactivateGate(const ActiveGate& g, Matrix& world);

// d is the surface code distance
int getCost(const std::string& name, const Environment& env);

#endif