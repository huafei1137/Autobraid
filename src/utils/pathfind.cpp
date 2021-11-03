#include "pathfind.hpp"

#include <algorithm>

// tries pathfind() on all starting points
std::vector<Point> braid(const Gate& g, const Lattice& grid, const Matrix& world) {
    static const Point corners[4] = {{0,0}, {0,1}, {1,0}, {1,1}};

    Cell source = grid.getLatticePosition(g.control);
    Cell dest = grid.getLatticePosition(g.target);

    int shortestDist = std::numeric_limits<int>::max();
    std::vector<Point> path;
    for(const Point& p : corners) { // try each corner
        Point start = source + p;
        auto tempPath = pathfind(start, dest, world); // perform A* search
        if(tempPath.size() == 0) continue; // no path found

        if(tempPath.size() < shortestDist) {
            shortestDist = tempPath.size();
            path = std::move(tempPath);
        }
    }
    return path;
}

// Matrix 'world' is copied by value b/c it is also used to store path traceback info.
// the heuristic cost function used must be consistent.
std::vector<Point> pathfind(const Point& start, const Cell& dest, Matrix world) {
    // functor used to order the fringe
    class CompareDist {
    public:
        CompareDist(const Cell& d, const Matrix& dist) : dest(d), dist(dist) {}
        bool operator()(const Point& p1, const Point& p2) {
            if(dist[p1]+manhattan(p1, dest) != dist[p2]+manhattan(p2, dest))
                return dist[p1]+manhattan(p1, dest) > dist[p2]+manhattan(p2, dest);
            else
                return dist[p1] < dist[p2];
        }

    private:
        Cell dest;
        const Matrix& dist;
    };

    // stores directions to expand search in
    static const Point directions[4] = {{-1,0}, {0,-1}, {1,0}, {0,1}};
    static const Point inverseDirections[4] = {{1,0}, {0,1}, {-1,0}, {0,-1}};

    std::vector<Point> path;
    if(world[start] != 0) return path; // shortcut check

    Matrix dist (world.numRows(), world.numCols()); // tracks best distance found so far
    bool dirty = false; // tracks if fringe has been updated and needs to be heapified again
    CompareDist comp (dest, dist);
    std::vector<Point> fringe;

    bool found = false;
    Point final; // stores final goal node, if a path is found

    dist[start] = 1;
    fringe.push_back(start);
    while(!fringe.empty()) {
        std::pop_heap(fringe.begin(), fringe.end(), comp);
        Point current = fringe.back();
        fringe.pop_back();

        // check if current is a goal node
        if(manhattan(current, dest) == 0) {
            found = true;
            final = current;
            break;
        }

        // parse neighbours and add them to fringe, if necessary
        for(int i = 0; i < 4; ++i) {
            Point next = current + directions[i];
            int newDist = dist[current] + 1; // tentative distance to reach current's neighbours

            // check the new point is in bounds
            if(next.x < 0 || next.x >= world.numCols() || next.y < 0 || next.y >= world.numRows()) {
                continue;
            }

            // check if the new point is taken/is an obstacle
            if(world[next] != 0 && dist[next] == 0) {
                continue;
            }

            // check if the new point is not yet in the fringe
            if(dist[next] == 0) {
                dist[next] = newDist;
                world[next] = i+1; // maintain traceback info
                fringe.push_back(next);
                if(!dirty) std::push_heap(fringe.begin(), fringe.end(), comp);
                continue;
            }

            // otherwise, new point is in the fringe; check if update is needed
            if(dist[next] > newDist) {
                dist[next] = newDist;
                world[next] = i+1;
                dirty = true;
            }
        }

        // check if fringe needs to be re-heapified
        if(dirty) {
            std::make_heap(fringe.begin(), fringe.end(), comp);
            dirty = false;
        }
    }

    if(found) {
        path.reserve(dist[final]);
        while(world[final] != 0) { // traceback to get path
            path.push_back(final);
            final = final + inverseDirections[world[final]-1];
        }
        path.push_back(final);
    }

    return path;
}

int manhattan(const Point& vertex, const Cell& dest) {
    int dx = std::min(abs(vertex.x - dest.x), abs(vertex.x - dest.x - 1));
    int dy = std::min(abs(vertex.y - dest.y), abs(vertex.y - dest.y - 1));
    return dx + dy;
}