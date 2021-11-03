#ifndef PARTITION_HPP
#define PARTITION_HPP

#include "metis.h"
#include "Graph.hpp"
#include "Lattice.hpp"

#include <vector>

// alternative graph representation used by metis libraries
struct metisGraph {
    std::vector<idx_t> xadj; // length of each adjacency list
    std::vector<idx_t> adjncy; // combined adjacency lists for each vertice
    std::vector<int> id_map; // conversion from index to actual Graph::Vertex id
};

// convert graph to metis representation
metisGraph convert2metisGraph(const Graph& g);

// initialize metis options (call before initialPlacement)
void initMetisOptions();

// perform recursive partition to get initial placement
std::vector<int> initialPlacement(const Graph& coupling, const Lattice& grid);

#endif