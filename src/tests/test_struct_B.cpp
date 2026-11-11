#include "scapegoat_tree.h"
#include <cassert>
#include <iostream>

using namespace std;


int main() {
    ScapegoatTree empty;
    assert(!empty.contains(5));
    assert(empty.height() == -1);
    assert(empty.size() == 0);
    assert(!empty.erase(5));

    ScapegoatTree t;
    ll keys[] = {50, 30, 70, 20, 40, 60, 80};

    for (ll k : keys) t.insert(k);

    cout << "inorder: ";
    t.printInorder();

    for (ll k : keys) assert(t.contains(k));

    assert(!t.contains(55));
    assert(!t.contains(0));
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

    cout << "after deletions: ";
    t.printInorder();

    ScapegoatTree chain;
    for (int i = 0; i < 20; i++) chain.insert(i);

    cout << "height after 20 sorted inserts: " << chain.height() << '\n';
    assert(chain.size() == 20);
    assert(chain.height() < 19);

    for (int i = 0; i < 8; i++) {
        assert(chain.erase(i));
    }
    assert(chain.size() == 12);

    for (int i = 0; i < 8; i++) assert(!chain.contains(i));
    for (int i = 8; i < 20; i++) assert(chain.contains(i));

    cout << "height after deletion-triggered rebuild: " << chain.height() <<'\n';
    cout << "Day 3 tests passed" <<'\n';
    return 0;
}
////
