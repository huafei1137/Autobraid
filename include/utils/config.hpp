#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Environment {
    std::string fileName;
    int d;
    double timePerCycle;
    bool doInitPlacement;
    bool doSwapOptimizer;
    double swapThreshold; // (scheduled ratio <= threshold) -> trigger placement optimizer
    int maxConsecutiveSWAPLayers; // number of consecutive swap layers allowed
    bool isQFT; // qft circuits need special treatment
};

Environment parse(int argc, char* argv[]);

// functions to convert between surface code distance and -log(PL)
int logPL2d(double PL);
double d2logPL(int d);

#endif