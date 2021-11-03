#include "findswaps.hpp"

#include <cassert>
#include "setutils.hpp"
#include "graphutils.hpp"
#include "pathfind.hpp"

/* ----- private helper functions ----- */

// see if the new swap gate can be scheduled w/o a braiding resource conflict
// returns number of vertices used, of -1 if braiding conflict occurs
// takes 'stk' and 'world' by copy
int canSchedule(std::stack<Gate> stk, const Gate& newSwap, const Lattice& grid, Matrix world) {
    int numVertices = 0;
    stk.push(newSwap);
    while(!stk.empty()) {
        Gate swap = stk.top();
        stk.pop();
        std::vector<Point> path = braid(swap, grid, world);
        if(path.empty()) {
            return -1;
        } else {
            numVertices += path.size();
            for(const Point& p : path) {
                assert(world[p] == 0);
                world[p] = 1;
            }
        }
    }
    return numVertices;
}

/* ----- actual function implementations ----- */

int findSwaps(std::vector<Gate> frontLayer, Lattice& grid, const Matrix& world, int& res) {
    res = 0; // at first, no resources are used yet
    for(int i = 0; i < frontLayer.size(); ++i) { // relabel gate ids for ease of access (function-local ids)
        assert(!isSingle(frontLayer[i]));
        frontLayer[i].id = i;
    }

    std::stack<Gate> SWAPstack; // stack of SWAP gates scheduled so far
    std::unordered_set<int> busyQubits; // set of logical qubits participating in a SWAP (in SWAPstack)

    // predicates to ensure the same (logical) qubit doesn't take part in 2+ SWAPS simultaneously
    auto isFreeQubit = [&](int qubit) -> bool { return !contains(busyQubits, qubit); };
    auto isFreeVertex = [&](const Graph::Vertex& v) -> bool {
        return isFreeQubit(frontLayer[v.id].control) && isFreeQubit(frontLayer[v.id].target);
    };

    // interference graph
    Graph interferenceGraph = buildInterferenceGraph(frontLayer, grid);
    int interference = std::numeric_limits<int>::max(); // objective we are trying to minimize

    // enter loop -> schedule as many swaps as possible
    while(true) {
        // attempt to find candidate gate 1
        int id1 = getMaxDegreeVertexId(interferenceGraph, isFreeVertex);
        if(id1 == -1) break;

        // attempt to find candidate gate 2
        const Graph::Vertex& v1 = interferenceGraph.getVertex(id1);
        auto isFreeNeighbour = [&](const Graph::Vertex& v) -> bool {
            return isFreeVertex(v) && contains(v1.neighbours, v.id);
        };
        int id2 = getMaxDegreeVertexId(interferenceGraph, isFreeNeighbour);
        if(id2 == -1) break;

        // determine swap qubit pair candidates
        int qubits1[2] = { frontLayer[id1].control, frontLayer[id1].target };
        int qubits2[2] = { frontLayer[id2].control, frontLayer[id2].target };
        int swapqubit1 = -1, swapqubit2 = -1; // placeholders for the chosen swap pair
        Gate newSwap = { -1, "swap", -1, -1 }; // id set to -1 b/c it is not used

        // try each qubit pair combination and see which one is the best
        for(int q1 : qubits1) {
            for(int q2 : qubits2) {
                if(!isFreeQubit(q1) || !isFreeQubit(q2)) continue; // make sure qubits are free
                newSwap.control = q1; newSwap.target = q1;
                int numVertices = canSchedule(SWAPstack, newSwap, grid, world);
                if(numVertices == -1) continue; // make sure resources are sufficient

                // test the swap and see if it reduces the objective
                grid.swapLogicalQubit(q1, q2);
                interferenceGraph = buildInterferenceGraph(frontLayer, grid);
                grid.swapLogicalQubit(q1, q2); // swap back to undo the mapping change
                if(interferenceGraph.numEdges() < interference) {
                    interference = interferenceGraph.numEdges();
                    swapqubit1 = q1;
                    swapqubit2 = q2;
                    res = numVertices; // update resource utilization metric
                }
            }
        }

        if(swapqubit1 == -1 || swapqubit2 == -1) break; // no valid swap candidate found

        // else, schedule new swap gate, officially alter mapping, and push onto the stack
        newSwap.control = swapqubit1; newSwap.target = swapqubit2;
        SWAPstack.push(newSwap);
        busyQubits.insert({swapqubit1, swapqubit2}); // mark qubits as busy
        grid.swapLogicalQubit(swapqubit1, swapqubit2);

        // update interference graph
        interferenceGraph = buildInterferenceGraph(frontLayer, grid);
    }

    return SWAPstack.size();
}
