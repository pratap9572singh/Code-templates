#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std;
using namespace __gnu_pbds;

template<typename T>
using ordered_multiset = tree<T, null_type, less_equal<T>, rb_tree_tag, tree_order_statistics_node_update>;

int main() {
    ordered_multiset<int> s;
    
    s.insert(5);
    s.insert(2);
    s.insert(6);
    s.insert(4);
    
    cout << "3rd smallest: " << *s.find_by_order(2) << "\n";
    
    cout << "Elements less than 6: " << s.order_of_key(6) << "\n";
    
    for(auto x : s) {
        cout << x << " ";
    }
    cout << "\n";
}
