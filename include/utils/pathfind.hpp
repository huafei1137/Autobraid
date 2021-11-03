#ifndef PATHFIND_HPP
#define PATHFIND_HPP

#include <vector>

#include "Lattice.hpp"
#include "Matrix.hpp"

// pathfinds for Cell -> Cell
// returns the path found, or an empty vector if no path is found
std::vector<Point> braid(const Gate& g, const Lattice& grid, const Matrix& world);

// performs A* search for Point -> Cell
// returns the path found, or an empty vector if no path is found
// world[i][j] == 0 indicates that that vertex is free; otherwise it is taken (ie is an obstacle).
std::vector<Point> pathfind(const Point& start, const Cell& dest, Matrix world);

// manhattan/L1 distance: used as heuristic cost for A* search
// equals 0 iff 'vertex' is a corner of 'dest'
int manhattan(const Point& vertex, const Cell& dest);

#endif