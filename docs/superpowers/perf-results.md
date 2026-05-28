# mapcodelib performance results — feat/optimize

Branch: `feat/optimize` | Measurement: `time ./unittest` at `-O3` (best `user` of 3 runs, multithreaded so user ≫ real) | Baseline T0 = 114.13s

| Commit | Change | Best user time | vs. baseline |
|--------|--------|---------------|-------------|
| `f1cb736` | baseline | 114.13s | 0.0% |
| `430bbba` | A1: cache `flags` per iteration in hot loops | 112.21s | +1.7% |
| `b9b3f2c` | A2: longitude-first in `fitsInsideBoundaries` (**reverted** — regression) | 121.10s | −6.1% |
| `1b01b82` | A3: `memcpy`-based result assembly in `encoderEngine` | 113.19s | +0.8% |
| `753c337` | A4: single division per iteration in `encodeBase31` (no gain at `-O3`) | ~114s | noise |
| `554af29` | B3: companion tables defined + `initCompanionTables` called at entry points | 109.99s | +3.6% |
| `3416f87` | B4: hot loops read from companion tables | **99.40s** | **+12.9%** |
| `2cdf52b` | B5: `#ifdef DEBUG` sanity check (zero cost in release) | 99.73s | +12.6% |

**Final cumulative speedup: ~12.6%** (14.4s saved out of 114.13s)

**Binary size growth:** +2,240 bytes (limit was 100,000 bytes)

## Notes

- **A2 reverted:** `isInRange()` checks for longitude wrap-around — its overhead exceeded the saving from the early-out it enabled.
- **A4 no-op:** GCC `-O3` already performs strength reduction on `/31` + `%31`, so the explicit version gave no new information to the compiler.
- **A5 no-op:** The early-exit was already present in the codebase.
- **Primary driver:** B4 — replacing per-iteration mask/shift operations on a 4-byte `flags` field with single byte loads from precomputed byte-sized companion tables (`RECORD_KIND`, `RECORD_CODEX`, `RECORD_REC_TYPE`, `RECORD_HEADER_LETTER`).
- **Why short of 20–50% target:** The test suite includes correctness overhead (asserts, sprintf, comparisons) that dilutes the encode/decode speedup. Approach C (loop restructuring) would be the next lever.
