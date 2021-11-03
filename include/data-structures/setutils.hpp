#ifndef SET_UTILS_HPP
#define SET_UTILS_HPP

#include <unordered_set>

template<class T>
inline bool contains(const std::unordered_set<T>& set, const T& item) {
    return set.find(item) != set.end();
}

#endif