#ifndef TRIVIAL_PRED_HPP
#define TRIVIAL_PRED_HPP

// defines a trivial predicate that always returns true no matter the input
template<class ...T>
class TrivialPred {
public:
    bool operator()(const T&... args) { return true; }
};

#endif