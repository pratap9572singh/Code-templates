# 2-SAT — Black-Box Reference

What it solves, how to phrase constraints for it, and the template.
The same code is in `2sat.cpp`, ready to paste.

---

## 1. When does 2-SAT apply?

Both conditions must hold.

1. **Every object has exactly two states.** One boolean per object. Three or more choices per object and 2-SAT does not apply.
2. **Every single condition mentions at most two objects.** This is per condition, not overall. You can have 200,000 booleans; what matters is that no single condition needs three of them at once.

The precise test: can the condition be written as an OR of at most two literals? A literal is "x is true" or "x is false".

A condition about many objects is fine if it splits into one-object facts. "No treasure anywhere in positions 5 to 90" is 86 separate statements joined by AND, so it becomes 86 clauses. What does not split is an OR over three or more: "treasure at p, or at q, or at r" is 3-SAT, and there is no polynomial algorithm for it.

> **Rule of thumb: ANDs are free, ORs are capped at two.**

---

## 2. Translating conditions into clauses

A clause is an OR of literals that must come out true. Restate every condition as "at least one of these literals is true".

| Condition | Clause | Code |
|---|---|---|
| At least one of u, v | u OR v | `addClause(lit(u, true), lit(v, true))` |
| Not both u and v | ¬u OR ¬v | `addClause(lit(u, false), lit(v, false))` |
| If u then v | ¬u OR v | `addClause(lit(u, false), lit(v, true))` |
| u must be true | u | `addUnit(lit(u, true))` |
| u must be false | ¬u | `addUnit(lit(u, false))` |
| Exactly one of u, v (they differ) | at least one, and not both | the first two rows together |
| u and v agree | ¬u OR v, and u OR ¬v | `addClause(lit(u, false), lit(v, true))` and `addClause(lit(u, true), lit(v, false))` |
| At most one of a group | not-both for every pair | `atMostOne({...})`, linear instead of quadratic |

Two that trip people up. **Not both** means at least one of them is false, which is the OR of the two negative literals. **If u then v** is broken only when u is true and v is false, which is exactly when ¬u OR v fails, so they are the same statement.

---

## 3. How to use the template

Four steps, all in your solve function. You never touch the internals.

**Step 1 — name the boolean.** Write down what "true" means: "treasure at island i", "item i goes left", "node i is in the set". Ambiguity here causes every later bug.

**Step 2 — construct the solver.**

```cpp
TwoSat ts(n);  // n booleans, indexed 0 .. n-1
```

**Step 3 — emit one call per condition.**

```cpp
ts.addClause(lit(u, true),  lit(v, true));   // at least one of u, v
ts.addClause(lit(u, false), lit(v, false));  // not both u and v
ts.addClause(lit(u, false), lit(v, true));   // if u then v
ts.addUnit(lit(u, true));                    // u must be true
ts.addUnit(lit(u, false));                   // u must be false
ts.atMostOne({lit(a, true), lit(b, true), lit(c, true)});  // at most one of a, b, c
```

`lit(i, true)` is "boolean i is true" and `lit(i, false)` is its negation. `x ^ 1` negates any literal `x`. Order inside a clause does not matter.

**Step 4 — solve and read the result.**

```cpp
string res;
if (!ts.solve(res)) { /* impossible */ }
// res[i] == '1' means boolean i is true
```

**Global conditions stay outside.** "At least one true overall", "exactly k true", "maximise the count" cannot be written as clauses. The solver returns *some* valid assignment, not the best one, so if that assignment fails your global check, it does **not** prove the answer is impossible. Handle these by forcing part of the choice with `addUnit` and re-running, for example inside a binary search on the answer.

---

## 4. Complexity

| | Cost |
|---|---|
| Time | O(V + C), V = booleans, C = clauses |
| Space | O(V + C) |
| Nodes built internally | 2V (one per literal) |
| Edges built internally | 2C (each clause becomes two implications) |

Measured with V = C = 10⁶: about 0.5 s and 120–150 MB. Fine under 256 MB; check first if the limit is smaller.

The real risk is clause count, not the algorithm. If a condition naively becomes one clause per pair ("this index conflicts with everything in [l, r]"), you get O(n²) clauses and time out. If the range only forces indices, use a difference array to find them and emit one unit clause per forced index. If it is "at most one of this group", use `atMostOne`, which needs about 3k clauses instead of k².

---

## 5. The template

Tarjan is written iteratively because a recursive version overflows the stack on large inputs.

```cpp
// 2-SAT, iterative Tarjan. Full notes: 2sat-reference.md
// Needs: #include <bits/stdc++.h> and using namespace std;
//
// Usage:
//   TwoSat ts(n);                                     // booleans 0 .. n-1
//   ts.addClause(lit(u, true), lit(v, false));        // (u OR not v)
//   ts.addUnit(lit(u, true));                         // u must be true
//   ts.atMostOne({lit(a, true), lit(b, true), lit(c, true)});
//   string res;
//   if (!ts.solve(res)) { /* impossible */ }          // res[i] == '1' -> i is true

// literal ids: 2*i = "x_i is true", 2*i+1 = "x_i is false"; negate a literal with x ^ 1
int lit(int i, bool val) { return 2 * i + (val ? 0 : 1); }

struct TwoSat {
    int n;
    vector<vector<int>> g;  // implication graph on 2n literal nodes

    TwoSat(int n) : n(n), g(2 * n) {}

    void addClause(int a, int b) {  // (a OR b)
        g[a ^ 1].push_back(b);      // if a is false, b must hold
        g[b ^ 1].push_back(a);      // if b is false, a must hold
    }
    void addUnit(int a) { addClause(a, a); }  // a must hold

    int addVar() { g.emplace_back(); g.emplace_back(); return n++; }  // new helper boolean

    // at most one literal in ls is true; about 3k clauses instead of k^2 pairs
    void atMostOne(const vector<int> &ls) {
        if (ls.size() <= 1) return;
        int pre = ls[0];  // "some literal so far is true"
        for (int j = 1; j < (int)ls.size(); j++) {
            int nxt = lit(addVar(), true);
            addClause(pre ^ 1, ls[j] ^ 1);  // pre -> not ls[j]
            addClause(pre ^ 1, nxt);        // pre -> nxt
            addClause(ls[j] ^ 1, nxt);      // ls[j] -> nxt
            pre = nxt;
        }
    }

    bool solve(string &res) {
        int N = 2 * n;
        vector<int> num(N, 0), low(N, 0), comp(N, -1), idx(N, 0), stk, call;
        int timer = 0, nc = 0;
        for (int s = 0; s < N; s++) {
            if (num[s]) continue;
            call.push_back(s);
            while (!call.empty()) {
                int v = call.back();
                if (!num[v]) { num[v] = low[v] = ++timer; stk.push_back(v); }
                bool down = false;
                while (idx[v] < (int)g[v].size()) {
                    int u = g[v][idx[v]++];
                    if (!num[u]) { call.push_back(u); down = true; break; }
                    else if (comp[u] == -1) low[v] = min(low[v], num[u]);
                }
                if (down) continue;
                call.pop_back();
                if (low[v] == num[v]) {
                    while (true) {
                        int u = stk.back(); stk.pop_back();
                        comp[u] = nc;
                        if (u == v) break;
                    }
                    nc++;
                }
                if (!call.empty()) low[call.back()] = min(low[call.back()], low[v]);
            }
        }
        res.assign(n, '0');
        for (int i = 0; i < n; i++) {
            if (comp[2 * i] == comp[2 * i + 1]) return false;  // x_i and not x_i imply each other
            if (comp[2 * i] < comp[2 * i + 1]) res[i] = '1';   // Tarjan numbering: '<' means true
        }
        return true;
    }
};
```

Worth knowing even as a black-box user:

- **Failure line:** `comp[2i] == comp[2i+1]`. A boolean and its negation force each other, so neither value works.
- **Assignment line:** `comp[2i] < comp[2i+1]` means true. The direction is Tarjan-specific; with a Kosaraju template it flips.
- **Unconstrained booleans** (no clause mentions them) come out true, since node 2i is visited before node 2i+1.
- **`atMostOne` adds helper booleans** after your n. `res` includes them at the end; just ignore those positions.

---

## 6. Problem patterns that are secretly 2-SAT

| Disguise | The boolean |
|---|---|
| Each item goes in one of two positions | item i takes position A |
| Each element is kept or removed | element i is kept |
| Each interval is placed left or right of a point | interval i goes left |
| Each edge is oriented one way or the other | edge i points forward |
| Each person is assigned to one of two groups | person i is in group A |
| Each value is flipped or left alone | value i is flipped |

Once the boolean is named, the constraints usually read off directly: "these two cannot both be in group A" is a not-both clause, and "if this one goes left then that one must too" is an implication.

**At most one of three or more is fine**: use `atMostOne`. Only **at least one of three or more** breaks, since it needs three literals in a single OR. So if the problem's structure already gives you the at-least-one part and only asks you to encode exclusions, you are still inside 2-SAT.

---

## 7. The shortcut — check before reaching for the template

Look at your finished clause list. If every two-literal clause is all-positive ("at least one of these is true") and negations only appear alone as unit clauses, you do not need the solver.

Set every boolean true except the ones forced false, then verify. This works whenever any assignment works, because making more booleans true can only help positive clauses and cannot disturb the forced ones.

The mirror case works the same way: all-negative two-literal clauses with only positive units, so set everything false except what is forced true.

You genuinely need the solver when some clause **mixes** a positive and a negative literal. That is where one choice propagates into others.

---

## 8. Pitfalls

| Pitfall | What happens | Fix |
|---|---|---|
| Adding only one implication edge per clause | No crash, silently wrong answers | Always both directions (`addClause` already does it) |
| Quadratic clause generation | Time limit on ranges and windows | Difference array, or `atMostOne` for groups |
| Recursive Tarjan | Stack overflow on large inputs | Use the iterative version above |
| 1-indexed input | Out-of-range crash or wrong boolean | Construct with `n + 1`, or subtract 1 |
| Trusting one assignment for a global condition | Reports "impossible" when it is not | Force part of the choice and re-run |
| Porting the assignment line to another template | Assignment comes out inverted and breaks clauses | Direction depends on which SCC algorithm numbered the components |
