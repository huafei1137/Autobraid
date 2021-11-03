#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <unordered_map>
#include <vector>

#include "setutils.hpp"

namespace detail {
    struct Vertex {
        int id;
        std::unordered_set<int> neighbours;

        Vertex(int id) : id(id) {}

        int degree() const { return neighbours.size(); }
    };
}

// generic undirected graph implementation
class Graph {
public:
    using Vertex = detail::Vertex;
    struct const_iterator; // read-only forward iterator wrapping underlying map iterator

    using vertex_list = std::unordered_map<int, Vertex>;

    void addVertex(int id);
    void deleteVertex(int id); // assumes id is present in the graph
    void addEdge(int id1, int id2); // assumes id1 and id2 are already present in the graph

    const Vertex& getVertex(int id) const { return graph.at(id); }
    int numVertices() const { return graph.size(); }
    int numEdges() const { return size; }

    const_iterator begin() const;
    const_iterator end() const;

private:
    vertex_list graph;
    int size = 0;
};

struct Graph::const_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = vertex_list::const_iterator::difference_type;
    using value_type        = const Vertex;
    using pointer           = const Vertex*;
    using reference         = const Vertex&;

    const_iterator() {}
    const_iterator(vertex_list::const_iterator iter) : ptr(iter) {}

    bool operator==(const const_iterator& other) { return ptr == other.ptr; }
    bool operator!=(const const_iterator& other) { return ptr != other.ptr; }

    const_iterator& operator++() { ++ptr; return *this; }
    const_iterator operator++(int) { const_iterator temp = *this; ++ptr; return temp; }

    reference operator*() const { return ptr->second; }
    pointer operator->() const { return &(ptr->second); }

private:
    vertex_list::const_iterator ptr;
};

#endif