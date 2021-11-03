#include "partition.hpp"

#include <cassert>
#include <array>
#include <unordered_map>
#include <algorithm>

/* ----- private helper functions and metis parameters ----- */

using Range = std::array<int, 2>; // represents the interval [arr0, arr1)

static idx_t options[METIS_NOPTIONS];

// recursive helper function called by initialPlacement()
static void initPlacementInner(
    std::vector<int>& map,
    const Graph& cp,
    const Lattice& grid,
    const Range& split, // the interval that will be halved
    const Range& keep, // the interval that is kept the same
    bool splitX // is 'split' an x-axis interval? (else it is a y-axis interval)
) {
    // test base condition ( <= 2 vertices to place )
    if(cp.numVertices() <= 2) {
        const Range& xRange = (splitX)? split : keep;
        const Range& yRange = (splitX)? keep : split;

        // assert positive area (ie there exist cells to place the logical qubits into)
        assert(xRange[1] > xRange[0] && yRange[1] > yRange[0]);

        // arbitrarily place remaining logical qubits
        auto vptr = cp.begin();
        for(int x = xRange[0]; x < xRange[1]; ++x) {
            for(int y = yRange[0]; y < yRange[1]; ++y) {
                if(vptr == cp.end()) break;
                map[vptr->id] = grid.getPhysQubitNumber({x, y});
                ++vptr;
            }
        }
        return;
    }

    // divide the interval to be split and get their areas
    Range half1 = { split[0], (split[0] + split[1])/2 };
    Range half2 = { (split[0] + split[1])/2, split[1] };
    int area1 = (half1[1] - half1[0])*(keep[1] - keep[0]);
    int area2 = (half2[1] - half2[0])*(keep[1] - keep[0]);
    real_t totalArea = area1 + area2;

    // convert graph to CSR format used by metis
    auto mg = convert2metisGraph(cp);

    // set up and execute graph partition
    idx_t nvtxs = cp.numVertices();
    idx_t ncon = 1;
    idx_t nparts = 2; // partition into two halves
    std::array<real_t, 2> tpwgts = { area1/totalArea, area2/totalArea };
    idx_t edgecut;
    std::vector<idx_t> partition (nvtxs);
    int status = METIS_PartGraphRecursive(
        &nvtxs, &ncon,
        mg.xadj.data(), mg.adjncy.data(),
        NULL, NULL, NULL,
        &nparts, tpwgts.data(),
        NULL, options,
        &edgecut,
        partition.data()
    );
    assert(status == METIS_OK);

    // calculate sizes of the returned partitions
    int size1 = std::count_if(partition.begin(), partition.end(), [](idx_t x) { return x == 0; });
    int size2 = nvtxs - size1;

    // redistribute vertices, if necessary (ie adjust partition sizes)
    while(size1 > area1) {
        --size1;
        ++size2;
    }
    while(size2 > area2) {
        --size2;
        ++size1;
    }

    // create subgraphs by deleting vertices not belonging to their corresponding partition
    Graph subcp1 = cp;
    Graph subcp2 = cp;
    idx_t index = 0;
    while(subcp1.numVertices() > size1 && subcp2.numVertices() > size2) {
        int actualIndex = mg.id_map[index];
        if(partition[index] == 0) subcp2.deleteVertex(actualIndex);
        else subcp1.deleteVertex(actualIndex);
        ++index;
    }
    // redistribute vertices, if necessary
    while(subcp1.numVertices() > size1) subcp1.deleteVertex(mg.id_map[index++]);
    while(subcp2.numVertices() > size2) subcp2.deleteVertex(mg.id_map[index++]);
    
    // recursively split partitions further
    initPlacementInner(map, subcp1, grid, keep, half1, !splitX);
    initPlacementInner(map, subcp2, grid, keep, half2, !splitX);
}

/* ----- header function implementations ----- */

metisGraph convert2metisGraph(const Graph& g) {
    metisGraph out;
    out.xadj.reserve(g.numVertices() + 1);
    out.adjncy.reserve(2*g.numEdges());
    out.id_map.reserve(g.numVertices());

    std::unordered_map<int, idx_t> inverseMap;

    // process vertex mapping
    idx_t freeIndex = 0; // track which index is currently free
    for(const Graph::Vertex& v : g) {
        out.id_map.push_back(v.id);
        inverseMap[v.id] = freeIndex;
        ++freeIndex;
    }

    // construct xadj and adjncy
    out.xadj.push_back(0);
    for(int i : out.id_map) {
        const Graph::Vertex& v = g.getVertex(i);
        out.xadj.push_back(out.xadj.back() + v.degree());
        for(int neighbour : v.neighbours) {
            out.adjncy.push_back(inverseMap[neighbour]);
        }
    }

    return out;
}

void initMetisOptions() {
    options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM;
    options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_GROW;
    options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM;
    options[METIS_OPTION_NO2HOP] = 0;
    options[METIS_OPTION_NCUTS] = 500;
    options[METIS_OPTION_NITER] = 10;
    options[METIS_OPTION_UFACTOR] = 1;
    options[METIS_OPTION_NUMBERING] = 0;
    options[METIS_OPTION_DBGLVL] = 0;
}

std::vector<int> initialPlacement(const Graph& coupling, const Lattice& grid) {
    assert(coupling.numVertices() <= grid.latticeLength()*grid.latticeLength());
    std::vector<int> map (coupling.numVertices());
    initPlacementInner(map, coupling, grid, {0, grid.latticeLength()}, {0, grid.latticeLength()}, true);
    return map;
}