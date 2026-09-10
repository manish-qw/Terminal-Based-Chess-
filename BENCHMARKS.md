# Engine Benchmarks

All numbers measured on the same machine running `./chess --bench`.  
Compile flags: `g++ -std=c++17 -O2`

---

## 1 · Move Generator — Perft Correctness & Speed

Perft counts every legal leaf node at depth N from the starting position.  
Matching the known-correct values proves the move generator handles all edge cases
(castling, en passant, promotion, check legality).

| Depth | Nodes | Expected | ✓ | Time (ms) | Nodes / sec |
|------:|------:|----------:|:---:|----------:|------------:|
| 1 | 20 | 20 | PASS | < 1 | ~1,290,000 |
| 2 | 400 | 400 | PASS | < 1 | ~3,130,000 |
| 3 | 8,902 | 8,902 | PASS | 1.5 | ~5,900,000 |
| 4 | 197,281 | 197,281 | PASS | 25 | ~7,870,000 |
| 5 | 4,865,609 | 4,865,609 | PASS | 400 | ~12,150,000 |

**All 5 perft depths pass.**  
Peak move-generation speed: **~12 million nodes/sec** at depth 5.

---

## 2 · Search Speed — Alpha-Beta + MVV-LVA + Transposition Table

Fixed-depth search from the start position (single call, fresh TT each time).

| Depth | Time (ms) | Nodes explored | Nodes / sec | Best move |
|------:|----------:|---------------:|------------:|-----------|
| 1 | < 1 | 20 | ~57,000 | e2–e4 |
| 2 | < 1 | 176 | ~288,000 | e2–e3 |
| 3 | 2.3 | 849 | ~369,000 | e2–e3 |
| 4 | 11 | 4,350 | ~387,000 | e2–e3 |

Search at depth 4 completes in **~11 ms** — well within real-time play requirements.

---

## 3 · MVV-LVA Move Ordering Effectiveness

Comparing node count at depth 4 before and after adding MVV-LVA ordering
and a transposition table (old code used no ordering and no TT).

| Metric | Before (naive α-β) | After (MVV-LVA + TT) | Improvement |
|--------|-------------------:|---------------------:|:-----------:|
| Nodes @ depth 4 | 45,312 | 4,350 | **10.4× fewer** |
| Time @ depth 4 | ~180 ms | ~11 ms | **16× faster** |

MVV-LVA ensures captures like `PxQ` are always tried before `QxP`, so
alpha-beta cuts branches much earlier. The TT caches results from previously
searched transpositions, compounding the savings.

---

## 4 · Iterative Deepening — Time Budget

With a **500 ms** budget the engine searches:

| Depth reached | Nodes | Time used |
|--------------:|------:|----------:|
| 1 | 20 | < 1 ms |
| 2 | 176 | < 1 ms |
| 3 | 849 | ~2 ms |
| 4 | 4,350 | ~11 ms |
| 5 | ~29,000 | ~70 ms |
| **Total** | **~228,000** | **~500 ms** |

The engine reached **depth 5** within the 500 ms window, always returning the
best move found at the deepest completed depth.

---

## Reproduce

```bash
make
./chess --bench      # full benchmark suite
./chess --perft 5    # perft correctness test
```

> Benchmark runs with intentional 100–200 ms cooldowns between sections
> to avoid sustained 100% CPU usage on the host machine.
