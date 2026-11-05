# Trie

A trie stores a set of **sequences** (words, or numbers written as bits) so that sequences with the **same beginning share the same path**.

- **One node = one prefix.** Every word below a node starts with that prefix.
- **A count at a node = how many stored items share that prefix.** Counts are what make deletion, counting and "k-th" queries work.

```
words: cat, car, cart, dog

             root  pass=4
            /          \
        c  pass=3      d  pass=1
           |              |
        a  pass=3      o  pass=1
         /     \          |
   t pass=1   r pass=2   g pass=1  ends=1
     ends=1     ends=1
                 |
               t pass=1  ends=1
```

---

## How to design one (decide these 4 things first)

### 1. What is one step of the path? → size of the children array
| Items stored | One step | Children per node | `nxt` type |
|---|---|---|---|
| numbers, bit by bit | one bit | 2 | `vector<array<int, 2>>` |
| lowercase words | one letter | 26 | `vector<array<int, 26>>` |
| digit strings | one digit | 10 | `vector<array<int, 10>>` |
| any characters (big alphabet) | one char | only the ones used | `vector<map<char, int>>` (slower, much less memory) |

`nxt[v][c]` = the node you reach from node v by step c. **0 means "no child".** That works because node 0 is the root, and the root is never anyone's child.

### 2. What counts does each node need?
| Count | Meaning | Needed for |
|---|---|---|
| `pass[v]` | how many stored items go **through** v | deletion, "how many start with this prefix", XOR counting, k-th |
| `ends[v]` | how many stored items **end exactly** at v | "is this exact word stored?" (strings only; every number has the same length in a bit trie, so it isn't needed there) |

Insert-only problems with "does a path exist" queries can skip counts entirely.

### 3. Which direction to walk?
- Strings: first letter → last letter.
- Numbers: **top bit → bit 0**, every number padded to the same B bits. Comparisons of numbers and XORs are decided by the **highest** differing bit, so that's the order that lets you make decisions greedily.

### 4. Delete nodes or just counts?
**Just counts.** Erase walks the same path and subtracts 1. Nodes stay, but a node with `pass = 0` is empty.
**Rule: always decide with `pass > 0`, never with "the child exists".** Otherwise a query can walk into a path whose items were all erased.

---

## Template 1: string trie

```cpp
// TEMPLATE: StringTrie
struct StringTrie {
    vector<array<int, 26>> nxt;     // nxt[v][c] = child of v by letter c, 0 = none (root is node 0)
    vector<int> pass;               // how many stored words go through v
    vector<int> ends;               // how many stored words end exactly at v

    StringTrie() { newNode(); }

    int newNode() {
        nxt.push_back({});          // all 26 children = 0
        pass.push_back(0);
        ends.push_back(0);
        return nxt.size() - 1;
    }

    void insert(const string &s) {
        int v = 0;
        pass[v]++;
        for (char ch : s) {
            int c = ch - 'a';
            if (!nxt[v][c]) {
                int u = newNode();  // create first, then link (push_back can move the vector)
                nxt[v][c] = u;
            }
            v = nxt[v][c];
            pass[v]++;
        }
        ends[v]++;
    }

    int find(const string &s) {     // node for prefix s, or -1 if it doesn't exist
        int v = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (!nxt[v][c]) return -1;
            v = nxt[v][c];
        }
        return v;
    }

    int countWord(const string &s) {      // how many times s is stored
        int v = find(s);
        return v == -1 ? 0 : ends[v];
    }

    int countPrefix(const string &p) {    // how many stored words start with p
        int v = find(p);
        return v == -1 ? 0 : pass[v];
    }

    bool erase(const string &s) {         // remove one copy; false if s isn't stored
        if (countWord(s) == 0) return false;
        int v = 0;
        pass[v]--;
        for (char ch : s) {
            v = nxt[v][ch - 'a'];
            pass[v]--;
        }
        ends[v]--;
        return true;
    }

    // longest prefix of s that is a prefix of some stored word
    int longestCommonPrefix(const string &s) {
        int v = 0, len = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (!nxt[v][c] || pass[nxt[v][c]] == 0) break;
            v = nxt[v][c];
            len++;
        }
        return len;
    }
};
```

**Use:**
```cpp
StringTrie t;
t.insert("cat"); t.insert("car"); t.insert("cart"); t.insert("dog");
t.countWord("car");    // 1
t.countWord("ca");     // 0   ("ca" is only a prefix)
t.countPrefix("ca");   // 3   (cat, car, cart)
t.erase("car");
t.countWord("car");    // 0
t.countPrefix("car");  // 1   (cart is still there)
t.longestCommonPrefix("cars");  // 3  ("car" is a prefix of "cart")
```

**Change the alphabet:** uppercase → `26` and `ch - 'A'`; digits → `10` and `ch - '0'`; mixed → `map<char, int>` per node.

**What it's for:** many strings + "prefix" in the statement: count words with a prefix, longest common prefix with a set of words, dictionary lookups while scanning a text, words-as-paths DP.

---

## Template 2: binary (XOR) trie

Same structure: 2 children instead of 26, the steps are the B bits of a number from the top.

```cpp
// TEMPLATE: XorTrie
const int B = 30;                   // values < 2^30; use 60 + long long for values up to 1e18

struct XorTrie {
    vector<array<int, 2>> nxt;      // nxt[v][bit] = child, 0 = none (root is node 0)
    vector<int> cnt;                // how many stored numbers go through v

    XorTrie() { clear(); }
    void clear() { nxt.assign(1, {0, 0}); cnt.assign(1, 0); }

    void add(int x, int d) {        // d = +1 insert, d = -1 erase (x must be stored)
        int v = 0;
        cnt[v] += d;
        for (int i = B - 1; i >= 0; i--) {
            int b = (x >> i) & 1;
            if (!nxt[v][b]) {
                int u = nxt.size();
                nxt.push_back({0, 0});
                cnt.push_back(0);
                nxt[v][b] = u;
            }
            v = nxt[v][b];
            cnt[v] += d;
        }
    }

    bool has(int v) { return v != 0 && cnt[v] > 0; }   // child exists AND not emptied

    int size() { return cnt[0]; }

    // max of (x ^ a) over stored a. Set must be non-empty.
    int maxXor(int x) {
        int v = 0, res = 0;
        for (int i = B - 1; i >= 0; i--) {
            int b = (x >> i) & 1;
            if (has(nxt[v][b ^ 1])) { res |= 1 << i; v = nxt[v][b ^ 1]; }   // opposite bit -> XOR bit 1
            else v = nxt[v][b];
        }
        return res;
    }

    // min of (x ^ a) over stored a. Set must be non-empty.
    int minXor(int x) {
        int v = 0, res = 0;
        for (int i = B - 1; i >= 0; i--) {
            int b = (x >> i) & 1;
            if (has(nxt[v][b])) v = nxt[v][b];                               // same bit -> XOR bit 0
            else { res |= 1 << i; v = nxt[v][b ^ 1]; }
        }
        return res;
    }

    // how many stored a have (x ^ a) < lim
    int countLess(int x, int lim) {
        int v = 0, res = 0;
        for (int i = B - 1; i >= 0; i--) {
            int b = (x >> i) & 1, l = (lim >> i) & 1;
            if (l == 1) {
                if (nxt[v][b]) res += cnt[nxt[v][b]];   // XOR bit 0 < lim bit 1: whole group counts
                v = nxt[v][b ^ 1];                        // XOR bit 1 = lim bit: still tied
            } else {
                v = nxt[v][b];                            // XOR bit must be 0 to stay tied
            }
            if (!has(v)) return res;
        }
        return res;                                       // equal to lim: not counted
    }

    // k-th smallest (x ^ a) over stored a, k is 1-based, 1 <= k <= size()
    int kthXor(int x, int k) {
        int v = 0, res = 0;
        for (int i = B - 1; i >= 0; i--) {
            int b = (x >> i) & 1;
            int same = nxt[v][b];                         // this side gives XOR bit 0 -> all smaller
            int inSame = same ? cnt[same] : 0;
            if (inSame >= k) {
                v = same;                                 // answer is inside this group
            } else {
                k -= inSame;                              // skip the whole group
                res |= 1 << i;                            // XOR bit becomes 1
                v = nxt[v][b ^ 1];
            }
        }
        return res;
    }
};
```

**Use:**
```cpp
XorTrie t;
for (int x : {6, 7, 8, 9, 15}) t.add(x, 1);
t.maxXor(6);         // 15  (6 ^ 9)
t.minXor(6);         // 0   (6 ^ 6)
t.countLess(6, 9);   // 2   (XORs are 0, 1, 14, 15, 9 -> 0 and 1 are < 9)
t.kthXor(6, 3);      // 9   (sorted XORs: 0, 1, 9, 14, 15)
t.add(9, -1);        // erase 9
t.maxXor(6);         // 14  (6 ^ 8)
```

---

## The walks — the one idea behind every query

> A node is a **group of numbers sharing the same top bits**, and its count is the group's size.
> At each bit, the child with **x's bit** gives XOR bit 0 (smaller), the other child gives XOR bit 1 (bigger).
> So the whole "same" group comes **before** the whole "other" group in sorted order of x ^ a.

| Query | Decision at each bit |
|---|---|
| max XOR | go to the **opposite** bit if that group is non-empty (XOR bit 1) |
| min XOR | go to the **same** bit if non-empty (XOR bit 0) |
| count XOR < lim | lim bit 1 → **add** the whole same-bit group, continue in the other; lim bit 0 → continue in the same-bit group |
| k-th smallest XOR | same-bit group has ≥ k → go in; else **skip it** (k −= its size), XOR bit 1, go to the other |

New variants (count ≥ lim, max XOR with an extra condition, …) are new walks with the same idea — write them from this table.

### ⭐ The k-th trick: a trie gives you a *sorted list* for free
`kthXor(x, k)` = entry k of the sorted list `x ^ a[0], x ^ a[1], …` — without building or sorting it.
That means a trie turns "for each i, the sorted XORs of a[i] with everything" into **n sorted lists you can read in O(B) per entry**.

**Pattern: the m smallest pair XORs (a[i] ^ a[j], i < j)** — from CF 2267F2:
1. Put all numbers in the trie.
2. List i = sorted `a[i] ^ a[*]`. Entry 1 is a[i] ^ a[i] = 0 (paired with itself) → start each list at **position 2**.
3. Min-heap of (value, i). Start with entry 2 of every list. Pop the smallest, push the next entry of the same list.
4. Each pair (i, j) shows up in list i **and** list j, so pop **2m** values and keep every other one.

```cpp
// m smallest values of a[i] ^ a[j] (i < j), sorted.  Needs m <= n(n-1)/2.
vector<int> smallestPairXors(const vector<int> &a, int m) {
    int n = a.size();
    XorTrie t;
    for (int x : a) t.add(x, 1);
    vector<int> pos(n, 2);                                   // pos[i] = position of list i in the heap
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;   // (value, i)
    for (int i = 0; i < n; i++) pq.push({t.kthXor(a[i], 2), i});
    vector<int> taken;
    while ((int)taken.size() < 2 * m) {
        auto [v, i] = pq.top(); pq.pop();
        taken.push_back(v);
        if (pos[i] < n) { pos[i]++; pq.push({t.kthXor(a[i], pos[i]), i}); }
    }
    vector<int> res;
    for (int p = 0; p < 2 * m; p += 2) res.push_back(taken[p]);
    return res;
}
```
Cost: 2m · (log n + B).

---

## Common reductions
| Problem says | Turn it into |
|---|---|
| max XOR of a pair | insert one by one; before inserting a[i], `maxXor(a[i])` |
| max XOR of a **subarray** | prefix XOR: subarray (i, j] = p[j] ^ p[i]. Insert p[0] = 0 first, then for each j: query p[j], insert p[j] |
| count subarrays with XOR < k | same prefix trick with `countLess` |
| insert / delete / best XOR | `add(x, +1)`, `add(x, -1)`, `maxXor` |
| x ^ a **equal** to k | no trie needed: look up x ^ k in a set / map |
| m smallest pair XORs, k-th smallest pair XOR | `kthXor` + heap (above) |

---

## Checklist / mistakes I've made
- `(x >> i) & 1`, **not** `& i`. One wrong character, and small hand tests can still pass. Test against brute force.
- In erase-able tries, check `cnt > 0` (`has()`), not just "child exists".
- Insert **p[0] = 0** for subarray problems.
- Create the node first, then link: `int u = newNode(); nxt[v][c] = u;` (push_back can move the vector).
- B must cover the values: 30 for < 2³⁰ (~1.07·10⁹), 60 with `long long` and `1LL << i` for up to 10¹⁸.
- `kthXor` needs 1 ≤ k ≤ number stored.

## Memory
Nodes ≤ (total length inserted) + 1.
- String trie: each node = 26 + 2 ints ≈ 112 bytes. 10⁶ letters ≈ 112 MB — check the limit.
- XOR trie: each node = 3 ints = 12 bytes, B nodes per number. n = 2·10⁵, B = 30 → 6·10⁶ nodes ≈ 72 MB.
- Rough rule: 1 MB ≈ 10⁶ bytes ≈ 250,000 ints.

## Cost
Every operation walks one path: O(length) for strings, O(B) for numbers.

## Practice
| Problem | What it uses |
|---|---|
| [CSES – Maximum Xor Subarray](https://cses.fi/problemset/task/1655) | prefix XOR + `maxXor` |
| [706D – Vasiliy's Multiset](https://codeforces.com/problemset/problem/706/D) | insert / erase / `maxXor` |
| [817E – Choosing The Commander](https://codeforces.com/problemset/problem/817/E) | insert / erase / `countLess` |
| [2267F2 – XOR Transformations](https://codeforces.com/problemset/problem/2267/F2) | `kthXor` + heap (m smallest pair XORs) |
