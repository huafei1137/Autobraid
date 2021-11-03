#include "Lattice.hpp"

#include <array>

Lattice::Lattice(int length): length(length) {
    log2phys = std::vector<int>(length*length);
    for(int i = 0; i < log2phys.size(); ++i) log2phys[i] = i;
}

void Lattice::setMapping(std::vector<int> mp) {
    log2phys = std::move(mp);
}

Cell Lattice::getLatticePosition(int logQubit) const {
    int physQubit = log2phys[logQubit];
    return { physQubit%length, physQubit/length };
}

int Lattice::getPhysQubitNumber(const Cell& c) const {
    return c.y*length + c.x;
}

int Lattice::getArea(const Gate& g) const {
    Point p1 = getLatticePosition(g.control);
    Point p2 = getLatticePosition(g.target);
    int dx = abs(p1.x - p2.x) + 1;
    int dy = abs(p1.y - p2.y) + 1;
    return dx*dy;
}

// NOTE: if two bounding boxes share an edge or even just a corner,
// they are considered as overlapping
bool Lattice::checkOverlap(const Gate& g1, const Gate& g2) const {
    using Range = std::array<int, 2>;

    // local function to generate box intervals
    auto getInterval = [this](const Gate& g, Range& x, Range& y) {
        Cell c1 = this->getLatticePosition(g.control);
        Cell c2 = this->getLatticePosition(g.target);
        x = { std::min(c1.x, c2.x), std::max(c1.x, c2.x) + 1 };
        y = { std::min(c1.y, c2.y), std::max(c1.y, c2.y) + 1 };
    };

    Range xRange1, yRange1;
    Range xRange2, yRange2;
    getInterval(g1, xRange1, yRange1);
    getInterval(g2, xRange2, yRange2);

    bool doesXOverlap = (xRange1[0] <= xRange2[1]) && (xRange2[0] <= xRange1[1]);
    bool doesYOverlap = (yRange1[0] <= yRange2[1]) && (yRange2[0] <= yRange1[1]);

    return doesXOverlap && doesYOverlap;
}

void Lattice::swapLogicalQubit(int log1, int log2) {
    int temp = log2phys[log1];
    log2phys[log1] = log2phys[log2];
    log2phys[log2] = temp;
}
