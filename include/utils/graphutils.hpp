#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include "Graph.hpp"
#include "CircuitDAG.hpp"
#include "Lattice.hpp"
#include "TrivialPred.hpp"

// use on graphs with max degree = 2
// splits vertices into components, sorted by size (small -> large)
// vertices within paths are listed in order from one end to the other
// vertices within cycles are listed in order along said cycle (w/ arbitrary starting vertex)
std::vector<std::vector<int>> getComponentsInOrder(const Graph& g);

Graph buildInterferenceGraph(const std::vector<Gate>& gates, const Lattice& grid);

// returns max degree vertex satisfying a filter, or -1 if there is none
// both tiebreaker and filter should take Graph::Vertex instances as arguments
// tiebreaker(v1, v2) == true implies v1 should be prioritized over v2
template<
    class Filter = TrivialPred<Graph::Vertex>,
    class Comp = TrivialPred<Graph::Vertex, Graph::Vertex>
>
int getMaxDegreeVertexId(const Graph& g, Filter filter = Filter(), Comp tiebreaker = Comp()) {
    // track id, degree, and area of highest interfering CX gate found so far
    int maxId = -1;
    int maxDegree = -1;
    for(const Graph::Vertex& v : g) {
        if(!filter(v)) continue; // ignore vertices not satisfying the filter
        bool isHigher = (v.degree() == maxDegree) ?
            tiebreaker(v, g.getVertex(maxId)) :
            v.degree() > maxDegree;
        if(isHigher) {
            maxId = v.id;
            maxDegree = v.degree();
        }
    }
    return maxId;
}

#endif