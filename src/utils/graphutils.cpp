#include "graphutils.hpp"

#include <cassert>
#include <algorithm>

/* ----- private helper functions ----- */

// recursive helper functions; DFS specialized for graphs w/ max degree 2
static void backDFS(
    const Graph& g,
    int id,
    std::vector<int>& cc,
    std::unordered_set<int>& visited,
    int& edges
) {
    visited.insert(id);
    for(int neighbour : g.getVertex(id).neighbours) {
        if(contains(visited, neighbour)) continue;
        backDFS(g, neighbour, cc, visited, edges);
        break;
    }

    cc.push_back(id);
    edges += g.getVertex(id).degree();
}

static void forwardDFS(
    const Graph& g,
    int id,
    std::vector<int>& cc,
    std::unordered_set<int>& visited,
    int& edges
) {
    visited.insert(id);
    for(int neighbour : g.getVertex(id).neighbours) {
        if(contains(visited, neighbour)) continue;

        cc.push_back(neighbour);
        edges += g.getVertex(id).degree();

        forwardDFS(g, neighbour, cc, visited, edges);
        break;
    }
}

/* ----- utility function implementations ----- */

std::vector<std::vector<int>> getComponentsInOrder(const Graph& g) {
    // will store # edges as last element of each Component (for sorting)
    using Component = std::vector<int>;

    std::vector<Component> result;
    std::unordered_set<int> visited;
    for(const Graph::Vertex& v : g) {
        if(contains(visited, v.id)) continue;
        Component cc;
        int numEdges = 0; // compute 2m, where m = # edges in component
        switch(v.degree()) {
            case 0:
                cc.push_back(v.id);
                break;
            case 1:
                backDFS(g, v.id, cc, visited, numEdges);
                break;
            case 2:
                backDFS(g, v.id, cc, visited, numEdges);
                forwardDFS(g, v.id, cc, visited, numEdges);
                break;
            default:
                assert(false); // all vertices should have degree <= 2
        }
        cc.push_back(numEdges);
        result.push_back(std::move(cc));
    }

    // sort the components by size
    auto compareComponents = [](const Component& c1, const Component& c2) -> bool {
        return c1.back() < c2.back();
    };
    std::sort(result.begin(), result.end(), compareComponents);

    // remove edge counter from each component
    std::for_each(result.begin(), result.end(), [](Component& c) { c.pop_back(); });
    return result;
}

Graph buildInterferenceGraph(const std::vector<Gate>& gates, const Lattice& grid) {
    Graph interference;
    for(int i = 0; i < gates.size(); ++i) {
        interference.addVertex(gates[i].id);
        for(int j = 0; j < i; ++j) {
            if(grid.checkOverlap(gates[i], gates[j])) {
                interference.addEdge(gates[i].id, gates[j].id);
            }
        }
    }
    return interference;
}