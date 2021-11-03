#include "CircuitDAG.hpp"

#include <cassert>
#include "QASMparser.h"

void CircuitDAG::build() {
    QASMparser parser (fileName);
    parser.Parse();

    gateList.reserve(parser.getNgates());

    // track dependencies for each qubit
    std::vector<int> lastGatePerQubit (parser.getNqubits(), -1);
    std::vector<bool> isControlQubit (parser.getNqubits());
    auto layers = parser.getLayers();
    for(int i = 0; i < layers.size(); i++) {
        auto& layer = layers[i];
        for(int j = 0; j < layer.size(); j++) {
            auto& gate = layer[j];
            GateNode node (gateList.size(), gate.type, gate.control, gate.target);

            assert(gate.control != gate.target);
            assert(gate.target != -1);

            numQubits = std::max(numQubits, gate.control+1);
            numQubits = std::max(numQubits, gate.target+1);

            // use these to determine dependencies for this gate and update DAG accordingly
            int controlDepId = -1;
            int targetDepId = -1;

            // use lambda expr to reduce code duplication
            auto parseDependency = [&](int qubit, bool isControl) -> int {
                int dependencyId = lastGatePerQubit[qubit];
                if(dependencyId != -1) {
                    node.numDependencies++;
                    if(isControlQubit[qubit])
                        this->gateList[dependencyId].controlChildId = node.g.id;
                    else
                        this->gateList[dependencyId].targetChildId = node.g.id;
                }
                lastGatePerQubit[qubit] = node.g.id;
                isControlQubit[qubit] = isControl;
                return dependencyId;
            };

            // only execute this stmt for 2-qubit gates
            if(gate.control != -1) controlDepId = parseDependency(gate.control, true);

            targetDepId = parseDependency(gate.target, false);

            // check if gate has no dependencies
            if(controlDepId == -1 && targetDepId == -1)
                canExecute.insert(node.g.id);
            
            gateList.push_back(std::move(node));
        }
    }

    assert(numQubits <= parser.getNqubits());
}

void CircuitDAG::activateGate(int id) {
    int removed = canExecute.erase(id);
    assert(removed == 1); // make sure gate was actually executable
}

void CircuitDAG::resolveGate(int id) {
    GateNode& node = gateList[id];
    
    // a gate should not be resolved twice in a working algorithm
    assert(!node.finished);
    node.finished = true;
    canExecute.erase(id);

    int childIdList[2] = { node.controlChildId, node.targetChildId };
    for(int childId : childIdList) {
        if(childId != -1) {
        GateNode& child = gateList[childId];
        child.numParentsFinished++;
        if(child.numParentsFinished == child.numDependencies)
            canExecute.insert(child.g.id);
        }
    }
}

const Gate& CircuitDAG::get(int id) const {
    return gateList[id].g;
}

void CircuitDAG::reset() {
    canExecute.clear();
    for(GateNode& node : gateList) {
        node.numParentsFinished = 0;
        node.finished = false;
        if(node.numDependencies == 0) canExecute.insert(node.g.id);
    }
}