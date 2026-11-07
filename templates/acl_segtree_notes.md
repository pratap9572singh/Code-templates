# AtCoder Library — `segtree` and `lazy_segtree`

## Setup

- Needs **C++14 or newer** (works with C++17 / 20 / 23).
- **AtCoder:** pre-installed → `#include <atcoder/all>`.
- **Codeforces / others:** not installed → paste it in (ACL's `expander.py` inlines headers into one file).
- All ranges are **half-open `[l, r)`**. 1-based inclusive `[l, r]` → call with `(l-1, r)`.

---

## 1. `segtree<S, op, e>`

You only answer three questions:

| | meaning |
|---|---|
| `S` | what a node stores |
| `S op(S a, S b)` | merge left block `a` with right block `b` |
| `S e()` | value that changes nothing when merged (0 for sum, +INF for min, -INF for max) |

```cpp
#include <atcoder/segtree>
using namespace atcoder;

int op(int a, int b) { return min(a, b); }
int e() { return INT_MAX; }

segtree<int, op, e> seg(n);     // all e()
segtree<int, op, e> seg(vec);   // from a vector<S>

seg.set(p, x);        // a[p] = x
seg.get(p);           // a[p]
seg.prod(l, r);       // op over [l, r)
seg.all_prod();       // op over everything
```

### Rules for `op` / `e`
1. Grouping doesn't matter: `op(op(a,b),c) == op(a,op(b,c))`. Order *may* matter (left is always merged before right).
2. `op(e(), a) == op(a, e()) == a`.

---

## 2. Binary search on the tree: `max_right` / `min_left`

```cpp
int r = seg.max_right(l, check);   // grow right from l
int l = seg.min_left(r, check);    // grow left from r
```

- `check` takes **exactly one argument of type `S`** (the merged value of the range) and returns `bool` = "is this range still fine?"
- Extra values (`k`, `v`) come in through **lambda capture**, never as extra arguments. Extra info about the range goes **inside `S`**.
- Use the **lambda form** `max_right(l, lambda)`. The `max_right<check>(l)` form only accepts a global function (no captures).

### Rules for `check`
1. `check(e())` must be true (the search starts from an empty range).
2. Once false, it stays false as the range grows. This lets the tree take whole blocks at once → O(log n).

### What comes back

| call | result | element that broke it | none found |
|---|---|---|---|
| `max_right(l, check)` | largest `r` with `check(prod(l, r))` true | `a[r]` | `r == n` |
| `min_left(r, check)` | smallest `l` with `check(prod(l, r))` true | `a[l-1]` | `l == 0` |

`min_left` excludes `r`: for "at or before index `i`" pass `r = i + 1`.

### Examples

```cpp
// max tree: first index >= l with a[i] >= v
int r = seg.max_right(l, [&](int x) { return x < v; });
if (r < n) { /* a[r] is it */ }

// max tree: last index < r with a[i] >= v
int l = seg.min_left(r, [&](int x) { return x < v; });
if (l > 0) { /* a[l-1] is it */ }

// sum tree, all a[i] >= 0: first point where a[l] + ... + a[r] >= k
int r = seg.max_right(l, [&](ll s) { return s < k; });
```

With negative numbers the sum check breaks rule 2. Don't use it there.

---

## 3. `lazy_segtree<S, op, e, F, mapping, composition, id>`

Range update + range query. Think of it as two levels:
- `F` is an **operation on each array element** ("add 5", "set to 7").
- `mapping` is a **shortcut**: if every element in this block got `f`, what is the block's new summary?

| | meaning |
|---|---|
| `S`, `op`, `e` | same as `segtree` |
| `F` | the update |
| `S mapping(F f, S x)` | node value after applying `f` to its whole block |
| `F composition(F f, F g)` | **`g` applied first, then `f`**, as one update |
| `F id()` | the update that does nothing |

```cpp
#include <atcoder/lazysegtree>

lazy_segtree<S, op, e, F, mapping, composition, id> seg(init);
seg.apply(l, r, f);   // update [l, r)
seg.apply(p, f);      // single position
seg.prod(l, r);       // query [l, r)
// set, get, all_prod, max_right, min_left: same as segtree
```

### How it works
An update covers `[l, r)` with O(log n) whole nodes. Each one fixes its own value (`mapping`) and keeps a note for its children. A node that already has a note merges the old and new one (`composition`). A note is passed down to the children only when a later operation needs to go inside that node.

### Rules
1. `op` grouping doesn't matter; `e` changes nothing (as above).
2. `mapping(id(), x) == x`.
3. **Update a merged block = merge the updated parts:**
   `mapping(f, op(a, b)) == op(mapping(f, a), mapping(f, b))`
4. **Combined note = both in order:**
   `mapping(composition(f, g), x) == mapping(f, mapping(g, x))`
5. `F` has a fixed size (combining notes must not grow them).

**If rule 3 fails:** something is missing from `S`. Add it (usually `len`). If no extra field can fix it, ACL is the wrong tool (see section 5).

### Why ACL needs `len` and hand-written code doesn't
A hand-written tree knows each node's range `[nl, nr)` from the recursion. ACL's `mapping` only sees the stored value, so the size must live in `S`. Leaves start with `len = 1`; `op` adds them up.

---

## 4. Ready-made examples

### Range add, range sum

```cpp
struct S { ll sum; int len; };
S op(S a, S b) { return {a.sum + b.sum, a.len + b.len}; }
S e() { return {0, 0}; }

using F = ll;
S mapping(F f, S x) { return {x.sum + f * x.len, x.len}; }
F composition(F f, F g) { return f + g; }
F id() { return 0; }

vector<S> init(n);
for (int i = 0; i < n; i++) init[i] = {a[i], 1};   // len = 1 per leaf!
lazy_segtree<S, op, e, F, mapping, composition, id> seg(init);

seg.apply(l - 1, r, x);              // 1-based [l, r]
cout << seg.prod(l - 1, r).sum;
```

### Range add, range max

```cpp
ll op(ll a, ll b) { return max(a, b); }
ll e() { return LLONG_MIN; }
ll mapping(ll f, ll x) { return x == LLONG_MIN ? x : x + f; }  // keep the empty value empty
ll composition(ll f, ll g) { return f + g; }
ll id() { return 0; }
```
No `len` needed: `max(a+f, b+f) = max(a,b) + f`.

### Range set, range min

```cpp
ll op(ll a, ll b) { return min(a, b); }
ll e() { return LLONG_MAX; }

struct F { bool has; ll v; };          // flag = safe for any v
ll mapping(F f, ll x) { return f.has ? f.v : x; }
F composition(F f, F g) { return f.has ? f : g; }   // newer set wins
F id() { return {false, 0}; }
```
`id()` can't be 0: "set to 0" is a real update. A special value (e.g. `LLONG_MIN`) works only if it can never be a real `v`.

### Range multiply-then-add (`x → b*x + c`), range sum, mod 998244353

```cpp
#include <atcoder/modint>
using mint = modint998244353;

struct S { mint sum; int len; };
S op(S a, S b) { return {a.sum + b.sum, a.len + b.len}; }
S e() { return {0, 0}; }

struct F { mint b, c; };
S mapping(F f, S x) { return {f.b * x.sum + f.c * x.len, x.len}; }
F composition(F f, F g) { return {f.b * g.b, f.b * g.c + f.c}; }  // g first, then f
F id() { return {1, 0}; }
```
Covers multiply (`b=k, c=0`), add (`b=1, c=k`) and set (`b=0, c=v`) in one type.
Practice: ACL Practice Contest **K – Range Affine Range Sum**.

---

## 5. When ACL is not enough

| problem | why |
|---|---|
| `x = min(x, v)` / sum, `x = √x`, `x = x mod m` | rule 3 fails and no field fixes it; needs "go deeper when stuck" → hand-written (Segment Tree Beats) |
| query an old version of the array | needs a persistent tree |
| indices up to 1e9, online | compress coordinates if you can read everything first; else a tree that creates nodes on demand |
| nodes storing sorted lists | `op` copies → too slow |
| merging two trees | ACL trees have fixed shape |
| best line at x | Li Chao tree |
| search where "fine" can come back after failing | walk the tree by hand |

### `min(x, v)` / sum in one line
Store sum, max, count of max, second max. At a fully covered node: `v ≥ max` → stop; `second max < v < max` → `sum -= cnt*(max-v)`, `max = v`, stop; otherwise go into children. Total O((n+q) log n).

---

## 6. Gotchas

- `seg(n)` gives every leaf `e()`. With `len` in `S` that means `len = 0`, so adds do nothing. Build from `init` with `len = 1`.
- 1-based `[l, r]` → `(l-1, r)`. Only `l` changes.
- `composition(f, g)`: **`g` is the older update**. Test with two different updates on the same range against brute force.
- `check` lambda argument type must be `S` (e.g. `ll`, not `int`, for big sums).
- A lambda stored in a variable needs `};`.
- `max_right` returns `n` / `min_left` returns `0` when nothing is found; check before indexing.
