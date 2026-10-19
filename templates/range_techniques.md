# Range Techniques (no segment tree)

There are really only **three core ideas**:

1. **Prefix array** — cheap range *queries*
2. **Difference array** — cheap range *updates*
3. **Slope trick** — when the amount depends on the position `i`, split it into
   **fixed part + something · i** and handle each part with 1 or 2

Everything else (arithmetic / geometric adds, weighted sums, right-to-left versions)
is one of these three applied to a different pattern.

All arrays are 1-based. Use `long long`.

---

## 1. Prefix array — range queries

```cpp
P[0] = 0;
for (int i = 1; i <= n; i++) P[i] = P[i - 1] + a[i];
// sum of a[l..r]
P[r] - P[l - 1]
```

**Example:** `a = 3 1 4 1 5` → `P = 0 3 4 8 9 14`.
Sum of 2..4 = `P[4] − P[1] = 9 − 3 = 6` (= 1 + 4 + 1).

- Works for anything that can be undone: sum, xor (`P[r] ^ P[l-1]`), counts.
- Does **not** work for min / max (you can't "subtract" a min).
- The array must not change after building.

---

## 2. Difference array — range updates

`d[i]` = how much the array changes from position `i−1` to `i`.

```cpp
// build from an existing array (a[0] = 0)
d[i] = a[i] - a[i - 1];

// add c to a[l..r]: only the two edges of the range change
d[l] += c;
d[r + 1] -= c;

// rebuild: running sum
for (int i = 1; i <= n; i++) d[i] += d[i - 1];   // d[i] = final value / total added at i
```

**Example (n = 5):** `+5 on 2..4`, then `+3 on 1..2`

| i | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|
| marks | +3 | +5 | −3 | 0 | −5 |
| running sum | 3 | 8 | 5 | 5 | 0 |

- All updates must come **before** reading.
- Simplest: keep `d` only for the updates, then `final[i] = a[i] + added[i]`.
- Prefix and difference undo each other: running sum ↔ neighbour differences.
- Chain them: updates with `d` → running sum = values → running sum again = prefix for range queries.

---

## 3. Slope trick — add an arithmetic progression to a range

Add `s, s+k, s+2k, …` to `a[l..r]`.

Position `i` gets `s + (i − l)·k`  =  **`(s − k·l)  +  k · i`**

Both `(s − k·l)` and `k` are the same for every cell → two ordinary difference arrays.

```cpp
vector<ll> cst(n + 2), slope(n + 2);

auto addAP = [&](int l, int r, ll s, ll k) {   // l gets s, l+1 gets s+k, ...
    ll base = s - k * l;
    cst[l]   += base;  cst[r + 1]   -= base;
    slope[l] += k;     slope[r + 1] -= k;
};

for (int i = 1; i <= n; i++) {
    cst[i]   += cst[i - 1];
    slope[i] += slope[i - 1];
    added[i] = cst[i] + slope[i] * i;
}
```

**Example (n = 6):** add `1,2,3` on 2..4 and `10,20` on 4..5

| i | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| cst | 0 | −1 | −1 | −31 | −30 | 0 |
| slope | 0 | 1 | 1 | 11 | 10 | 0 |
| cst + slope·i | 0 | 1 | 2 | 13 | 20 | 0 |

(`cst` alone can look strange; only `cst + slope·i` must be right.)

### Right to left (reverse)

`r` gets `s`, `r−1` gets `s+k`, …, `l` gets `s + (r−l)·k`.

Position `i` gets `s + (r − i)·k`  =  **`(s + k·r)  +  (−k) · i`**

```cpp
auto addAPfromRight = [&](int l, int r, ll s, ll k) {
    ll base = s + k * r, sl = -k;
    cst[l]   += base;  cst[r + 1]   -= base;
    slope[l] += sl;    slope[r + 1] -= sl;
};
```

**Example:** `3,2,1` on 2..4 (`s = 1, k = 1, r = 4`): base 5, slope −1 → `5−2, 5−3, 5−4` = `3, 2, 1`.

**Rule:** never memorise directions — write the value at `i` as
`something + something·i` and read off base and slope.
(2276E: added `B − i` → base `B`, slope `−1`.)

Polynomial adds (`i²`, …): same idea, one extra difference array per power of `i`.

---

## 4. Geometric progression add (difference array, different rebuild rule)

Add `c, c·q, c·q², …` to `a[l..r]` (same `q` for all updates, usually mod p).

The running sum must **multiply** the carried value by `q` each step:

```cpp
// mark
d[l]     += c;
d[r + 1] -= c * q^(r - l + 1);      // the value that would carry past r

// rebuild
for (int i = 1; i <= n; i++) g[i] = g[i - 1] * q + d[i];   // all mod p
```

**Example (q = 2):** `1,2,4` on 2..4 and `3,6` on 3..4
→ `d = [0, 1, 3, 0, −8−12, 0]`, `g = 0 1 5 10 0 0`.

- Cancel amount uses `q^(length)`, not `q^(length−1)`.
- Different ratios → one array per ratio.
- Alternative (slope-trick style): `c·q^(i−l) = (c·q^(−l)) · q^i` → constant add of `c·q^(−l)`,
  multiply by `q^i` at the end (needs modular inverse).

**General pattern:** a rule that builds each cell from the previous one; start the pattern
at `l`, cancel whatever would carry past `r`.

---

## 5. Weighted range sum (slope trick on the query side)

Query: `a[l]·1 + a[l+1]·2 + … + a[r]·(r−l+1)`, array fixed.

Weight of `a[i]` is `i − (l−1)` → keep two prefix arrays:
`P` = prefix of `a[i]`, `Q` = prefix of `a[i]·i`.

```cpp
for (int i = 1; i <= n; i++) {
    P[i] = P[i - 1] + a[i];
    Q[i] = Q[i - 1] + a[i] * i;
}
// weights 1,2,3,... counted from l
(Q[r] - Q[l - 1]) - (ll)(l - 1) * (P[r] - P[l - 1])
// weights 1,2,3,... counted from r   (weight = (r+1) - i)
(ll)(r + 1) * (P[r] - P[l - 1]) - (Q[r] - Q[l - 1])
```

**Example:** `a = 3 1 4 1 5`, `l = 2, r = 5`, so `P = 0 3 4 8 9 14`, `Q = 0 3 5 17 21 46`
- from l: `1·1 + 4·2 + 1·3 + 5·4 = 32` → `(46−3) − 1·(14−3) = 32` ✓
- from r: `4·1 + 3·4 + 2·1 + 1·5 = 23` → `6·11 − 43 = 23` ✓

Slope trick splits **what you add**; this splits **what you multiply by**.
Square weights → add a prefix of `a[i]·i²`.

---

## When to stop and use a data structure

- Updates and queries mixed → Fenwick tree (constant add + range sum).
- Min / max / assignment mixed with updates → lazy segment tree.
