#include "Graph.hpp"

void Graph::addVertex(int id) {
    graph.insert({id, Vertex(id)});
}

void Graph::addEdge(int id1, int id2) {
    int oldDegree = graph.at(id1).degree();
    graph.at(id1).neighbours.insert(id2);
    graph.at(id2).neighbours.insert(id1);
    if(oldDegree != graph.at(id1).degree())
        ++size; // increment if edge was not already present
}

void Graph::deleteVertex(int id) {
    for(int neighbour : graph.at(id).neighbours) {
        graph.at(neighbour).neighbours.erase(id);
    }
    size -= graph.at(id).degree();
    graph.erase(id);
}

/* ----- graph iterator functions ----- */

Graph::const_iterator Graph::begin() const {
    return const_iterator(graph.cbegin());
}

Graph::const_iterator Graph::end() const {
    return const_iterator(graph.cend());
}