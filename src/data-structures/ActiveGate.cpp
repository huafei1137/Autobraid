#include "ActiveGate.hpp"

#include <cassert>

ActiveGate activateGate(const Gate& g, std::vector<Point> resources, Matrix& world, const Environment& env) {
    for(const Point& p : resources) {
        assert(world[p] == 0); // make sure resource is not already taken
        world[p] = 1;
    }
    return {g, std::move(resources), getCost(g.name, env)};
}

void deactivateGate(const ActiveGate& g, Matrix& world) {
    for(const Point& p : g.braidPath) world[p] = 0;
}

int getCost(const std::string& name, const Environment& env) {
    if(name == "cx") {
        return (env.isQFT) ? // NOTE: THIS CHECK IS A QUICK HACK AND NOT ACTUALLY A GOOD IDEA
            2 + 2*(2*env.d + 3) : // general controlled rotation for qft circuits
            2*env.d + 3; // standard CX gate
    }
    if(name == "u3(1.570796, 0.000000, 3.141593)" || name == "h") return env.d + 10; // Hadamard gate
    if(name == "swap") return 3*(2*env.d + 3); // SWAP gate (aka 3 CX gates)
    return 1; // generic single-qubit gate
}