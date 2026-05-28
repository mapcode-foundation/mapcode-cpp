# mapcodelib speed optimization — design spec

- **Date:** 2026-05-28
- **Branch:** `feat/optimize` (off `fix/bugfix`)
- **Author:** Rijn Buve (with Claude)

## 1. Scope & goals

Achieve a substantial speedup on the `mapcoder.c` encode and decode hot paths, measured by `time ./unittest` in `test/` (real wall-clock against the existing unit test suite, which already loops over millions of points).

**Target:** 20–50% wall-time reduction at `-O3`.

### Hard guardrails

- **Bit-exact output.** All existing unit tests must pass unchanged. No test files modified.
- **Strict portable C99/C11.** No `__builtin_*`, no SIMD intrinsics, no compiler-specific attributes.
- **No runtime / heap growth.** Per-call stack and heap usage must not increase. Public ABI (struct sizes in `mapcoder.h`) unchanged.
- **Small static-table growth OK.** Precomputed companion tables in the order of tens of KB are acceptable (negligible against existing data tables in `internal_data.h`).

### Out of scope

- Public API changes.
- Refactoring unrelated to performance.
- New features.
- Build-system changes (CMake, scripts) beyond what's strictly needed to compile.
- Approach C (loop restructuring, DFA decode parser) — deferred to a future branch if A+B underdeliver.

## 2. Measurement methodology

1. Build with `-O3` exactly as `test/run_normal.sh` does:
   ```
   cd mapcodelib && gcc -O3 -c mapcoder.c
   cd ../test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
   ```
2. Run `time ./unittest` three times back-to-back; record the best `user` time.
3. Repeat after each commit that touches `mapcoder.c`.

### Baseline

Capture the baseline on the **first commit** of `feat/optimize` (before any code change) so we have a clean reference number.

### Per-commit reporting

Each optimization commit's message includes:

- `time` output before vs. after (best of 3).
- Cumulative speedup vs. the baseline.
- One-line description of the change.

### Test integrity

Every commit must end with `unittest` printing `Unit tests passed` and zero errors. Any commit that regresses tests is rolled back before continuing.

### Noise caveat

The unit test does correctness work (asserts, sprintf, comparisons) in addition to encode/decode, so a 30% speedup of encode/decode core will appear as a smaller wall-time delta (perhaps 10–20%). That is expected; "significant" is judged on the total wall-clock number since that is the measurement method chosen.

## 3. Approach A — Local hot-path cleanups

Discrete, low-risk edits in `mapcoder.c`. Each is independently revertable and individually testable.

### A1. Cache `flags` per iteration in encode/decode loops

In `encoderEngine` (`mapcoder.c:1460`) and `decoderEngine` (`mapcoder.c:2651`), the loop body invokes `IS_NAMELESS(i)`, `IS_RESTRICTED(i)`, `IS_SPECIAL_SHAPE(i)`, `REC_TYPE(i)`, `coDex(i)`, `SMART_DIV(i)` on the same `i`. Each macro re-reads `TERRITORY_BOUNDARIES[i].flags`.

Replace with a single `const int flags = TERRITORY_BOUNDARIES[i].flags;` at the top of the loop body, plus local extracted variables where used more than once.

Same pattern in `firstNamelessRecord` (`mapcoder.c:822`) and `countNamelessRecords` (`mapcoder.c:835`).

### A2. Reorder checks in `fitsInsideBoundaries`

`mapcoder.c:602` checks lat-min, lat-max, lon-range. For mapcode territory boundaries (most are narrow longitudinally relative to the full earth lon range), test longitude band first to maximise early-out. Same logic, same correctness, fewer comparisons on average.

### A3. Length-tracked result assembly in `encoderEngine`

Around `mapcoder.c:1520` the per-result code uses `strcpy` + `strcat` patterns. `strcat` re-scans the destination from the start each time. Replace with `memcpy` using known lengths.

### A4. Speed up `encodeBase31`

`mapcoder.c:1083` emits a base-31 representation by repeated `%31` / `/31`. The compiler turns `/31` into a magic-multiply, but the loop is small. A tight unroll (or const lookup for the common small `nrchars` cases) can help.

### A5. Early-exit in `repackIfAllDigits` / `unpackIfAllDigits`

`mapcoder.c:892`, `:933`. If the input clearly fails the "all digits" precondition (e.g. first non-digit position), bail before any string mutation. The existing code already does some of this; tighten the check.

### Risk per change

Very low; each one is mechanically equivalent. Run full unit tests after each commit.

### Expected gain from A

~10–20% wall-time.

## 4. Approach B — Precomputed companion tables

Core insight: `encoderEngine`/`decoderEngine` loop over a range of records and re-derive metadata (`codex`, `recType`, `isNameless`, `isSpecialShape`, `smartDiv`) from `flags` on every iteration. Approach A caches `flags` per-iteration, but across many iterations we still recompute the same masks/shifts on the same record data. Precompute that metadata into a compact companion table once at library init, and have the hot loops read directly from it.

### B1. Companion arrays sized to `TERRITORY_BOUNDARIES`

Add to `mapcoder.c` at file scope (not exported):

```c
// One byte per record. Initialized once via initCompanionTables().
static unsigned char RECORD_CODEX[MAPCODE_BOUNDARY_MAX + 1];      // = coDex(m): 10*(c/5)+(c%5+1), c = flags & 31
static unsigned char RECORD_REC_TYPE[MAPCODE_BOUNDARY_MAX + 1];   // (flags >> 7) & 3
static unsigned char RECORD_KIND[MAPCODE_BOUNDARY_MAX + 1];       // bit0=nameless, bit1=restricted,
                                                                  // bit2=specialShape, bit3=hasHeaderLetter
static unsigned char RECORD_HEADER_LETTER[MAPCODE_BOUNDARY_MAX + 1]; // ENCODE_CHARS[(flags >> 11) & 31]
static unsigned short RECORD_SMART_DIV[MAPCODE_BOUNDARY_MAX + 1];   // flags >> 16
```

Per-record overhead: 6 bytes. With ~12,000 records, total ≈ 72 KB of static `.bss`/`.data`. Well within "small additions OK".

### B2. Per-territory derived metadata

```c
static int TERRITORY_FIRST_NAMELESS[_TERRITORY_MAX + 1];   // -1 if none
static int TERRITORY_NAMELESS_COUNT[_TERRITORY_MAX + 1];   // 0 if none
```

Eliminates linear scans in `firstNamelessRecord` (`mapcoder.c:822`) and `countNamelessRecords` (`mapcoder.c:835`), turning O(records-per-territory) into O(1). With ~241 territories, ~2 KB extra.

### B3. One-time initialization

A `static int companion_initialized = 0;` flag and an `initCompanionTables()` function called at the top of every public entry point that uses the data (`encodeLatLonToMapcodes_internal`, `decoderEngine`, etc.). The init computes everything from `TERRITORY_BOUNDARIES` and the flag-extraction macros — guaranteeing the precomputed values are *definitionally* the same as the macros.

**Thread safety.** The public API is not specified as thread-safe for first call; the existing code uses pthread only in tests. Init is idempotent (writes deterministic values), so even under a first-call race the worst case is duplicate work — never wrong values. This assumption is documented inline.

### B4. Replace macro call sites in hot loops only

`IS_NAMELESS(m)` and the other macros stay defined (used in cold paths, debug asserts, and the init function itself). In `encoderEngine`/`decoderEngine` inner loops, switch to direct table reads:

```c
const unsigned char kind = RECORD_KIND[i];
const int codex = RECORD_CODEX[i];
// ...
```

This decouples hot-loop performance from the bit-pack layout of `flags`.

### B5. Debug-build sanity check

In `#ifdef DEBUG` builds, `initCompanionTables` also asserts that the precomputed values match the macro-derived values for every record. Free correctness guarantee during development; zero cost in `-O3` release.

### Risk

Medium-low. Companion tables are derived directly from the same macros they replace, so by construction they hold identical values. The debug assertions catch any drift if someone later modifies the flag layout.

### Interaction with A1

A1 caches `flags` locally in the very same hot loops that B4 later switches to direct companion-table reads. After B4 lands, the local `flags` cache in those specific loop bodies becomes unused and must be removed as part of that commit. A1 remains useful in any cold-path call site that does not switch to companion tables.

### Expected combined gain from A+B

~20–35% wall-time.

## 5. Workflow

One commit per checkpoint, linear progression:

| # | Commit | Purpose |
|---|--------|---------|
| 1 | `chore: branch baseline` | Pin the baseline. Run `time ./unittest` 3× and record best `user` time in commit message. |
| 2 | `perf: A1 — cache flags per loop iteration` | Smallest, safest first. Re-run timing. |
| 3 | `perf: A2 — reorder fitsInsideBoundaries checks` | |
| 4 | `perf: A3 — length-tracked result assembly` | |
| 5 | `perf: A4 — tighten encodeBase31` | |
| 6 | `perf: A5 — early-exit in repack/unpack` | |
| 7 | `perf: B1+B2+B3 — companion tables init` | Tables defined and populated, but not yet read in hot path. Verifies init correctness in isolation. |
| 8 | `perf: B4 — hot loops read from companion tables` | The main payoff commit. |
| 9 | `perf: B5 — debug-build sanity check` | Optional polish. |

Each commit ends with:

- Full unit tests green (`Unit tests passed`, 0 errors).
- `time ./unittest` (best of 3) noted in the commit body, plus cumulative speedup vs. commit #1.

## 6. Risks & mitigations

| Risk | Mitigation |
|------|------------|
| A change breaks bit-exact behaviour | Run full test suite after every commit; revert if red. |
| Companion table values drift from `flags` semantics | B5 debug-build assertion verifies equivalence at init. |
| Thread-safety regression on first call | `initCompanionTables` is idempotent (deterministic writes). Documented. Safe under existing usage patterns. |
| Gains too small to justify | Stop after A and reassess; A alone should be ≥10% and is the lowest-risk subset. |
| Test wall-time too noisy to read signal | Use best-of-3 `user` time (not `real`), build with `-O3`, run on a quiet machine. |

## 7. Success criteria

- **Mandatory:** all unit tests pass after every commit.
- **Mandatory:** binary size growth ≤ 100 KB (companion tables are ~74 KB; allow margin).
- **Target:** ≥20% wall-time reduction on `time ./unittest` at `-O3` after commit #8.
- **Stretch:** ≥30% wall-time reduction.
- **Stop condition:** if Approach A alone delivers ≥30%, B is optional. If A+B underdeliver vs. 20%, re-evaluate before any further restructuring.
