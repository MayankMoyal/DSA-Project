#include "insertions.h"
#include "scapegoat_tree.h"
#include <chrono>
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {
    try {
        if (argc != 5) throw std::invalid_argument("usage: measure_rebuilds n trial seed alpha");
        int n = std::stoi(argv[1]), trial = std::stoi(argv[2]);
        auto seed = std::stoull(argv[3]);
        double alpha = std::stod(argv[4]);
        std::cout << std::setprecision(12);
        std::cout << "n,trial,seed,alpha,pattern,mode,insert_ns,rebuilds,nodes_rebuilt,rotations,average_depth,height,trace_hash\n";
        for (auto pattern : {"ascending", "descending", "extremes", "blocks", "shuffle"}) {
            auto keys = adversary::insertionKeys(n, seed, pattern);
            adversary::TraceHash hash;
            for (auto key : keys) hash.add('I', key);
            for (auto mode : {ARRAY_REBUILD, STOUT_WARREN_REBUILD}) {
                ScapegoatTree tree(alpha, mode);
                auto start = std::chrono::steady_clock::now();
                for (auto key : keys) tree.insert(key);
                auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
                if (tree.size() != n || !tree.checkInvariants()) throw std::runtime_error("insertion audit failed");
                auto expected = keys; std::sort(expected.begin(), expected.end());
                if (tree.toVector() != expected) throw std::runtime_error("insertion contents mismatch");
                auto s = tree.getRebuildStats();
                std::cout << n << ',' << trial << ',' << seed << ',' << alpha << ',' << pattern << ','
                          << (mode == ARRAY_REBUILD ? "array" : "dsw") << ',' << ns << ','
                          << s.scapegoat_rebuilds << ',' << s.nodes_rebuilt << ',' << s.stout_warren_rotations << ','
                          << tree.averageDepth() << ',' << tree.height() << ',' << hash.get() << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
