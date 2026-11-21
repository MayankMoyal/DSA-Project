#include "include/tree_adapter.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    const std::string mode = argc == 2 ? argv[1] : "delete";
    if (mode != "delete" && mode != "rebuild") return 2;
    churn::HibbardBst successor;
    churn::HibbardBst randomized(churn::DeletionPolicy::RandomPredecessorOrSuccessor, 42);
    ScapegoatTree array(0.7, ARRAY_REBUILD), dsw(0.7, STOUT_WARREN_REBUILD);
    auto each = [&](auto action) { action(successor); action(randomized); action(array); action(dsw); };
    auto emit = [&](char operation, int key) {
        std::cout << "{\"operation\":\"" << operation << "\",\"key\":" << key << ",\"trees\":[";
        int index = 0;
        each([&](const auto& tree) {
            if (!churn::correct_shape(tree) || churn::keys_in_order(tree) != successor.inorder())
                throw std::runtime_error("Demo invariant failure");
            auto stats = churn::inspect(tree);
            if (index++) std::cout << ',';
            std::cout << "{\"depth\":" << stats.average_depth << ",\"rebuilds\":" << stats.rebuild_events
                      << ",\"rotations\":" << stats.rotations << ",\"nodes\":[";
            int count = 0;
            for (auto node : tree.snapshot()) {
                if (count++) std::cout << ',';
                std::cout << '[' << node[0] << ',' << node[1] << ',' << node[2] << ']';
            }
            std::cout << "]}";
        });
        std::cout << "]}\n";
    };
    if (mode == "delete") {
        for (int key : {4,2,6,1,3,5,7}) each([&](auto& t) { t.insert(key); });
        emit('S',0);
        for (int key : {4,5,6}) {
            each([&](auto& t) { t.erase(key); }); emit('D',key);
            each([&](auto& t) { t.insert(key); }); emit('I',key);
        }
    } else {
        emit('S',0);
        for (int key = 1; key <= 15; ++key) {
            each([&](auto& t) { t.insert(key); }); emit('I',key);
        }
        // Five removals cross the shrink threshold: 10 < 0.7 * 15.
        for (int key : {8, 4, 12, 2, 14}) {
            each([&](auto& t) { t.erase(key); }); emit('D',key);
        }
    }
}
