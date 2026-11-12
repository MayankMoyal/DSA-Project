#include "scapegoat_tree.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>

using namespace std;

void testBasicOperations() {
    ScapegoatTree empty;
    assert(!empty.contains(5));
    assert(empty.height() == -1);
    assert(empty.size() == 0);
    assert(empty.averageDepth() == 0.0);
    assert(!empty.erase(5));

    ScapegoatTree t;
    ll keys[] = {50, 30, 70, 20, 40, 60, 80};

    for (ll k : keys) t.insert(k);
    for (ll k : keys) assert(t.contains(k));

    assert(!t.contains(55));
    assert(t.size() == 7);

    t.insert(40);
    t.insert(50);
    assert(t.size() == 7);

    assert(t.erase(20));
    assert(!t.contains(20));
    assert(t.size() == 6);

    assert(t.erase(30));
    assert(t.contains(40));
    assert(t.contains(50));
    assert(t.size() == 5);

    assert(!t.erase(999));
    assert(t.size() == 5);
}

void testBothRebuildModes(RebuildMode mode) {
    ScapegoatTree t(0.7, mode);

    for (int i = 0; i < 500; i++) t.insert(i);

    assert(t.size() == 500);
    assert(t.height() < 30);

    for (int i = 0; i < 500; i++) assert(t.contains(i));

    DepthStats d = t.depthStats();
    assert(d.count == 500);
    assert(d.height == t.height());
    assert(d.average() > 0.0);
    assert(abs(d.average() - t.averageDepth()) < 1e-9);

    for (int i = 0; i < 200; i++) assert(t.erase(i));
    for (int i = 0; i < 200; i++) assert(!t.contains(i));
    for (int i = 200; i < 500; i++) assert(t.contains(i));

    const RebuildStats& s = t.getRebuildStats();
    assert(s.insertions == 500);
    assert(s.deletions == 200);
    assert(s.scapegoat_rebuilds > 0);
    assert(s.whole_tree_rebuilds > 0);
    assert(s.nodes_rebuilt > 0);

    if (mode == ARRAY_REBUILD) {
        assert(s.array_rebuilds > 0);
    } else {
        assert(s.stout_warren_rebuilds > 0);
        assert(s.stout_warren_rotations > 0);
    }
}

void testRandomAgainstSet(RebuildMode mode) {
    ScapegoatTree t(0.65, mode);
    set<ll> oracle;
    unsigned int seed = 1234567;

    for (int i = 0; i < 20000; i++) {
        seed = seed * 1664525u + 1013904223u;
        ll x = static_cast<ll>((seed >> 1) % 2000);

        if (seed & 1u) {
            t.insert(x);
            oracle.insert(x);
        } else {
            bool expected = oracle.erase(x) > 0;
            assert(t.erase(x) == expected);
        }

        assert(t.size() == static_cast<int>(oracle.size()));

        if (i % 100 == 0) {
            for (ll key = 0; key < 2000; key += 37) {
                assert(t.contains(key) == (oracle.count(key) > 0));
            }
            assert(t.height() < 100);
            assert(t.depthStats().count == static_cast<long long>(oracle.size()));
        }
    }
}

int main() {
    testBasicOperations();
    testBothRebuildModes(ARRAY_REBUILD);
    testBothRebuildModes(STOUT_WARREN_REBUILD);
    testRandomAgainstSet(ARRAY_REBUILD);
    testRandomAgainstSet(STOUT_WARREN_REBUILD);

    cout << "All Structure B tests passed." <<'\n';
    return 0;
}







