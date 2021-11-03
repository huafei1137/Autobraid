#include "Gate.hpp"

void printGate(std::ostream& out, const Gate& g) {
    if(isSingle(g)) out << g.name << ' ' << g.target << std::endl; // single qubit gate
    else out << g.name << ' ' << g.control << ' ' << g.target << std::endl; // two qubit gate
}
