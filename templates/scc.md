# SCC (Strongly Connected Components, Kosaraju)

Splits a directed graph into groups where **everyone in a group can reach everyone else** in it.
Squashing each group into one vertex gives a graph with **no cycles**.

**Reach for it when:**
- moving around inside a loop is free, so a loop behaves like one vertex
- the problem would be easy if the graph had no cycles
- "who can reach whom both ways"
- 2-SAT (built on top of this)

## Usage

```cpp
SCC s(n);                        // vertices 1..n
for (int i = 0; i < m; i++) {
    int u, v; cin >> u >> v;
    s.addEdge(u, v);             // arrow u → v
}
s.build();                       // call once, after all edges
```

| field | meaning |
|---|---|
| `s.groups` | number of groups |
| `s.comp[v]` | group of vertex v, from `0` to `groups - 1` |
| `s.members[g]` | vertices in group g |
| `s.dag[g]` | groups that g has arrows to (no duplicates, no arrows to itself) |

**The one guarantee:** every arrow in `dag` goes from a **smaller** group number to a **bigger** one.

- DP that flows **along** the arrows ("best ending here") → `for (g = 0; g < groups; g++)`
- DP that flows **against** the arrows ("best starting here") → `for (g = groups - 1; g >= 0; g--)`

Vertices are 1-based. For 0-based input, add 1 when reading.

## Code

```cpp
// Strongly connected components (Kosaraju).
// Groups vertices so that inside a group everyone can reach everyone.
// Group numbers come in topological order: every arrow between two
// different groups goes from a smaller group number to a bigger one.
struct SCC {
    int n, groups = 0;
    vector<vector<int>> adj, radj;      // arrows, and the same arrows flipped
    vector<int> comp;                   // comp[v] = group of v
    vector<vector<int>> members;        // members[g] = vertices in group g
    vector<vector<int>> dag;            // dag[g] = groups that g has arrows to

    vector<bool> seen;
    vector<int> order;

    SCC(int n) : n(n), adj(n + 1), radj(n + 1) {}    // vertices 1..n

    void addEdge(int u, int v) {
        adj[u].push_back(v);
        radj[v].push_back(u);
    }

    // pass 1: DFS recording exit order.
    // Written with an explicit stack so a long chain can't overflow the call stack.
    // Each stack entry is {vertex, index of the next arrow to try}.
    void dfs1(int start) {
        vector<pair<int, int>> st = {{start, 0}};
        seen[start] = true;
        while (!st.empty()) {
            int u = st.back().first;
            int i = st.back().second;
            if (i < (int)adj[u].size()) {
                st.back().second++;             // next time, try the following arrow
                int v = adj[u][i];
                if (!seen[v]) {
                    seen[v] = true;
                    st.push_back({v, 0});       // go into v
                }
            } else {
                order.push_back(u);             // all arrows tried: leave u
                st.pop_back();
            }
        }
    }

    // pass 2: collect everything reachable on flipped arrows that has no group yet.
    // Order doesn't matter here, so a plain stack is enough.
    void dfs2(int start) {
        vector<int> st = {start};
        comp[start] = groups;
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            for (int v : radj[u])
                if (comp[v] == -1) {
                    comp[v] = groups;
                    st.push_back(v);
                }
        }
    }

    void build() {
        seen.assign(n + 1, false);
        for (int v = 1; v <= n; v++)
            if (!seen[v]) dfs1(v);

        comp.assign(n + 1, -1);
        for (int i = n - 1; i >= 0; i--) {  // last to leave goes first
            int v = order[i];
            if (comp[v] == -1) {
                dfs2(v);
                groups++;
            }
        }

        members.assign(groups, {});
        for (int v = 1; v <= n; v++) members[comp[v]].push_back(v);

        dag.assign(groups, {});
        for (int u = 1; u <= n; u++)
            for (int v : adj[u])
                if (comp[u] != comp[v]) dag[comp[u]].push_back(comp[v]);
        for (auto &d : dag) {           // remove duplicate arrows
            sort(d.begin(), d.end());
            d.erase(unique(d.begin(), d.end()), d.end());
        }
    }
};
```

## Examples

**Is the whole graph one group?** (everyone reaches everyone)
```cpp
if (s.groups == 1) cout << "YES\n";
```

**Collect the most coins** (CSES Coin Collector): sum the coins inside each group, then take the best path through the squashed graph.
```cpp
vector<long long> coins(s.groups, 0), best(s.groups, 0);
for (int v = 1; v <= n; v++) coins[s.comp[v]] += k[v];

for (int g = s.groups - 1; g >= 0; g--) {      // against the arrows: best path starting at g
    best[g] = coins[g];
    for (int h : s.dag[g]) best[g] = max(best[g], coins[g] + best[h]);
}
cout << *max_element(best.begin(), best.end()) << '\n';
```

**Groups with no arrows coming in or going out**
```cpp
vector<int> in(s.groups, 0);
for (int g = 0; g < s.groups; g++)
    for (int h : s.dag[g]) in[h]++;
// in[g] == 0       → nothing outside points into g
// s.dag[g].empty() → g points to nothing outside
```

## Things to remember

- Call `build()` once, after adding all edges.
- Both DFS passes use an explicit stack, so a chain of 2·10⁵ vertices is safe even with a small stack.
- Cost: O(n + m). Memory: each arrow is stored twice (normal and flipped).
- Group 0 is a group nothing points into; group `groups - 1` points to nothing.
