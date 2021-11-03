#include <iostream>
#include <chrono>
#include <cassert>
#include <forward_list>
#include <stack>
#include "setutils.hpp"

#include "config.hpp"
#include "partition.hpp"
#include "pathfind.hpp"
#include "graphutils.hpp"
#include "findswaps.hpp"

#include "Graph.hpp"
#include "CircuitDAG.hpp"
#include "ActiveGate.hpp"
#include "Matrix.hpp"
#include "Lattice.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    // parse command line options and configure environment
    auto env = parse(argc, argv);

    // debug/output information
    int numCycles = 0;
    int placementOptimizerCount = 0; // how many times placement optimizer was used
    int consecutiveSWAPLayers = 0; // how many times placement optimizer was used in a row
    unsigned long cumulativeVert = 0; // cumulative braiding resource usage
    unsigned long currentVert = 0; // braiding resource usage for the current cycle
    auto startTime = std::chrono::high_resolution_clock::now();

    // build circuit
    cerr << "Building circuit... ";
    CircuitDAG circuit (env.fileName);
    circuit.build();
    cerr << "Done." << endl;

    // build 2d grid
    int length = 0; // get smallest square that can hold all the qubits
    while(length*length < circuit.numLogicalQubits()) ++length;
    Lattice grid (length);
    Matrix world (grid.latticeLength() + 1); // used in A* search

    // initial placement
    if(env.doInitPlacement) {
        // create logical qubit coupling graph
        cerr << "Building coupling graph... ";
        Graph coupling;
        for(int i = 0; i < circuit.numLogicalQubits(); ++i) coupling.addVertex(i);
        auto frontLayer = circuit.getExecutableGates();
        while(!frontLayer.empty()) {
            for(int gateId : frontLayer) {
                const Gate& g = circuit.get(gateId);
                if(!isSingle(g)) coupling.addEdge(g.control, g.target);
                circuit.resolveGate(gateId);
            }
            frontLayer = circuit.getExecutableGates();
        }
        circuit.reset(); // reset circuit so it can be parsed again in the main loop
        cerr << "Done." << endl;

        cerr << "Performing initial placement... ";
        // determine which initial placement method to use
        bool isLineGraph = false;
        int vertexId = getMaxDegreeVertexId(coupling);
        std::vector<std::vector<int>> components;
        if(coupling.getVertex(vertexId).degree() <= 2) {
            components = getComponentsInOrder(coupling);
            if(components.size() == 1) isLineGraph = true;
        }

        std::vector<int> mapping;
        if(isLineGraph) { // special placement approach when coupling graph is a line graph
            cerr << "choosing line graph layout approach... ";
            auto verticesInOrder = components[0];
            mapping.resize(circuit.numLogicalQubits());
            for(int i = 0; i < verticesInOrder.size(); ++i) { // arrange qubits in snake/zigzag pattern
                int posy = i/grid.latticeLength();
                int posx = (posy%2 == 0) ?
                    i%grid.latticeLength() :
                    grid.latticeLength() - 1 - i%grid.latticeLength();
                mapping[verticesInOrder[i]] = grid.getPhysQubitNumber({posx, posy});
            }
        } else { // perform graph partition and getting initial mapping
            cerr << "choosing gpmetis approach... ";
            initMetisOptions();
            mapping = initialPlacement(coupling, grid);
        }
        grid.setMapping(std::move(mapping));
        cerr << "Done." << endl;
    }

    // auxiliary data structures
    std::forward_list<ActiveGate> activeGates;
    std::unordered_set<int> activeGateIds; // store active gates for quick lookup
    std::unordered_set<int> executableGates = circuit.getExecutableGates();
    std::stack<int> CXstack; // used to hold removed CX gates in the loop

    // lambdas used for the interference graph
    auto compareArea = [&](const Graph::Vertex& v1, const Graph::Vertex& v2) -> bool {
        return grid.getArea(circuit.get(v1.id)) > grid.getArea(circuit.get(v2.id));
    };
    auto isNotActive = [&](const Graph::Vertex& v) -> bool {
        return !contains(activeGateIds, v.id);
    };

    // enter loop
    while(!activeGates.empty() || !executableGates.empty()) {
        cerr << "\rPerforming braiding (cycle " << numCycles << ")... ";

        // BEGIN STACK-BASED SCHEDULING SECTION
        std::vector<Gate> CXgates; // holds the relevant CX gates for which to build the interference graph
        std::forward_list<ActiveGate> pendingGates; // holds gates that are able to activate this cycle
        int numScheduledCX = 0;

        // separate single-qubit and two-qubit gates amongst currently executing gates
        for(const ActiveGate& g : activeGates) {
            if(!isSingle(g)) {
                CXgates.push_back(g);
                ++numScheduledCX; // currently active gates count as scheduled gates
            }
        }

        // separate single-qubit and two-qubit gates in executable layer
        for(int gateId : executableGates) {
            const Gate& g = circuit.get(gateId);
            if(isSingle(g)) { // single-qubit gates can possibly be scheduled immediately
                pendingGates.push_front(activateGate(g, std::vector<Point>(), world, env));
            } else {
                CXgates.push_back(g);
            }
        }

        // construct interference graph
        Graph interferenceGraph = buildInterferenceGraph(CXgates, grid);

        // remove highest interference inactive CX gates from graph and push onto the stack
        // until either max degree of inactive gates <= 2 or there are no more inactive gates
        int interferer = getMaxDegreeVertexId(interferenceGraph, isNotActive, compareArea);
        while(interferer != -1 && interferenceGraph.getVertex(interferer).degree() >= 3) {
            CXstack.push(interferer);
            interferenceGraph.deleteVertex(interferer);
            interferer = getMaxDegreeVertexId(interferenceGraph, isNotActive, compareArea);
        }

        // remove all active gates from interference graph to enable component finding
        {
            std::vector<int> gatesToRemove;
            for(const Graph::Vertex& v : interferenceGraph) {
                if(isNotActive(v)) continue;
                else gatesToRemove.push_back(v.id);
            }
            for(int id : gatesToRemove) interferenceGraph.deleteVertex(id);
        }
        
        // begin finding braid paths for inactive CX gates remaining in the interference graph
        auto components = getComponentsInOrder(interferenceGraph);
        for(auto component : components) {
            for(int id : component) {
                const Gate& g = circuit.get(id);
                std::vector<Point> path = braid(g, grid, world);
                if(!path.empty()) {
                    pendingGates.push_front(activateGate(g, std::move(path), world, env));
                    ++numScheduledCX;
                }
            }
        }

        // attempt to find braid paths for CX gates in the stack
        while(!CXstack.empty()) {
            int id = CXstack.top();
            CXstack.pop();
            const Gate& g = circuit.get(id);
            std::vector<Point> path = braid(g, grid, world);
            if(!path.empty()) {
                pendingGates.push_front(activateGate(g, std::move(path), world, env));
                ++numScheduledCX;
            }
        }

        // determine if SWAP-based placement optimizer should be triggered
        double ratio = static_cast<double>(numScheduledCX) / CXgates.size();
        if(env.doSwapOptimizer) {
            if(ratio <= env.swapThreshold && consecutiveSWAPLayers < env.maxConsecutiveSWAPLayers) {
                ++placementOptimizerCount;
                ++consecutiveSWAPLayers;
                // assert(consecutiveSWAPLayers <= env.maxConsecutiveSWAPLayers);

                // wait for currently active gates to finish and reset everything
                int numTicks = 0;
                for(const ActiveGate& g : activeGates) {
                    cumulativeVert += (g.cycleCost - g.lifetime)*g.braidPath.size(); // debug
                    numTicks = std::max(numTicks, g.cycleCost - g.lifetime);
                    circuit.resolveGate(g.id);
                }
                numCycles += numTicks;
                clear(world);
                activeGateIds.clear();
                activeGates.clear();
                pendingGates.clear(); // do not officially active pending gates
                currentVert = 0;

                // determine front layer of CX gates and schedule swaps
                auto frontLayer = circuit.getExecutableGates();
                std::vector<Gate> CXfrontLayer;
                for(int id : frontLayer) {
                    const Gate& g = circuit.get(id);
                    if(!isSingle(g)) CXfrontLayer.push_back(g);
                }
                int numSWAPVertices;
                int numSwaps = findSwaps(CXfrontLayer, grid, world, numSWAPVertices);

                if(numSwaps == 0) {
                    cerr << "activated placement optimizer but 0 SWAPs inserted." << endl; // debug
                } else {
                    numCycles += getCost("swap", env); // SWAPs cost 3 CX gates
                    cumulativeVert += numSWAPVertices*getCost("swap", env); // debug
                }

                continue; // do not enter cycle update section
            } else {
                consecutiveSWAPLayers = 0;
            }
        }

        // else, officially activate pending gates
        for(auto root = pendingGates.before_begin(); !pendingGates.empty();) {
            auto front = root; ++front; // get front of list
            currentVert += front->braidPath.size(); // debug
            circuit.activateGate(front->id); // remove gate from executable layer in DAG
            activeGateIds.insert(front->id); // add id to lookup table
            activeGates.splice_after(activeGates.before_begin(), pendingGates, root); // add gate to activeGate list
        }
        // END STACK-BASED SCHEDULING SECTION
        
        // BEGIN CYCLE UPDATE SECTION
        assert(!activeGates.empty());
        // determine how many cycles to tick forward (minimum to make an active gate finish)
        int numTicks = std::numeric_limits<int>::max();
        for(const ActiveGate& g : activeGates) {
            numTicks = std::min(numTicks, g.cycleCost - g.lifetime);
        }

        // tick forward
        numCycles += numTicks;
        cumulativeVert += currentVert*numTicks;

        // update active gate lifetimes and see if any have completed
        for(auto prev = activeGates.before_begin(), current = activeGates.begin();
            current != activeGates.end();
            prev = current++
        ) {
            current->lifetime += numTicks;
            if(isDone(*current)) {
                // resolve completed gate
                currentVert -= current->braidPath.size();
                deactivateGate(*current, world);
                circuit.resolveGate(current->id);

                assert(contains(activeGateIds, current->id));
                // remove from activeGates list and lookup table
                activeGateIds.erase(current->id);
                activeGates.erase_after(prev);
                current = prev;
            }
        }
        // END CYCLE UPDATE SECTION

        // while-loop updates
        executableGates = circuit.getExecutableGates();
    }
    cerr << "Done.\n" << endl;

    // calculation total computation time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

    // calculate resource utilization
    long double resUtil = static_cast<long double>(cumulativeVert)/(grid.latticeLength()*grid.latticeLength()*numCycles);

    // program output
    cout << "time taken: " << timeTaken << " microseconds" << endl;
    if(env.doSwapOptimizer) cout << "number of swap layers inserted: " << placementOptimizerCount << endl;
    cout << "num qubits: " << circuit.numLogicalQubits() << endl;;
    cout << "num gates: " << circuit.numGates() << endl;
    cout << "lattice length: " << grid.latticeLength() << endl;
    cout << "surface code distance: " << env.d << endl;
    cout << "logical error rate (-log(PL)): " << d2logPL(env.d) << endl;
    cout << "resource utilization: " << resUtil << endl;;
    cout << "scheduled circuit runtime: " << numCycles << " cycles" << endl;
    cout << "                           " << numCycles*env.timePerCycle << " microseconds" << endl;
    if(env.isQFT) { // compute formula for maslov's approach
        // NOTE: FORMULA RELIES ON ASSUMPTION THAT d > 0 (d = surface code distance)
        int numLayers = 2*circuit.numLogicalQubits() - 3;
        int maslov = getCost("h", env) + numLayers*(getCost("cx", env) + getCost("swap", env));
        cout << "maslov's approach (for qft): " << maslov << " cycles" << endl;
        cout << "                             " << maslov*env.timePerCycle << " microseconds" << endl;
    }

    return 0;
}