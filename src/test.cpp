#include <iostream>
#include <cassert>
#include <forward_list>
#include <stack>

#include "setutils.hpp"
#include "config.hpp"
#include "CircuitDAG.hpp"
#include "Matrix.hpp"
#include "Graph.hpp"
#include "ActiveGate.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    // parse command line options and configure environment
    auto env = parse(argc, argv);

    int numCycles = 0;

    // build circuit
    CircuitDAG circuit (argv[1]);
    circuit.build();

    cout << "num qubits used: " << circuit.numLogicalQubits() << endl;
    cout << "num gates: " << circuit.numGates() << endl;

    // placeholder matrix
    Matrix world (10);

    std::forward_list<ActiveGate> activeGates;
    while(!activeGates.empty() || !circuit.getExecutableGates().empty()) {
        for(int gateId : circuit.getExecutableGates()) {
            ActiveGate g = activateGate(circuit.get(gateId), std::vector<Point>(), world, env);
            activeGates.push_front(g);
            circuit.activateGate(gateId);
        }

        // increment cycle by one
        ++numCycles;

        // update active gate lifetimes and see if any have completed
        for(auto prev = activeGates.before_begin(), current = activeGates.begin();
            current != activeGates.end(); prev = current++
        ) {
            ++current->lifetime;
            if(isDone(*current)) {
                // resolve completed gate
                deactivateGate(*current, world);
                circuit.resolveGate(current->id);

                // remove from activeGates list
                activeGates.erase_after(prev);
                current = prev;
            }
        }
    }

    cout << "surface code distance: " << env.d << endl;
    cout << "critical path is " << numCycles << " cycles" << endl;
    cout << "critical path is " << numCycles*env.timePerCycle << " microseconds" << endl;
    return 0;
}