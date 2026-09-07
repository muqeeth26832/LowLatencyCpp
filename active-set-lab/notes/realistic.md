## Active-Set Realistic Tick Benchmark

The money benchmark: models a real feed-handler tick. Set is 50% populated (2048 actives); each iteration performs `churn` random toggles (churn % of the active half), then **sweeps the full active set** — mutation + iteration combined, mixed insert/erase rates from 10% to 90%.

### Results (ns per tick, N = 4096, 2048 active)

| Implementation | churn 10% | churn 50% | churn 90% |
|---|---|---|---|
| DenseStructSet | **6,091** | 23,653 | 45,893 |
| DensePtrSet | 7,268 | 24,477 | 40,133 |
| DenseIdSet | 7,749 | 22,768 | 40,620 |
| BitmaskSet | 7,771 | **22,457** | **37,561** |
| IntrusiveListSet | 22,426 | 46,302 | 61,009 |
| HashSet | 27,035 | 61,866 | 101,408 |
| ByteFlagSet | 41,701 | 59,312 | 73,911 |
| BitFlagSet | 51,135 | 68,404 | 82,714 |
| TreeSet | 85,849 | 243,773 | 450,323 |

### Interpretation

- **At low churn (10%), iteration dominates and DenseStructSet wins** (6.1 µs): swap-and-pop keeps the ~2000-element prefix contiguous, so the per-tick sweep is a fast sequential pass and only ~10% of the work is mutation.
- **At high churn (90%), mutation dominates and BitmaskSet wins** (37.6 µs): toggling a bit is a single masked word op, cheaper than the dense variants' swap-and-movebookkeeping × ~3700 times.
- **DensePtrSet / DenseIdSet sit between the two**, never worst among flat structures but never best — the extra indirection (deref/id-lookup) makes their sweeps slower than by-value, while their swap tax makes mutation slower than a bitmask.
- **DenseStructSet degrades the most with churn** — it is the best sweeper but the worst mutator (48-byte copies), and high churn exposes exactly that.
- **IntrusiveListSet** is ~3× behind the flat winners: O(1) mutates but the sweep is a dependent pointer chase over ~2000 nodes.
- **ByteFlagSet / BitFlagSet** collapse at low churn: the per-tick sweep *has* to inspect all 4096 flag bytes even when only 2048 are active.
- **TreeSet breaks everything**: 14–12× behind BitmaskSet, both in mutation (red-black toggles) and in sweep (log-N walk).

### Who wins when

| Workload flavour | Winner |
|---|---|
| Sparse iterate (10% active) | DenseStructSet (33× vs byte sweep) |
| Dense iterate (100%) | DenseStructSet (1.5× vs byte sweep) |
| Insert-only bulk | ByteFlagSet |
| Remove-only bulk | ByteFlagSet |
| Pure mutation churn | BitmaskSet / flags |
| Realistic tick, low churn | DenseStructSet |
| Realistic tick, high churn | BitmaskSet |
| Everything at once, balanced | DenseIdSet / DensePtrSet (never extreme) |

### Overall takeaways

1. **Keep the active prefix dense for iteration** — by-value swap-and-pop is the fastest sweeper; ids/pointers cost one extra indirection each.
2. **Do not pay movement tax on hot removes** — if churn is high, a bitmask (in-place bit toggles) beats every dense variant.
3. **Never use node-based containers for this pattern** — `std::set` is 12×-70× slower on every workload; `std::unordered_set` 2.5×-9× slower.
4. **The trade-off is the whole point**: dense prefix buys O(active) sequential iteration but pays per-op movement; flags buy O(1) mutation but O(N) iteration. Real systems (e.g. swap-and-pop + back-pointer) pick the dense prefix because iteration is the recurring hot path.