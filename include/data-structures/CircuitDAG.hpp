#ifndef CIRCUIT_DAG_HPP
#define CIRCUIT_DAG_HPP

#include <string>
#include <vector>

#include "setutils.hpp"
#include "Gate.hpp"

namespace detail {
    struct GateNode {
        Gate g;
        int controlChildId = -1, targetChildId = -1;
        int numDependencies = 0; // number of parent gates in DAG
        int numParentsFinished = 0;
        bool finished = false; // used for asserts
        
        GateNode(int id, std::string n, int c, int t) : g{id, n, c, t} {}
    };
}

// NOTE: no bounds checking is performed on GateNode ids!
class CircuitDAG {
public:
    using GateNode = detail::GateNode;

    CircuitDAG(std::string fname) : fileName(fname) {}

    const Gate& get(int id) const;
    std::unordered_set<int> getExecutableGates() const { return canExecute; } // returns snapshot of CX layer

    int numLogicalQubits() const { return numQubits; }
    int numGates() const { return gateList.size(); }

    void build();
    void activateGate(int id); // mark gate as activated
    void resolveGate(int id); // mark gate as completed and update canExecute
    void reset();

private:
    std::string fileName;
    int numQubits = 0;
    std::unordered_set<int> canExecute; // holds layer of gates which can execute concurrently
    std::vector<GateNode> gateList;
};

#endif