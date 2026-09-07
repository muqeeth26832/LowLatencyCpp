## What I learned in active-set-lab

These are the takeaways worth remembering from building and benchmarking
9 active-instrument tracking implementations. The use case that drove this:
a small set of active instruments (~8) iterated (book-swept) every tick,
with only a handful of swaps (removes + adds) per day.

### 1. The id indirection costs you in the hot path

Storing active **ids** and resolving `universe[id]` on every iteration adds a
lookup to the most-repeated code path. The active set is iterated constantly;
the id resolution is dead weight in that loop.

**Change:** store the **instruments themselves** in the dense active set
instead of ids. Iteration becomes a pure sequential scan of exactly the
active objects.

### 2. Swap-and-pop keeps iteration O(active)

Swap-and-pop keeps the active elements packed at the front of a contiguous
vector, so iterating the set touches **only active instruments**, never the
inactive ones:

- With a flag/`bool active` inside the struct (or a separate flag array),
  every sweep walks **all N instruments** and costs the full struct
  cache-line read just to test one bit — O(N) traffic even when 8 of 4096
  are active (~100× more memory traffic in our sparse case).
- With swap-and-pop, the sweep is a straight line over the ~8 actives.

### 3. When swaps are rare, by-value storage is fine

Swap-and-pop's O(1) remove moves the last element into the hole. With ids
that's an 8-byte move; with by-value structs it's a `sizeof(Instrument)`
copy. We only do a few swaps per day, so the bigger memcpy is negligible —
and worth it, because by-value iteration has **zero indirection**.

### 4. The trade-off, stated cleanly

| | Dense by-value (swap-pop) | Flag / bool-active sweep |
|---|---|---|
| Iteration | O(active), sequential, no lookup | O(N) always, full scan |
| Insert/remove | O(1), but moves `sizeof(Instrument)` | O(1), pure bit/byte write |
| Best when | iterate-heavy, sparse actives, low churn | mutation-heavy / near-full |

### 5. Projects / interviews talking points

- Hot-path iteration should touch only what is active, and nothing else.
- Indirection (id lookup / pointer deref) is the enemy of a sweep; removing
  it can beat both flags and O(log N) containers.
- Big-O locks in the shape; memory traffic decides the constant.
- A "small object, big expected gain" micro-opt only matters because the
  container is iterated constantly and the inactive set is huge by
  comparison.

### 6. "A better hash set" does not fix an iteration hot path

Tested a flat open-addressing hash set (`flat_hash_set` style) as a 10th
implementation. On mutation it beats `std::unordered_set` (insert 4.5×,
remove 3.1×) — cache-friendly linear probing beats scattered buckets. But on
iteration — our hot path — it is O(capacity), not O(active), and ends up 7×
slower than by-value swap-and-pop at 10% active, 10× slower in the realistic
tick. Lesson: if iteration dominates, no hash set (flat or Swiss) fixes it;
only a container whose sweep is proportional to the live set does. Match the
data structure to the dominant operation, not the "better" generic one.