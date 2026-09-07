## Active-Set Mixed Churn Benchmark

Continuous mutation: the set starts 50% full (2048 active) and each iteration issues `churn` random toggles (insert if absent, erase if present) driven by a reference bitmap so every implementation sees the same op sequence. No iteration, pure mutator throughput.

### Results (ns per iteration, N = 4096, half-populated)

| Implementation | churn 10% (410 ops) | churn 50% (2048 ops) | churn 90% (3686 ops) |
|---|---|---|---|
| BitmaskSet | **7,309** | 37,716 | 68,427 |
| ByteFlagSet | 7,778 | 37,961 | 68,305 |
| DenseIdSet | 7,631 | 43,524 | 72,084 |
| DensePtrSet | 8,156 | 42,921 | 76,651 |
| DenseStructSet | 8,755 | 43,760 | 79,561 |
| IntrusiveListSet | 7,938 | 38,731 | 71,606 |
| BitFlagSet | 7,802 | 36,815 | 67,151 |
| HashSet | 20,155 | 93,385 | 170,535 |
| TreeSet | 83,222 | 419,468 | 783,279 |

### Interpretation

- **All flat structures cluster tightly** (~7–9 µs at 10% churn) because a churn op is one random address + one wire write. BitmaskSet / BitFlagSet / ByteFlagSet lead: the toggle is a single masked bit/byte flip with no element movement.
- **Dense structures pay their swap tax on every erase-side toggle**, which is why they drift ~5–15% behind the flags at high churn — an erase from mid-set pulls the last element into the hole and patches its back-pointer.
- **IntrusiveListSet stays mid-pack**: unlinking is O(1) with no movement, so churn-only it competes with the flat arrays.
- **TreeSet and HashSet are annihilated**: at 90% churn they are 11× and 2.5× slower than the best flat structure. Random-access mutation is exactly where node-based containers pay allocations and rebalances.

### Key Takeaway

When **mutation rate is high and iteration is absent**, flags and bitmasks win on simplicity — an erase is just clearing a bit, no reordering. The dense swap-and-pop variants are close but always pay the movement tax on removes. The takeaway for a real feed handler: if churn dominates, prefer in-place structures; let iteration be the thing that justifies a dense prefix.