#include "scapegoat_tree.h"
#include <cassert>
#include <iostream>

using namespace std;
typedef long long ll;


int main() {

    ScapegoatTree empty;
    assert(!empty.contains(5));
    assert(empty.height() == -1);

    ScapegoatTree t;
    ll keys[] = {50, 30, 70, 20, 40, 60, 80};
    for (ll k : keys) {
        t.insert(k);
    }

    cout << "inorder: ";
    t.printInorder();            

    for (ll k : keys) {
        assert(t.contains(k));
    }

    assert(!t.contains(55));
    assert(!t.contains(0));
    assert(t.height() == 2);

    t.insert(40);
    t.insert(50);

    cout<<"after duplicates: ";
    t.printInorder();           
    assert(t.height() == 2);

    ScapegoatTree chain;

    for (int i=0; i<20; i++) {
        chain.insert(i);
    }
    assert(chain.height() == 19);
    cout<<"height after 20 sorted inserts: "<<chain.height()<<'\n';

    cout<<"stage 1 tests passed"<<'\n';
    return 0;

}








