/*
--- Standard Set Operations ---
insert(x)      : Adds x to the tree.
erase(x)       : Removes value x.
find(x)        : Returns iterator to x, or end() if not found.
lower_bound(x) : Returns iterator to first element >= x.
upper_bound(x) : Returns iterator to first element > x.
size()         : Returns total number of elements.
empty()        : Returns true if the tree is empty.
clear()        : Removes all elements.
begin() / end(): Iterators for traversal.

--- PBDS Exclusive Operations ---
find_by_order(k) : Returns an iterator to the k-th smallest element (0-indexed).
order_of_key(x)  : Returns an integer count of elements strictly less than x.


REMEMBER DO NOT USE IT FOR MULTISET IT DOES NOT SUPPORT THAT 
*/

#include <iostream>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

using namespace std;
using namespace __gnu_pbds;

typedef tree<int, null_type, less<int>, rb_tree_tag, tree_order_statistics_node_update> os;

int main() {
#ifndef ONLINE_JUDGE
    freopen("input.in", "r", stdin);
    freopen("output.in", "w", stdout);
#endif
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    os t;
    
    t.insert(10);
    t.insert(20);
    t.insert(30);
    t.insert(40);

    cout << *t.find_by_order(1) << endl;

    cout << t.order_of_key(25) << endl;

    return 0;
}
