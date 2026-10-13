# DSU (Disjoint Set Union)

Keeps elements in groups and merges groups. Every operation is almost O(1).

**Reach for it when:**
- you add edges and keep asking "are these two connected?"
- you need the number of connected parts, or the size of a part
- Kruskal (minimum spanning tree)
- you only ever **merge**, never split (splitting needs a different tool)

## Functions

| call | does |
|---|---|
| `DSU d(n)` | creates nodes `0..n`, so 1-based and 0-based both work |
| `d.find(x)` | root of x's group |
| `d.unite(a, b)` | merges the two groups; `true` if they were different, `false` if already together |
| `d.same(a, b)` | are a and b in the same group? |
| `d.size(x)` | number of elements in x's group |
| `d.components()` | number of groups among the n real nodes |

## Code

```cpp
struct DSU {
    vector<int> parent, sz;
    int comps;

    DSU(int n) : parent(n + 1), sz(n + 1, 1), comps(n) {
        for (int i = 0; i <= n; i++) parent[i] = i;
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);      // point x straight at the root
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;                // already in the same group
        if (sz[a] < sz[b]) swap(a, b);           // a is the bigger group
        parent[b] = a;                           // hang the smaller under the bigger
        sz[a] += sz[b];
        comps--;
        return true;
    }

    bool same(int a, int b) { return find(a) == find(b); }

    int size(int x) { return sz[find(x)]; }      // sz is correct only at the root

    int components() { return comps; }
};
```

## Usage

**Connected parts and cycle check**
```cpp
DSU d(n);
for (int i = 0; i < m; i++) {
    int u, v; cin >> u >> v;
    if (!d.unite(u, v)) {
        // u and v were already connected: this edge closes a cycle
    }
}
cout << d.components() << '\n';
cout << d.size(1) << '\n';               // size of the part containing 1
```

**Kruskal**
```cpp
vector<array<long long, 3>> edges;       // {w, u, v}, weight first so sort works
sort(edges.begin(), edges.end());
long long mst = 0;
for (auto [w, u, v] : edges)
    if (d.unite(u, v)) mst += w;
// if d.components() > 1, the graph is not connected
```

**Members of every group**
```cpp
map<int, vector<int>> groups;
for (int v = 1; v <= n; v++) groups[d.find(v)].push_back(v);
```

## Things to remember

- `sz[x]` is only correct when `x` is a root. Always call `d.size(x)`, never read `sz[x]` directly.
- Compare roots, not nodes: `d.same(a, b)`, not `d.find(a) == b`.
- Merging the smaller group under the bigger keeps trees shallow, so the recursive `find` is safe even for 10⁶ nodes in a chain.
- Pass it by reference to functions: `void solve(DSU &d)`. Without the `&` the whole thing is copied on every call.
- Queries offline with a "weight ≤ k" condition (like 1213G): process edges in weight order and merge as you go; answers change only when a merge happens.
