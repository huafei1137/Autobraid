#ifndef POINT_HPP
#define POINT_HPP

// represents the vertex of a cell in the lattice
struct Point {
    int x, y;
};

inline Point operator+(const Point& a, const Point& b) {
    return { a.x+b.x, a.y+b.y };
}

// represents a cell in the lattice
// a cell is identified by its top left corner
using Cell = Point;

#endif