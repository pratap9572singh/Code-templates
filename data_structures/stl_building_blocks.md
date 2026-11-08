# STL Building Blocks — Interview Revision

Every STL container sits on one of a few data structures. Learn those, and the containers follow.

| Building block | Containers | Key point |
|---|---|---|
| Dynamic array | `vector`, `string` | O(1) index, O(1) average `push_back`, cache-friendly |
| Block array | `deque` (and `stack`, `queue` by default) | O(1) at both ends |
| Linked list | `list`, `forward_list` | O(1) insert at a known spot, no indexing |
| Red-black tree | `set`, `map`, `multiset`, `multimap` | sorted, O(log n) everything |
| Hash table | `unordered_set`, `unordered_map` | O(1) average, O(n) worst, no order |
| Binary heap | `priority_queue` | O(1) top, O(log n) push/pop |

`stack`, `queue`, `priority_queue` are **wrappers**: they sit on another container and only expose one end.

---

## 1. Dynamic array — `vector`, `string`

One contiguous block + **size** (elements used) + **capacity** (space reserved).

- **Growing:** when full, allocate a block ~2× bigger, move everything, free the old one.
- **Why `push_back` is O(1) on average:** copies happen at sizes 1, 2, 4, 8, … — they add up to less than 2n over n pushes. Worst moment is right after a copy (e.g. after 9 pushes: 15 copies < 18). Total work < 3n.
- Growing by a fixed amount (+10) instead of ×2 would make it O(n) per push.

| Operation | Cost |
|---|---|
| `v[i]`, `front`, `back` | O(1) |
| `push_back` / `pop_back` | O(1) average |
| insert / erase in middle | O(n) — elements shift |
| search unsorted / `lower_bound` sorted | O(n) / O(log n) |

**Traps**
- Growing **invalidates pointers and iterators** (elements move). `reserve(n)` up front avoids it.
- `v{5}` = one element 5; `v(5)` = five zeros.
- `v.size() - 1` on an empty vector wraps to a huge number (unsigned).
- `string`: short strings (~15 chars) live inside the object, no heap allocation (SSO).

> "A vector is a dynamic array stored contiguously. Index access is O(1), push_back is amortised O(1) because it doubles capacity when full, and middle insert is O(n). It's the default because contiguous memory is cache-friendly. Reallocation invalidates pointers and iterators."

---

## 2. Block array — `deque`

Elements in many **fixed-size blocks** + a small array of pointers to those blocks.

- Add at back → last block (new block when full). Add at front → first block (new block in front).
- `d[i]`: block = i / block size, position = i % block size → O(1), but two jumps.
- Existing elements **never move** when it grows.
- Slower than `vector` in practice (two jumps, memory not in one piece).

> "A deque stores elements in fixed-size blocks with an array of pointers to them: O(1) push/pop at both ends, O(1) indexing. It's the default under stack and queue."

---

## 3. Linked list — `list`, `forward_list`

Each element is a separate heap **node** with pointers to next (and previous, for `list`).

| Operation | Cost |
|---|---|
| insert / erase at an iterator you already have | O(1) |
| push / pop at either end | O(1) |
| reach i-th element, search | O(n) — no `l[i]` |

- Elements never move → iterators stay valid. `splice` moves chunks in O(1).
- Rarely used: one allocation per node, extra pointer memory, cache miss at every step. `vector` often wins even for middle inserts.
- Classic use: **LRU cache** = `list` (usage order) + `unordered_map<key, list iterator>`, all O(1).
- `forward_list`: next pointer only, less memory, forward only.

> "A list is a doubly linked list: O(1) insert/erase at a known position and stable iterators, but no random access and cache-unfriendly, so vector is usually faster. Classic use is an LRU cache with a hash map."

---

## 4. Binary search tree → red-black tree — `set`, `map`, `multiset`, `multimap`

### Binary search tree (BST)
Each node holds a value, a left child and a right child.
**Rule:** everything in the left subtree is smaller, everything in the right subtree is bigger.

```
        8
      /   \
     3     10
    / \      \
   1   6      14
      / \
     4   7
```

**Search** (find 6): start at the top. 6 < 8 → go left to 3. 6 > 3 → go right to 6. Found.
If you fall off the tree (reach an empty child), the value isn't there. One step per level → cost = **height**.

**Insert** (insert 5): search for 5 as above: 8 → left to 3 → right to 6 → left to 4 → right of 4 is empty → attach 5 there. New values always become a new leaf.

**Delete** — three cases:
1. **Leaf** (e.g. 1): just remove it.
2. **One child** (e.g. 10, child 14): connect the parent straight to that child, then remove the node.
3. **Two children** (e.g. 3): replace its value with the **next bigger value** — the leftmost node in its right subtree (here 4). Then delete that node instead; it has at most one child, so it's case 1 or 2.

**Min / max:** keep going left → smallest (1). Keep going right → largest (14).

**Sorted order:** visit left subtree, then the node, then the right subtree → 1, 3, 4, 6, 7, 8, 10, 14. That's why iterating a `set` gives sorted output.

**Next element** (what `++it` does):
- If the node has a right child → go right once, then all the way left.
- Otherwise → go up until you arrive from a left child; that parent is the next one.

**`lower_bound(x)`** (first value ≥ x): walk down like a search. Whenever the current value is ≥ x, remember it and go left (maybe something smaller still works); otherwise go right. The last remembered node is the answer.
e.g. `lower_bound(5)`: 8 ≥ 5 (remember, left) → 3 < 5 (right) → 6 ≥ 5 (remember, left) → 4 < 5 (right) → empty. Answer: **6**.

### The problem: height
Every operation costs the height. Random inserts give height ~log n, but inserting **1, 2, 3, …, n in order** puts each value to the right of the last → a chain of height n → **O(n)** per operation. Sorted input is common, so a plain BST isn't safe.

### The fix: self-balancing
After each insert/erase, a balanced BST repairs its shape with **rotations** — a parent and child swap places while keeping smaller-left / bigger-right:

```
    x                y
     \      →       /
      y            x
```

A **red-black tree** colours nodes red/black and uses those colours to decide when to rotate. You don't need the rules — only the result: **height always stays O(log n)**, so every operation is O(log n), even on sorted input. (AVL trees and treaps are other balanced BSTs.)

### Costs in `set` / `map`

| Operation | Cost |
|---|---|
| insert, erase, find, `lower_bound`, `upper_bound` | O(log n) |
| min / max (`*s.begin()`, `*s.rbegin()`) | O(1) |
| iterate in sorted order | O(n) |

- `set` = values, `map` = key → value sorted by key; `multi` versions allow duplicates.
- Iterators stay valid until that element is erased.
- ~40 bytes per node for `int` → heavier and slower than a sorted vector or a hash table.

**Traps**
- `lower_bound(s.begin(), s.end(), x)` on a set/map is **O(n)**. Use `s.lower_bound(x)`.
- `mp[key]` **creates** the key if missing. Use `find` / `count` to just check.
- `ms.erase(x)` removes **all** copies. One copy: `ms.erase(ms.find(x))`.
- `ms.count(x)` is O(log n + count) — slow with many copies.

**Use cases**
- Keep data sorted while inserting/deleting: leaderboards, live price levels, a set of free slots.
- "Nearest value ≥ x / ≤ x" queries: `lower_bound`, `upper_bound`, then step `prev`.
- Need both min and max at once (a heap gives only one): `*begin()`, `*rbegin()`.
- Sliding window max/min with deletions: `multiset`, erase one copy as the window moves.
- Intervals and scheduling: find the interval that starts just before a point.
- Counting by key with ordered output: `map<key, count>`.
- k-th smallest / "how many < x": not in `set` — use PBDS `ordered_set` (`find_by_order`, `order_of_key`).

> "set and map are red-black trees — self-balancing BSTs — so insert, erase, find and lower_bound are O(log n) and elements stay sorted. That ordering is the advantage over unordered_map, at the cost of speed and memory."

---

## 5. Hash table — `unordered_set`, `unordered_map`

**How it works**
1. **Hash function** turns the key into a number (same key → same number; different keys spread out).
   - Integers in GCC: the hash is the number itself.
   - Strings: mix letters in order, e.g. `h = h * 31 + c`.
2. `number % bucket_count` → bucket index (GCC uses a prime bucket count).
3. Store the key in that bucket.
4. **Find:** hash → bucket → walk that bucket's list comparing the **actual keys** with `==`.

**Collisions:** different keys in the same bucket are kept in a small list (**chaining**). Values never mix up — keys are always compared.

**Rehashing:** C++ keeps about one bucket per key (load factor ≤ 1). When exceeded, it ~doubles the buckets and reinserts every key — O(n) occasionally, O(1) on average (same trick as vector).

| Operation | Average | Worst |
|---|---|---|
| insert, erase, find | O(1) | O(n) |
| sorted order / `lower_bound` | not available | |

**Worst case:** many keys in one bucket. Since GCC's integer hash is the number itself, Codeforces anti-hash tests force this → TLE. Fix:

```cpp
struct custom_hash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }
    size_t operator()(uint64_t x) const {
        static const uint64_t R = chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + R);
    }
};
unordered_map<long long, int, custom_hash> mp;
mp.reserve(n);   // skip rehashing if you know the size
```

**Why not give each key its own index directly?** You'd need a lookup to know which index a key got — that's the map itself. A hash **computes** the index from the key. Direct indexing *is* best when:
- keys are small integers → plain array `cnt[x]++`;
- all keys known in advance → **coordinate compression** (sort, number them 1, 2, 3, …).

**`map` vs `unordered_map`:** need order / `lower_bound` / min-max → `map`. Only "is it there / its value" → `unordered_map` (with custom hash on Codeforces).

> "A hash map hashes the key to pick a bucket, so lookup, insert and erase are O(1) on average. Collisions are handled by chaining, and it rehashes into about twice as many buckets when the load factor gets too high. Worst case is O(n) if keys collide, which can be forced, so in contests I use a randomised hash. If I need ordering, I use map at O(log n)."

---

## 6. Binary heap — `priority_queue`

**Rule:** every parent ≥ its children → the **max is always on top**. Only partly sorted (no left/right order) — cheaper than a BST.

**Stored in a plain array** (tree filled level by level, no gaps):
- children of `i`: `2i + 1`, `2i + 2`
- parent of `i`: `(i − 1) / 2`

```
        9
      /   \          index:  0  1  2  3  4  5
     7     8         value: [9, 7, 8, 3, 5, 6]
    / \   /
   3   5 6
```

**Push** (bubble **up**): put the new element at the end; while it's bigger than its parent, swap with the parent. O(log n).

**Pop** (bubble **down**): move the **last** element to the top; while its **larger child** is bigger, swap with that larger child (swapping with the smaller one would break the rule). Stop when both children are ≤ it or it has no children. O(log n).

Example — pop `[9, 7, 8, 3, 5, 6]`:
- last (6) to top → `[6, 7, 8, 3, 5]`
- children 7, 8 → larger is 8 > 6 → swap → `[8, 7, 6, 3, 5]`
- 6 at index 2 has no children → stop. New top = 8 ✓

| Operation | Cost |
|---|---|
| `top()` | O(1) |
| `push` / `pop` | O(log n) |
| build from n elements (`priority_queue pq(v.begin(), v.end())`) | O(n) |
| find / erase arbitrary element, change a value | not supported |

```cpp
priority_queue<int> pq;                                  // max on top
priority_queue<int, vector<int>, greater<int>> pq;       // min on top
```

- `pop()` doesn't return the value — read `top()` first.
- **Dijkstra:** you can't decrease a value inside the heap → push the new distance too, and skip outdated entries on pop: `if (d > dist[u]) continue;`
- Need both max **and** min quickly → use a `multiset` instead.

**Why building from an array is O(n), not O(n log n):** bubble down every parent, starting from the last parent and going up to the root. Half the nodes are leaves (no work), a quarter move at most one level, an eighth at most two… most nodes barely move, so the total is about n.

**Heap vs set:** a heap only keeps the top in order, so it's faster and lighter, but you can't search it, delete from the middle, or get both ends. Use a set when you need any of those.

**Use cases**
- **Shortest paths (Dijkstra), minimum spanning tree (Prim):** always process the closest node next — min-heap.
- **Top k largest of a stream:** keep a **min-heap of size k**; when a new value beats the top, pop and push. O(n log k).
- **Merge k sorted lists:** heap holds the current head of each list; pop the smallest, push the next from that list.
- **Running median:** a max-heap for the lower half + a min-heap for the upper half, kept within one element in size; median comes from the tops.
- **Greedy "always take the best available":** scheduling by deadline or profit, Huffman coding, merging ropes/files at least cost (pop two smallest, push their sum).
- **Event simulation:** process events in time order as new ones get added.
- **Heap sort:** build a heap, pop n times — O(n log n), no extra memory.

> "priority_queue is a binary heap stored in an array: each parent is at least its children, so the max is at index 0. Push adds at the end and bubbles up, pop moves the last element to the top and bubbles down, both O(log n); top is O(1). It's lighter than a set because it only keeps the top in order."

---

## Quick choice guide

| Need | Use |
|---|---|
| Index access, append, scan | `vector` |
| Add/remove at both ends | `deque` |
| Sorted order, `lower_bound`, min and max | `set` / `map` |
| Fast "is it there / what's its value" | `unordered_set` / `unordered_map` |
| Repeatedly take the largest (or smallest) | `priority_queue` |
| Keys are small integers | plain array |
