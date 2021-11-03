#include "config.hpp"

#include "cxxopts.hpp"
#include <iostream>
#include <cmath>
#include <cassert>

using std::cout;
using std::cerr;
using std::endl;

Environment parse(int argc, char* argv[]) {
    Environment env;

    try {
        cxxopts::Options options (argv[0], "QEC surface code braider");
        options
            .set_width(100)
            .positional_help("<qasmFileName>")
            .show_positional_help()
        ;

        double logInvPL = -1; // 1/pL

        options.add_options("parameter")
            ("d,distance", "specify surface code distance",
                cxxopts::value<int>(env.d)->default_value("33"))
            ("p,logPL", "specify target logical error rate (-log(PL))",
                cxxopts::value<double>(logInvPL))
            ("t,cycle-time", "specify time per cycle (microsec)",
                cxxopts::value<double>(env.timePerCycle)->default_value("2.2"))
            ("swap-threshold", "specify when placement optimizer activates",
                cxxopts::value<double>(env.swapThreshold)->default_value(".10"))
            ("max-swaps", "specify maximum swap layers allowed in a row",
                cxxopts::value<int>(env.maxConsecutiveSWAPLayers)->default_value("10"))
        ;
        options.add_options("algorithm config")
            ("init-place", "toggle gpmetis initial placement",
                cxxopts::value<bool>(env.doInitPlacement)->default_value("false"))
            ("swap-opt", "toggle swap-based placement optimizer",
                cxxopts::value<bool>(env.doSwapOptimizer)->default_value("false"))
            ("qft", "enable specialized code for qft circuits",
                cxxopts::value<bool>(env.isQFT)->default_value("false"))
        ;
        options.add_options("detail") // hidden options do not appear in help screen
            ("input", "input file name", cxxopts::value<std::string>(env.fileName))
        ;
        options.parse_positional("input");

        // print usage if no arguments present
        if(argc == 1) {
            cout << options.help({"parameter", "algorithm config"});
            exit(0);
        }

        // parse
        auto result = options.parse(argc, argv);
        // require that exactly one filename was input
        if(result.count("input") != 1) {
            cerr << "Exactly one filename must be input.\n";
            cerr << "Use command \'" << argv[0] << "\' with no arguments to see usage." << endl;
            exit(1);
        }
        
        // parse 1/PL option, if present
        if(result.count("p") > 0) {
            if(result.count("d") > 0) {
                cerr << "Both -d and -p options specified; -d option takes precedence." << endl;
            } else {
                env.d = logPL2d(logInvPL);
                assert(d2logPL(env.d) >= logInvPL);
            }
        }

        // emit warning for --qft option
        if(env.isQFT) {
            cerr << "WARNING (--qft enabled): code currently does not check that "
                    "the provided circuit is actually a QFT circuit." << endl;
        }
    } catch(const cxxopts::OptionException& e) {
        cerr << "parse error: " << e.what() << endl;
        exit(1);
    }

    return env;
}

// empirical constants needed to convert between d and invPL
static constexpr double p = 0.1; // both numbers represent percentages
static constexpr double pth = 0.57;

static double coeff = log10(100.0/3.0);
static double base = log10(pth/p);

int logPL2d(double PL) {
    return ceil(2*(PL - coeff)/base - 1);
}

double d2logPL(int d) {
    return coeff + (d+1)*base/2.0;
}