#ifndef GATE_HPP
#define GATE_HPP

#include <string>
#include <iostream>

struct Gate {
    int id; // unique identifier in DAG

    std::string name;
    int control;
    int target;
};

inline bool isSingle(const Gate& g) { return g.control == -1; }

void printGate(std::ostream& out, const Gate& g);

#endif