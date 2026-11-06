# Segment Tree over DP ("dynamic DP")

**Use when:** a left-to-right DP with few states per position + **point updates** (or range queries) on the array.

---

## The one test
> Can I describe a segment by a small table — **the best answer for every combination of states at its two ends** — such that once those end states are fixed, the inside of the segment **no longer depends on anything outside it**?

If yes, and merging two tables is cheap → segment tree of tables. A point update = new leaf + re-merge up to the root (log n merges).

Short version to ask on any problem with point updates and a left-to-right dependency (even if it doesn't look like DP):
> **Can I summarize an entire interval by how it behaves at its boundaries?**

## Checklist
1. Write a left-to-right DP with **few states**.
2. Each step depends only on the **previous state** + the current element.
3. Node = table over the **interface states** (what a segment must tell its neighbours at each end) — call their number S. Often S = number of DP states (Chain Chips: used/skipped), but not always: the summary can be different (max subarray stores sum/pref/suf/best instead).
4. Merge = combine the two children's summaries using **only what they store**. For endpoint tables: try every compatible pair of states at the join and keep the best. For other summaries (like max subarray's sum/pref/suf/best): derive the matching formula.
5. **Cost — always compute the real count:** total ≈ (n + q · log n) · (cost of one merge) — n merges to build, log n per update.
   - Endpoint table, naive merge: S⁴ per merge (two outer states × two join states).
   - Transition matrix (or the endpoint merge done in two steps — first fold the join pair, then the other side): S³ per merge.
   - With q = 2·10⁵ and ~18 merges per update (rough operation counts, not exact runtimes):

     | S | naive S⁴ | two-step S³ | verdict |
     |---|---|---|---|
     | 2 | 16 → ~6·10⁷ | 8 → ~3·10⁷ | comfortable |
     | 3 | 81 → ~3·10⁸ | 27 → ~10⁸ | naive risky, S³ fine |
     | 5 | 625 → ~2·10⁹ | 125 → ~4.5·10⁸ | both possibly too slow |
6. **Custom merge? Check associativity:** a custom merge is associative when it correctly computes the summary of the two segments glued together **and** the summary keeps everything needed for future merges. Then (A·B)·C and A·(B·C) both describe the same interval and compute the same optimal summary. A genuine matrix product over an associative semiring is associative automatically.

## Why it's correct — two proofs (both work)
**My proof (constant shift):** fix the state at the boundary. Then everything before it is just **one constant added to every value after it**, because min and + let a constant pass straight through: min(x + C, y + C) = min(x, y) + C. So the best choice inside the right segment never depends on the left one → it can be precomputed.
- This is why the **end states must be fixed**: with both states open, the left side hands over two different constants, and min(x + C₁, y + C₂) is not a single shift. So the table stores each end state separately and the merge takes the min.
- **Scope:** this argument is for optimisation DPs (min/+ or max/+). For counting DPs the prefix enters as a **multiplier** instead (count = left count × right count for fixed join states), and for reachability it's AND/OR — the "can't do better + can reach it" style (independence of the two halves) is the proof that covers every case.

**Can't do better + can reach it:** fix the two states at the join.
- Cost of the whole = cost of left part + cost of right part.
- Every rule is inside the left, inside the right, or only about the join states.
- The right part of any answer obeys all of the right segment's rules (they're a subset of the whole's rules) → it's one of the patterns the right table minimised over → costs ≥ the stored best. Same for left. **Can't do better.**
- Gluing the left's best and the right's best is valid (join allowed) → **reaches it.**

## Which operations
| Problem | combine choices | combine consecutive parts |
|---|---|---|
| minimum cost | min | + |
| maximum score | max | + |
| counting (mod p) | + | × |
| reachable? | OR | AND |

## Two representations (same idea, different meaning of the entries)
| | Endpoint table `D[a][b]` | Transition matrix `M[u][v]` |
|---|---|---|
| entry means | best inside the segment, given the state **of its first and last element** | best way to go **from incoming DP state u to outgoing state v** across the segment |
| merge | try join pairs (c, d), skip forbidden ones: `L[a][c] + R[d][b]` | matrix product with min as + and + as ×: `min over k of A[u][k] + B[k][v]` |
| example | Chain Chips (used/skipped edges) | Clean Substrings (counting version, 5 × 5, + and ×) |

A genuine matrix product over an associative semiring is always associative. For an endpoint table, check point 6 of the checklist.

## When it breaks (or gets expensive)
- **The cost of a piece depends on the whole piece**, and pieces can be any length (max − min of a piece, number of distinct values…). The boundary would need to remember an unfinished piece → too many states.
  → e.g. **ABC480 G Zonal Score Minimization**: there the segment tree is used differently — over the **DP values** (range add + range min), to speed up "min over all earlier cut points".
- **Whole-array rule** like "exactly k chosen": put the count in the state → tables k × k, merge ~k² or more. Fine only for small k.
- **Operations that don't let a constant pass through** (e.g. XOR mixed with max): the constant-shift argument fails — look for a *different* small summary before giving up.

**Quick way to tell the two segment-tree-DP kinds apart:**
- each DP step looks at a **fixed few** previous states → tables / matrices (this note)
- each DP step takes a min over a **whole range** of earlier positions → segment tree over DP values (Zonal)

---

## Worked example: [1814E – Chain Chips](https://codeforces.com/problemset/problem/1814/E) (2300)

**Reduction (chips disappear):**
- Every edge is crossed an even number of times (chips left of it: same count at start and end).
- Edge 1 and edge n−1 must be crossed; two neighbouring edges can't both be uncrossed (the chip between them is stuck).
- Any such set is reachable: skipped edges cut the line into groups of ≥ 2 vertices; rotate chips in each group → each used edge crossed exactly twice.
- **Answer = 2 × min sum of used edges, edge 1 and n−1 used, no two skipped edges adjacent.**

**One-pass DP:** take[i] = w[i] + min(take[i−1], skip[i−1]), skip[i] = take[i−1].

**Table (S = 2):** best[a][b] = cheapest inside the segment, a/b = first/last edge used (1) / skipped (0).
- leaf: best[1][1] = w, best[0][0] = 0, mixed = ∞
- merge: best[a][b] = min over (c, d) ≠ (0, 0) of L[a][c] + R[d][b]
- answer: 2 · root.best[1][1]

```cpp
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll INF = 4e18;

struct Node { ll best[2][2]; };
int n, m;
vector<Node> tr;

Node leaf(ll w) {
    Node x;
    x.best[1][1] = w; x.best[0][0] = 0;
    x.best[0][1] = x.best[1][0] = INF;
    return x;
}

Node merge(const Node &L, const Node &R) {
    Node x;
    for (int a = 0; a < 2; a++)
        for (int b = 0; b < 2; b++) {
            x.best[a][b] = INF;
            for (int c = 0; c < 2; c++)              // L's last edge
                for (int d = 0; d < 2; d++) {        // R's first edge
                    if (c == 0 && d == 0) continue;  // two skips touching
                    if (L.best[a][c] == INF || R.best[d][b] == INF) continue;
                    x.best[a][b] = min(x.best[a][b], L.best[a][c] + R.best[d][b]);
                }
        }
    return x;
}

void build(int v, int l, int r, const vector<ll> &w) {
    if (l == r) { tr[v] = leaf(w[l]); return; }
    int mid = (l + r) / 2;
    build(2 * v, l, mid, w); build(2 * v + 1, mid + 1, r, w);
    tr[v] = merge(tr[2 * v], tr[2 * v + 1]);
}

void update(int v, int l, int r, int pos, ll val) {
    if (l == r) { tr[v] = leaf(val); return; }
    int mid = (l + r) / 2;
    if (pos <= mid) update(2 * v, l, mid, pos, val);
    else update(2 * v + 1, mid + 1, r, pos, val);
    tr[v] = merge(tr[2 * v], tr[2 * v + 1]);
}

int main() {
    scanf("%d", &n); m = n - 1;
    vector<ll> w(m);
    for (auto &x : w) scanf("%lld", &x);
    tr.resize(4 * m);
    build(1, 0, m - 1, w);
    int q; scanf("%d", &q);
    while (q--) {
        int k; ll x; scanf("%d %lld", &k, &x);
        update(1, 0, m - 1, k - 1, x);
        printf("%lld\n", 2 * tr[1].best[1][1]);
    }
}
```

---

## Takeaway
Make each segment **self-contained**: it stores its best answer for every possible state at its two ends, so it never needs to know what its neighbours contain. Fix the states at a join and the two sides become independent — that's why one merge per level is enough.

Three levels:
1. **Recognise:** can I summarise an interval by its boundary behaviour?
2. **Prove:** fix the boundary states → neither side can beat its stored best, and gluing the bests reaches it (or: the left side is just a constant shift).
3. **Implement:** summaries in a segment tree, associative merge, recompute only the ancestors of the updated leaf.
