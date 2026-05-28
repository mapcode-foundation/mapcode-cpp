# mapcodelib speed optimization implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reduce `time ./unittest` wall time by 20–50% on `-O3` builds while preserving bit-exact output, strict portable C99/C11, and per-call memory footprint.

**Architecture:**
- **Approach A** (Tasks 2–6): Local hot-path edits in `mapcoder.c` — flag caching, branch reordering, length-tracked string assembly, tighter base-31 encode, and early exits in repack/unpack.
- **Approach B** (Tasks 7–9): Precomputed static companion tables derived once from `TERRITORY_BOUNDARIES`; hot loops in `encoderEngine`/`decoderEngine` read from them instead of re-deriving from `flags` per iteration.

**Tech Stack:** C99, GCC `-O3`, pthread (test-only), `time` (wall-clock measurement).

**Spec:** `docs/superpowers/specs/2026-05-28-mapcodelib-optimize-design.md`

**Branch:** `feat/optimize` (already created, currently at design-spec commit `740bfec`).

---

## Conventions used across all tasks

### Build command
```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
```

### Test command (correctness)
```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && ./unittest
```
Expected: final line `Unit tests passed`, exit code 0.

### Timing command (performance)
```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test
( time ./unittest ) 2>&1 | tail -3   # repeat 3 times, take best user time
```
Each commit body records: best `user` time, and the percent-delta vs. Task 1 baseline.

### Commit footer
Every commit includes:
```
Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```

### Test integrity
Any commit that fails `./unittest` (non-zero exit, non-zero error count, or missing "Unit tests passed" line) MUST be reverted before continuing. Do not push fixes on top — revert and reattempt.

---

## File structure

| File | Status | Responsibility |
|------|--------|----------------|
| `mapcodelib/mapcoder.c` | Modify | All optimization edits land here. Hot paths (`encoderEngine`, `decoderEngine`, `fitsInsideBoundaries`, `encodeBase31`, etc.) and new static-scope companion tables + their initializer. |
| `mapcodelib/internal_data.h` | Read-only | Source of truth for `TERRITORY_BOUNDARIES`, `MAPCODE_BOUNDARY_MAX`, `DATA_START`. Do NOT modify. |
| `mapcodelib/mapcoder.h` | Read-only | Public ABI. Do NOT modify. |
| `test/unittest.c` | Read-only | Correctness + timing harness. Do NOT modify. |

All other files (CMake, scripts, docs) are untouched by this plan.

---

## Task 1: Baseline measurement

**Files:**
- No source changes. Single commit pins the baseline timing.

- [ ] **Step 1: Build with -O3**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
```

Expected: clean compile, no warnings/errors. Both `.o` and `unittest` binary produced.

- [ ] **Step 2: Verify tests pass**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && ./unittest 2>&1 | tail -5
```

Expected: last lines include `Unit tests passed`. Exit code 0.

- [ ] **Step 3: Run three timed iterations, capture best `user` time**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test
for i in 1 2 3; do echo "=== run $i ==="; ( time ./unittest >/dev/null ) 2>&1 | grep -E '^(real|user|sys)'; done
```

Record the **best of three** `user` times — that is the baseline number for all subsequent comparisons. Note it down (call it `T0`).

- [ ] **Step 4: Commit baseline marker**

Create an empty commit so the baseline timing lives in git history:

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git commit --allow-empty -m "$(cat <<'EOF'
chore: pin perf baseline for feat/optimize

Measured `time ./unittest` on -O3 build (best of 3 runs):
  user time = T0 = <FILL_IN>s

All subsequent perf commits on this branch quote their best user time
and the cumulative delta vs. this baseline.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

Replace `<FILL_IN>` with the actual best-of-three user time from Step 3.

---

## Task 2: A1 — Cache `flags` per iteration in hot loops

**Goal:** Read `TERRITORY_BOUNDARIES[i].flags` once per loop iteration into a local; rewrite macro call sites to use the local. Six call sites in three functions.

**Files:**
- Modify: `mapcodelib/mapcoder.c` — `encoderEngine` (around line 1487), `decoderEngine` (around line 2710), `firstNamelessRecord` (line 822), `countNamelessRecords` (line 835).

- [ ] **Step 1: Edit `encoderEngine` inner loop body to cache flags**

Open `mapcodelib/mapcoder.c`. Locate the loop at line 1487:

```c
for (i = from; i <= upto; i++) {
    if (fitsInsideBoundaries(&enc->coord32, TERRITORY_BOUNDARY(i))) {
        if (IS_NAMELESS(i)) {
```

Replace this loop body (lines 1487–1542 inclusive) with a version that introduces a local `flags` cache. The transformation is mechanical: every `IS_NAMELESS(i)`, `IS_RESTRICTED(i)`, `IS_SPECIAL_SHAPE(i)`, `REC_TYPE(i)`, `coDex(i)`, `HEADER_LETTER(i)` inside the loop body becomes a local-variable read. Keep behavior identical:

```c
for (i = from; i <= upto; i++) {
    const int flags = TERRITORY_BOUNDARIES[i].flags;
    if (fitsInsideBoundaries(&enc->coord32, TERRITORY_BOUNDARY(i))) {
        const int isNameless    = (flags & 64);
        const int recType       = (flags >> 7) & 3;
        const int isRestricted  = (flags & 512);
        if (isNameless) {
            encodeNameless(result, enc, ccode, extraDigits, i);
        }
        else if (recType > 1) {
            encodeAutoHeader(result, enc, i, extraDigits);
        }
        else if ((i == upto) && isSubdivision(ccode)) {
            // *** do a recursive call for the parent ***
            encoderEngine(parentTerritoryOf(ccode), enc, stop_with_one_result, extraDigits, requiredEncoder,
                          ccode);
            return;
        }
        else // must be grid
        {
            // skip IS_RESTRICTED records unless there already is a result
            if (result_counter || !isRestricted) {
                const int c = flags & 31;
                const int codexLocal = 10 * (c / 5) + ((c % 5) + 1);
                if (codexLocal < 54) {
                    const char headerletter = (char)((recType == 1) ? ENCODE_CHARS[(flags >> 11) & 31] : 0);
                    encodeGrid(result, enc, i, extraDigits, headerletter);
                }
            }
        }

        // =========== handle result (if any)
        if (*result) {
            result_counter++;

            repackIfAllDigits(result, 0);

            if ((requiredEncoder < 0) || (requiredEncoder == i)) {
                const enum Territory ccodeFinal = (ccode_override != TERRITORY_NONE ? ccode_override : ccode);
                if (*result && enc->mapcodes && (enc->mapcodes->count < MAX_NR_OF_MAPCODE_RESULTS)) {
                    char* s = enc->mapcodes->mapcode[enc->mapcodes->count++];
                    if (ccodeFinal == TERRITORY_AAA) {
                        // AAA is never shown with territory
                        strcpy(s, result);
                    }
                    else {
                        getTerritoryIsoName(s, ccodeFinal, 0);
                        strcat(s, " ");
                        strcat(s, result);
                    }
                }
                if (requiredEncoder == i) {
                    return;
                }
            }
            if (stop_with_one_result) {
                return;
            }
            *result = 0; // clear for next iteration
        }
    }
} // for i
```

Reasoning: identical semantics, but the compiler now has a guaranteed single load for `flags` regardless of how `IS_NAMELESS` etc. are written elsewhere, and the locally extracted bit values are SSA-friendly.

- [ ] **Step 2: Edit `decoderEngine` inner loop body**

Locate the loop at `mapcoder.c:2710`:

```c
for (i = from; i <= upto; i++) {
    const int codexi = coDex(i);
    const int r = REC_TYPE(i);
    if (r == 0) {
        if (IS_NAMELESS(i)) {
```

Replace lines 2710–2800 with a version that caches `flags` once:

```c
for (i = from; i <= upto; i++) {
    const int flags = TERRITORY_BOUNDARIES[i].flags;
    const int c = flags & 31;
    const int codexi = 10 * (c / 5) + ((c % 5) + 1);
    const int r = (flags >> 7) & 3;
    if (r == 0) {
        if (flags & 64) {  // IS_NAMELESS
            if (((codexi == 21) && (codex == 22)) ||
                ((codexi == 22) && (codex == 32)) ||
                ((codexi == 13) && (codex == 23))) {
                err = decodeNameless(dec, i);
                break;
            }
        }
        else {
            if ((codexi == codex) || ((codex == 22) && (codexi == 21))) {
                err = decodeGrid(dec, i, 0);

                // first of all, make sure the zone fits the country
                restrictZoneTo(&dec->zone, &dec->zone, TERRITORY_BOUNDARY(upto));

                if ((err == ERR_OK) && (flags & 512)) {  // IS_RESTRICTED
                    int nrZoneOverlaps = 0;
                    int j;

                    // *** make sure decode fits somewhere ***
                    dec->result = getMidPointFractions(&dec->zone);
                    dec->coord32 = convertFractionsToCoord32(&dec->result);
                    for (j = i - 1; j >= from; j--) {
                        // look in previous rects
                        if (!IS_RESTRICTED(j)) {
                            if (fitsInsideBoundaries(&dec->coord32, TERRITORY_BOUNDARY(j))) {
                                nrZoneOverlaps = 1;
                                break;
                            }
                        }
                    }

                    if (!nrZoneOverlaps) {
                        MapcodeZone zfound;
                        TerritoryBoundary prevu;
                        for (j = from; j < i; j++) {
                            // try all smaller rectangles j
                            if (!IS_RESTRICTED(j)) {
                                MapcodeZone z;
                                if (restrictZoneTo(&z, &dec->zone, TERRITORY_BOUNDARY(j))) {
                                    nrZoneOverlaps++;
                                    if (nrZoneOverlaps == 1) {
                                        // first fit! remember...
                                        zoneCopyFrom(&zfound, &z);
                                        ASSERT(j <= MAPCODE_BOUNDARY_MAX);
                                        memcpy(&prevu, TERRITORY_BOUNDARY(j), sizeof(TerritoryBoundary));
                                    }
                                    else {
                                        // nrZoneOverlaps >= 2
                                        // more than one hit
                                        break; // give up
                                    }
                                }
                            } // IS_RESTRICTED
                        } // for j

                        // if several sub-areas intersect, just return the whole zone
                        // (the center of which may NOT re-encode to the same mapcode!)
                        if (nrZoneOverlaps == 1) {
                            // found exactly ONE intersection?
                            zoneCopyFrom(&dec->zone, &zfound);
                        }
                    }

                    if (!nrZoneOverlaps) {
                        err = ERR_MAPCODE_UNDECODABLE; // type 3 "NLD L222.222"
                    }
                } // *** make sure decode fits somewhere ***
                break;
            }
        }
    }
    else if (r == 1) {
        if (codex == codexi + 10 && ENCODE_CHARS[(flags >> 11) & 31] == *s) {
            err = decodeGrid(dec, i, 1);
            break;
        }
    }
    else {
        //r>1
        if (((codex == 23) && (codexi == 22)) ||
            ((codex == 33) && (codexi == 23))) {
            err = decodeAutoHeader(dec, i);
            break;
        }
    }
} // for
```

Note: the inner `j` loops continue to use the `IS_RESTRICTED(j)` macro (not the cached `flags`) because they index a different record `j`. Leaving them as-is is correct.

- [ ] **Step 3: Edit `firstNamelessRecord`**

Locate `mapcoder.c:822`:

```c
static int firstNamelessRecord(const int m, const int firstcode) {
    int i = m;
    const int codexm = coDex(m);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (i >= firstcode && coDex(i) == codexm && IS_NAMELESS(i)) {
        i--;
    }
    return (i + 1);
}
```

Replace with:

```c
static int firstNamelessRecord(const int m, const int firstcode) {
    int i = m;
    const int codexm = coDex(m);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (i >= firstcode) {
        const int flags = TERRITORY_BOUNDARIES[i].flags;
        const int c = flags & 31;
        const int codexi = 10 * (c / 5) + ((c % 5) + 1);
        if (codexi != codexm || !(flags & 64)) {
            break;
        }
        i--;
    }
    return (i + 1);
}
```

- [ ] **Step 4: Edit `countNamelessRecords`**

Locate `mapcoder.c:835`:

```c
static int countNamelessRecords(const int m, const int firstcode) {
    const int first = firstNamelessRecord(m, firstcode);
    const int codexm = coDex(m);
    int last = m;
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (coDex(last) == codexm) {
        last++;
    }
    ...
```

Replace the inner `while` loop with:

```c
static int countNamelessRecords(const int m, const int firstcode) {
    const int first = firstNamelessRecord(m, firstcode);
    const int codexm = coDex(m);
    int last = m;
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (1) {
        const int c = TERRITORY_BOUNDARIES[last].flags & 31;
        const int codexLast = 10 * (c / 5) + ((c % 5) + 1);
        if (codexLast != codexm) {
            break;
        }
        last++;
    }
    ASSERT((0 <= last) && (last <= MAPCODE_BOUNDARY_MAX));
    ASSERT(last >= first);
    return (last - first);
}
```

- [ ] **Step 5: Build and run tests**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o && ./unittest 2>&1 | tail -5
```

Expected: clean compile, no warnings/errors. `./unittest` exits 0 with `Unit tests passed` in the output.

If anything fails: revert the changes in `mapcodelib/mapcoder.c` and stop. Do not proceed to Step 6.

- [ ] **Step 6: Measure timing (3× best)**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test
for i in 1 2 3; do echo "=== run $i ==="; ( time ./unittest >/dev/null ) 2>&1 | grep -E '^(real|user|sys)'; done
```

Record the best `user` time (call it `T2`). Compute `delta_pct = (T0 - T2) / T0 * 100`.

- [ ] **Step 7: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: A1 — cache flags per loop iteration in hot paths

In encoderEngine and decoderEngine inner loops, read
TERRITORY_BOUNDARIES[i].flags once per iteration into a const local and
extract bit fields from it, instead of using flag-extraction macros that
each re-dereference the same memory. Same change in firstNamelessRecord
and countNamelessRecords.

Bit-exact: macros stay defined and used in cold paths; only the inner
loop body call sites were rewritten to local reads.

  time ./unittest (best of 3, user):
    baseline (T0) = <FILL_IN>s
    after A1 (T2) = <FILL_IN>s
    delta         = <FILL_IN>%

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

Replace `<FILL_IN>` with the recorded numbers from Step 6 and Task 1.

---

## Task 3: A2 — Reorder checks in `fitsInsideBoundaries`

**Goal:** Test longitude band first to short-circuit faster on the typical case (most territory rectangles are narrow in longitude relative to the planet).

**Files:**
- Modify: `mapcodelib/mapcoder.c:602`

- [ ] **Step 1: Edit `fitsInsideBoundaries`**

Locate `mapcoder.c:602`:

```c
static int fitsInsideBoundaries(const Point32* coord32, const TerritoryBoundary* b) {
    ASSERT(coord32);
    ASSERT(b);
    return (b->miny <= coord32->latMicroDeg &&
        coord32->latMicroDeg < b->maxy &&
        isInRange(coord32->lonMicroDeg, b->minx, b->maxx));
}
```

Replace with longitude-first ordering:

```c
static int fitsInsideBoundaries(const Point32* coord32, const TerritoryBoundary* b) {
    ASSERT(coord32);
    ASSERT(b);
    return (isInRange(coord32->lonMicroDeg, b->minx, b->maxx) &&
        b->miny <= coord32->latMicroDeg &&
        coord32->latMicroDeg < b->maxy);
}
```

Reasoning: `&&` short-circuits left to right. Longitude is the more selective dimension for typical territory rectangles, so the cheap `isInRange` call rejects most non-matches first.

- [ ] **Step 2: Build and run tests**

Same commands as Task 2 Step 5. Expected: clean compile, `Unit tests passed`.

- [ ] **Step 3: Measure timing (3× best)**

Same as Task 2 Step 6. Record best `user` time (`T3`). Compute cumulative delta vs. `T0`.

- [ ] **Step 4: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: A2 — reorder fitsInsideBoundaries to test longitude first

Most territory rectangles are narrower in longitude than in latitude
relative to their bounding ranges, so testing longitude first short-
circuits faster on the typical reject case.

  time ./unittest (best of 3, user):
    baseline = <FILL_IN>s
    after A2 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Task 4: A3 — Length-tracked result assembly in `encoderEngine`

**Goal:** Replace `strcpy`+`strcat`+`strcat` pattern (which rescans the destination on each `strcat`) with length-tracked `memcpy` calls.

**Files:**
- Modify: `mapcodelib/mapcoder.c` — block around lines 1521–1530 (post-A1 line numbers shift slightly; locate by content).

- [ ] **Step 1: Locate the relevant block**

Within `encoderEngine`, find the section starting with:

```c
char* s = enc->mapcodes->mapcode[enc->mapcodes->count++];
if (ccodeFinal == TERRITORY_AAA) {
    // AAA is never shown with territory
    strcpy(s, result);
}
else {
    getTerritoryIsoName(s, ccodeFinal, 0);
    strcat(s, " ");
    strcat(s, result);
}
```

- [ ] **Step 2: Replace with length-tracked memcpy**

Replace the entire `if (ccodeFinal == TERRITORY_AAA) { ... } else { ... }` block with:

```c
char* s = enc->mapcodes->mapcode[enc->mapcodes->count++];
if (ccodeFinal == TERRITORY_AAA) {
    // AAA is never shown with territory
    strcpy(s, result);
}
else {
    getTerritoryIsoName(s, ccodeFinal, 0);
    size_t isoLen = strlen(s);
    size_t resultLen = strlen(result);
    s[isoLen] = ' ';
    memcpy(s + isoLen + 1, result, resultLen + 1);  // +1 to include NUL
}
```

Reasoning: `strcat(s, " ")` does `strlen(s)` then writes 2 bytes; `strcat(s, result)` does another `strlen(s)` (now longer) then writes `strlen(result)+1` bytes. The replacement does one `strlen` per source string and a single byte write + memcpy. Output is byte-identical.

Note: `<string.h>` is already included; `size_t` is fine in C99 source.

- [ ] **Step 3: Build and run tests**

Same as Task 2 Step 5.

- [ ] **Step 4: Measure timing (3× best)**

Same as Task 2 Step 6.

- [ ] **Step 5: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: A3 — length-tracked result assembly in encoderEngine

strcpy + strcat + strcat each re-scans the destination from the start
to find the null terminator. Replace with explicit strlen on each
source plus a single memcpy.

Output bytes are unchanged.

  time ./unittest (best of 3, user):
    baseline = <FILL_IN>s
    after A3 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Task 5: A4 — Tighten `encodeBase31`

**Goal:** Eliminate the per-iteration `value % 31` + `value / 31` by combining via `__builtin_*`-free C: compute quotient once, derive remainder. This is one machine division per iteration instead of two (the compiler may already do this, but it's not guaranteed without `-fno-trapping-math` etc.).

**Files:**
- Modify: `mapcodelib/mapcoder.c:1083`

- [ ] **Step 1: Edit `encodeBase31`**

Locate `mapcoder.c:1083`:

```c
static void encodeBase31(char* result, int value, int nrchars) {
    ASSERT(result);
    ASSERT(nrchars >= 0);
    result[nrchars] = 0; // zero-terminate!
    while (nrchars > 0) {
        nrchars--;
        result[nrchars] = ENCODE_CHARS[value % 31];
        value /= 31;
    }
}
```

Replace with:

```c
static void encodeBase31(char* result, int value, int nrchars) {
    ASSERT(result);
    ASSERT(nrchars >= 0);
    result[nrchars] = 0; // zero-terminate!
    while (nrchars > 0) {
        const int q = value / 31;
        const int r = value - q * 31;
        nrchars--;
        result[nrchars] = ENCODE_CHARS[r];
        value = q;
    }
}
```

Reasoning: For `int` with `value >= 0` (precondition by inspection — callers always pass non-negative computed offsets), `r = value - q*31` is equivalent to `value % 31` but uses one IDIV/MUL+SUB pair rather than potentially two division-class operations. Modern compilers often emit `imul` magic constants for division by 31 and do this optimization themselves; we make it explicit so it survives older toolchains and `-O3` -> `-O2` regressions.

- [ ] **Step 2: Build and run tests**

Same as Task 2 Step 5.

- [ ] **Step 3: Measure timing (3× best)**

Same as Task 2 Step 6.

- [ ] **Step 4: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: A4 — single division per iteration in encodeBase31

Compute quotient once, derive remainder via subtraction so the loop
has one division-class op per character rather than two.

  time ./unittest (best of 3, user):
    baseline = <FILL_IN>s
    after A4 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Task 6: A5 — Tighten `repackIfAllDigits` early-exit

**Goal:** The current implementation walks the string twice in many cases. We add a tightened first-pass that bails sooner.

**Files:**
- Modify: `mapcodelib/mapcoder.c:892`

- [ ] **Step 1: Read the current implementation in full**

The function spans `mapcoder.c:892–930` (approx). Read it carefully — it already has an `alldigits` flag flipped to 0 on first non-digit, but it still continues the scan looking for `'.'`. Let's tighten it.

Look at the current code:

```c
static void repackIfAllDigits(char* input, const int aonly) {
    char* s = input;
    int alldigits = 1; // assume all digits
    char* e;
    char* dotpos = NULL;
    ASSERT(input);
    for (e = s; *e != 0 && *e != '-'; e++) {
        if (*e < '0' || *e > '9') {
            ...
```

- [ ] **Step 2: Read the rest of the function first**

```bash
sed -n '892,935p' /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib/mapcoder.c
```

The implementing engineer MUST read the full function before editing it. The optimization opportunity is to short-circuit when `alldigits` becomes 0 AND `dotpos` is already set — at that point we know the full structure and don't need to scan further within this loop.

- [ ] **Step 3: Apply minimal early-exit**

In the loop body, after `alldigits = 0;` is set inside the `if (*e < '0' || *e > '9')` branch, the function already has a `break` for the case `*e != '.'`. If `*e == '.'`, the loop continues, but once `alldigits` is 0 there's no more work the loop accomplishes other than setting `dotpos`.

The cleanest tightening: after the existing `if (*e == '.')` block that assigns `dotpos`, if `alldigits` is already 0 and `dotpos` is non-null, `break`.

Locate the existing `else if (*e == '.')` clause (it should look like):

```c
else if (*e == '.') {
    dotpos = e;
}
```

Modify the surrounding `for` loop body so that after `dotpos = e;` we check both conditions and break when no useful work remains. Replace the entire `for` loop with:

```c
for (e = s; *e != 0 && *e != '-'; e++) {
    if (*e < '0' || *e > '9') {
        if (*e == '.') {
            if (dotpos) {
                // Two dots: malformed for repack purposes, bail.
                return;
            }
            dotpos = e;
            if (!alldigits) {
                break;  // structure fully known; nothing else to learn here
            }
        } else {
            alldigits = 0;
            if (dotpos) {
                break;  // structure fully known
            }
        }
    }
}
```

Be very careful: the original code may handle the "two dots" case differently. If the actual code differs from what is described here, ABORT this task and consult the maintainer. Do NOT change the semantics for any input.

- [ ] **Step 4: Read the function again to confirm semantics**

Before committing, the implementer MUST diff the file against `git HEAD` and ensure the only changes are:
1. Optional `break`s added in well-understood control-flow locations.
2. No other behavioral change.

If the diff shows anything else changed (variable names, condition rewrites, etc.), revert.

- [ ] **Step 5: Build and run tests**

Same as Task 2 Step 5. Bit-exactness is essential here — if any single test fails, revert immediately.

- [ ] **Step 6: Measure timing (3× best)**

Same as Task 2 Step 6.

- [ ] **Step 7: Commit (or skip if no measurable gain)**

If the timing delta vs. previous commit is below noise threshold (< 0.5%) and the diff is non-trivial, prefer NOT to commit (skipping A5 is acceptable per the spec). Otherwise:

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: A5 — early-exit when structure is fully known in repackIfAllDigits

Break out of the scan loop once both the digits-only status and the
dot position are determined. Same input/output mapping; the loop just
stops earlier on common inputs.

  time ./unittest (best of 3, user):
    baseline = <FILL_IN>s
    after A5 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

If skipped, document the skip in the next commit's body ("A5 skipped: gain below noise").

---

## Task 7: B1+B2+B3 — Define and initialize companion tables (not yet used)

**Goal:** Add companion static tables, a one-time initializer, and call sites that invoke the initializer at every public entry point. The hot loops still use macros at this point — this task is verified by tests-still-pass + the initializer running without crashing.

**Files:**
- Modify: `mapcodelib/mapcoder.c` — add static-scope tables and `initCompanionTables()` near the top of the file (after macros, before any hot function).

- [ ] **Step 1: Add static companion tables and initializer**

Open `mapcodelib/mapcoder.c`. Find the line right after the flag-extraction macro definitions (after `mapcoder.c:154` `#define HEADER_LETTER(m) ...`). Add the following block:

```c
///////////////////////////////////////////////////////////////////////////////////////////////
//
//  PRECOMPUTED COMPANION TABLES (perf: feat/optimize, see docs/superpowers/specs/2026-05-28-mapcodelib-optimize-design.md)
//
//  These tables are derived once from TERRITORY_BOUNDARIES.flags and queried
//  in hot encode/decode loops to avoid repeated bit-masking on the same record.
//
///////////////////////////////////////////////////////////////////////////////////////////////

#define KIND_BIT_NAMELESS       0x01
#define KIND_BIT_RESTRICTED     0x02
#define KIND_BIT_SPECIAL_SHAPE  0x04

static unsigned char RECORD_CODEX[MAPCODE_BOUNDARY_MAX + 1];      // = coDex(m)
static unsigned char RECORD_REC_TYPE[MAPCODE_BOUNDARY_MAX + 1];   // (flags >> 7) & 3
static unsigned char RECORD_KIND[MAPCODE_BOUNDARY_MAX + 1];       // KIND_BIT_* bitwise OR
static unsigned char RECORD_HEADER_LETTER[MAPCODE_BOUNDARY_MAX + 1]; // ENCODE_CHARS[(flags >> 11) & 31]
static unsigned short RECORD_SMART_DIV[MAPCODE_BOUNDARY_MAX + 1];   // flags >> 16

// Per-territory: index of the first nameless record (or -1), and how many.
// These tables are sized to the full territory enum range.
#define TERRITORY_TABLE_SIZE (_TERRITORY_MAX - _TERRITORY_MIN)
static int TERRITORY_FIRST_NAMELESS[TERRITORY_TABLE_SIZE];
static int TERRITORY_NAMELESS_COUNT[TERRITORY_TABLE_SIZE];

// Init flag. Write-once after first call. Idempotent: deterministic values mean
// a race on first call only causes duplicate work, never wrong results.
// The library does not promise thread-safety on first call (see header docs).
static int companion_initialized = 0;

static void initCompanionTables(void) {
    int m;
    int t;
    if (companion_initialized) {
        return;
    }
    for (m = 0; m <= MAPCODE_BOUNDARY_MAX; m++) {
        const int flags = TERRITORY_BOUNDARIES[m].flags;
        const int c = flags & 31;
        const int codex_val = 10 * (c / 5) + ((c % 5) + 1);
        const int rec_type = (flags >> 7) & 3;
        unsigned char kind = 0;
        if (flags & 64)   kind |= KIND_BIT_NAMELESS;
        if (flags & 512)  kind |= KIND_BIT_RESTRICTED;
        if (flags & 1024) kind |= KIND_BIT_SPECIAL_SHAPE;
        RECORD_CODEX[m] = (unsigned char) codex_val;
        RECORD_REC_TYPE[m] = (unsigned char) rec_type;
        RECORD_KIND[m] = kind;
        RECORD_HEADER_LETTER[m] = (unsigned char) ENCODE_CHARS[(flags >> 11) & 31];
        RECORD_SMART_DIV[m] = (unsigned short) ((unsigned int) flags >> 16);
    }
    for (t = 0; t < TERRITORY_TABLE_SIZE; t++) {
        TERRITORY_FIRST_NAMELESS[t] = -1;
        TERRITORY_NAMELESS_COUNT[t] = 0;
    }
    for (t = 0; t < TERRITORY_TABLE_SIZE - 1; t++) {
        const int from = DATA_START[t];
        const int upto_excl = DATA_START[t + 1];
        int i;
        int first = -1;
        int count = 0;
        for (i = from; i < upto_excl; i++) {
            if (RECORD_KIND[i] & KIND_BIT_NAMELESS) {
                if (first < 0) {
                    first = i;
                }
                count++;
            }
        }
        TERRITORY_FIRST_NAMELESS[t] = first;
        TERRITORY_NAMELESS_COUNT[t] = count;
    }
    companion_initialized = 1;
}
```

Placement: insert this block immediately after the `HEADER_LETTER` macro (line 154) and BEFORE any function definitions. It depends only on `TERRITORY_BOUNDARIES` and `ENCODE_CHARS` (defined further down) — so be careful: `ENCODE_CHARS` is defined at `mapcoder.c:332`. The initializer references it. Since `initCompanionTables` is a function (not a static initializer), it's evaluated at call time, after `ENCODE_CHARS` is fully visible in the translation unit. C99 allows a function to reference any file-scope identifier as long as the identifier is declared before the function's first call. We must therefore either:
  - Insert this block AFTER `ENCODE_CHARS` (i.e. after line 332). RECOMMENDED.
  - Or insert a forward declaration of `ENCODE_CHARS` earlier in the file.

Use the RECOMMENDED option: place the entire companion-tables block immediately AFTER the `ENCODE_CHARS` definition (around line 332+, before `decodeChar` at line 352).

- [ ] **Step 2: Call `initCompanionTables()` from public entry points**

The public encode/decode entry points are:
- `encodeLatLonToMapcodes_internal` (`mapcoder.c:1549`)
- `decoderEngine` (`mapcoder.c:2651`)

At the very top of `encodeLatLonToMapcodes_internal` (after the `ASSERT(mapcodes);` etc., before `convertCoordsToMicrosAndFractions`), add:

```c
initCompanionTables();
```

At the very top of `decoderEngine` (after `ASSERT(dec);`, before `parseMapcodeString`), add:

```c
initCompanionTables();
```

Both calls are guarded by the cheap `if (companion_initialized) return;` so the call cost is one branch per call after the first.

- [ ] **Step 3: Build and run tests**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o && ./unittest 2>&1 | tail -5
```

Expected: clean compile (no unused-variable warnings on the new tables — they ARE used by the initializer), `Unit tests passed`. If anything fails, revert.

- [ ] **Step 4: Measure timing (3× best)**

Same as Task 2 Step 6. The expected delta vs. previous commit is tiny (near zero or slightly negative due to init cost on first call). That's expected; B4 is where the payoff lands.

- [ ] **Step 5: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: B1+B2+B3 — add precomputed companion tables and one-time init

Static tables RECORD_CODEX / RECORD_REC_TYPE / RECORD_KIND /
RECORD_HEADER_LETTER / RECORD_SMART_DIV are precomputed once from
TERRITORY_BOUNDARIES.flags. Per-territory TERRITORY_FIRST_NAMELESS /
TERRITORY_NAMELESS_COUNT replace linear scans.

This commit only defines the tables and calls initCompanionTables at
the top of encodeLatLonToMapcodes_internal and decoderEngine. Hot loops
still use the existing macros; the switch happens in B4.

Memory footprint: ~74 KB of additional static data.

  time ./unittest (best of 3, user):
    baseline = <FILL_IN>s
    after B3 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Task 8: B4 — Hot loops read from companion tables

**Goal:** The payoff commit. Switch the inner-loop reads in `encoderEngine`, `decoderEngine`, `firstNamelessRecord`, `countNamelessRecords` from the local `flags` cache (added in A1) to the precomputed companion tables.

**Files:**
- Modify: `mapcodelib/mapcoder.c` — same loop bodies edited in Task 2 (A1), plus the two nameless helpers.

- [ ] **Step 1: Replace `encoderEngine` loop body to use companion tables**

Locate the loop edited in Task 2 (the `for (i = from; i <= upto; i++)` in `encoderEngine`). Replace the local `flags`-based extractions with direct table reads:

```c
for (i = from; i <= upto; i++) {
    const unsigned char kind   = RECORD_KIND[i];
    const unsigned char recTypeI = RECORD_REC_TYPE[i];
    if (fitsInsideBoundaries(&enc->coord32, TERRITORY_BOUNDARY(i))) {
        if (kind & KIND_BIT_NAMELESS) {
            encodeNameless(result, enc, ccode, extraDigits, i);
        }
        else if (recTypeI > 1) {
            encodeAutoHeader(result, enc, i, extraDigits);
        }
        else if ((i == upto) && isSubdivision(ccode)) {
            encoderEngine(parentTerritoryOf(ccode), enc, stop_with_one_result, extraDigits, requiredEncoder,
                          ccode);
            return;
        }
        else // must be grid
        {
            if (result_counter || !(kind & KIND_BIT_RESTRICTED)) {
                if (RECORD_CODEX[i] < 54) {
                    const char headerletter = (char)((recTypeI == 1) ? RECORD_HEADER_LETTER[i] : 0);
                    encodeGrid(result, enc, i, extraDigits, headerletter);
                }
            }
        }

        // =========== handle result (if any)
        if (*result) {
            result_counter++;

            repackIfAllDigits(result, 0);

            if ((requiredEncoder < 0) || (requiredEncoder == i)) {
                const enum Territory ccodeFinal = (ccode_override != TERRITORY_NONE ? ccode_override : ccode);
                if (*result && enc->mapcodes && (enc->mapcodes->count < MAX_NR_OF_MAPCODE_RESULTS)) {
                    char* s = enc->mapcodes->mapcode[enc->mapcodes->count++];
                    if (ccodeFinal == TERRITORY_AAA) {
                        strcpy(s, result);
                    }
                    else {
                        getTerritoryIsoName(s, ccodeFinal, 0);
                        size_t isoLen = strlen(s);
                        size_t resultLen = strlen(result);
                        s[isoLen] = ' ';
                        memcpy(s + isoLen + 1, result, resultLen + 1);
                    }
                }
                if (requiredEncoder == i) {
                    return;
                }
            }
            if (stop_with_one_result) {
                return;
            }
            *result = 0;
        }
    }
} // for i
```

The local `const int flags = ...` line introduced in A1 is removed (no longer used inside this loop).

- [ ] **Step 2: Replace `decoderEngine` loop body to use companion tables**

Locate the corresponding loop in `decoderEngine`. Replace with:

```c
for (i = from; i <= upto; i++) {
    const unsigned char kind   = RECORD_KIND[i];
    const int codexi           = RECORD_CODEX[i];
    const int r                = RECORD_REC_TYPE[i];
    if (r == 0) {
        if (kind & KIND_BIT_NAMELESS) {
            if (((codexi == 21) && (codex == 22)) ||
                ((codexi == 22) && (codex == 32)) ||
                ((codexi == 13) && (codex == 23))) {
                err = decodeNameless(dec, i);
                break;
            }
        }
        else {
            if ((codexi == codex) || ((codex == 22) && (codexi == 21))) {
                err = decodeGrid(dec, i, 0);

                // first of all, make sure the zone fits the country
                restrictZoneTo(&dec->zone, &dec->zone, TERRITORY_BOUNDARY(upto));

                if ((err == ERR_OK) && (kind & KIND_BIT_RESTRICTED)) {
                    int nrZoneOverlaps = 0;
                    int j;

                    dec->result = getMidPointFractions(&dec->zone);
                    dec->coord32 = convertFractionsToCoord32(&dec->result);
                    for (j = i - 1; j >= from; j--) {
                        if (!(RECORD_KIND[j] & KIND_BIT_RESTRICTED)) {
                            if (fitsInsideBoundaries(&dec->coord32, TERRITORY_BOUNDARY(j))) {
                                nrZoneOverlaps = 1;
                                break;
                            }
                        }
                    }

                    if (!nrZoneOverlaps) {
                        MapcodeZone zfound;
                        TerritoryBoundary prevu;
                        for (j = from; j < i; j++) {
                            if (!(RECORD_KIND[j] & KIND_BIT_RESTRICTED)) {
                                MapcodeZone z;
                                if (restrictZoneTo(&z, &dec->zone, TERRITORY_BOUNDARY(j))) {
                                    nrZoneOverlaps++;
                                    if (nrZoneOverlaps == 1) {
                                        zoneCopyFrom(&zfound, &z);
                                        ASSERT(j <= MAPCODE_BOUNDARY_MAX);
                                        memcpy(&prevu, TERRITORY_BOUNDARY(j), sizeof(TerritoryBoundary));
                                    }
                                    else {
                                        break;
                                    }
                                }
                            }
                        }

                        if (nrZoneOverlaps == 1) {
                            zoneCopyFrom(&dec->zone, &zfound);
                        }
                    }

                    if (!nrZoneOverlaps) {
                        err = ERR_MAPCODE_UNDECODABLE;
                    }
                }
                break;
            }
        }
    }
    else if (r == 1) {
        if (codex == codexi + 10 && RECORD_HEADER_LETTER[i] == (unsigned char) *s) {
            err = decodeGrid(dec, i, 1);
            break;
        }
    }
    else {
        if (((codex == 23) && (codexi == 22)) ||
            ((codex == 33) && (codexi == 23))) {
            err = decodeAutoHeader(dec, i);
            break;
        }
    }
} // for
```

The inner `j` loops now also use `RECORD_KIND[j] & KIND_BIT_RESTRICTED` (this is a small extra win since the same loop ran the `IS_RESTRICTED(j)` macro repeatedly).

- [ ] **Step 3: Replace `firstNamelessRecord` and `countNamelessRecords` to use tables**

For `firstNamelessRecord`:

```c
static int firstNamelessRecord(const int m, const int firstcode) {
    int i = m;
    const int codexm = RECORD_CODEX[m];
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (i >= firstcode && RECORD_CODEX[i] == codexm && (RECORD_KIND[i] & KIND_BIT_NAMELESS)) {
        i--;
    }
    return (i + 1);
}
```

For `countNamelessRecords`:

```c
static int countNamelessRecords(const int m, const int firstcode) {
    const int first = firstNamelessRecord(m, firstcode);
    const int codexm = RECORD_CODEX[m];
    int last = m;
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (RECORD_CODEX[last] == codexm) {
        last++;
    }
    ASSERT((0 <= last) && (last <= MAPCODE_BOUNDARY_MAX));
    ASSERT(last >= first);
    return (last - first);
}
```

- [ ] **Step 4: Build and run tests**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o && ./unittest 2>&1 | tail -5
```

Expected: `Unit tests passed`. Any failure → immediate revert.

- [ ] **Step 5: Measure timing (3× best)**

Same as Task 2 Step 6. This should show the largest single-commit speedup.

- [ ] **Step 6: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: B4 — hot loops read from precomputed companion tables

Replace per-iteration mask/shift extractions in encoderEngine,
decoderEngine, firstNamelessRecord, countNamelessRecords with direct
reads from RECORD_KIND / RECORD_CODEX / RECORD_REC_TYPE /
RECORD_HEADER_LETTER. The companion tables are byte-sized so each
field is a single byte load with friendlier cache behavior than the
4-byte flags field.

Values are derived from the same macros they replace (see B3 init);
output is bit-exact.

  time ./unittest (best of 3, user):
    baseline = <FILL_IN>s
    after B4 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Task 9: B5 — Debug-build sanity check

**Goal:** In `#ifdef DEBUG` builds only, have `initCompanionTables` assert that every precomputed value matches the macro-derived value. Free correctness guarantee during development.

**Files:**
- Modify: `mapcodelib/mapcoder.c` — the `initCompanionTables` function.

- [ ] **Step 1: Add the debug check at the end of `initCompanionTables`**

Just before `companion_initialized = 1;` (the last line of `initCompanionTables`), add:

```c
#ifdef DEBUG
    for (m = 0; m <= MAPCODE_BOUNDARY_MAX; m++) {
        const int flags = TERRITORY_BOUNDARIES[m].flags;
        const int c = flags & 31;
        const int codex_val = 10 * (c / 5) + ((c % 5) + 1);
        ASSERT(RECORD_CODEX[m] == (unsigned char) codex_val);
        ASSERT(RECORD_REC_TYPE[m] == (unsigned char) ((flags >> 7) & 3));
        ASSERT(((RECORD_KIND[m] & KIND_BIT_NAMELESS) != 0) == ((flags & 64) != 0));
        ASSERT(((RECORD_KIND[m] & KIND_BIT_RESTRICTED) != 0) == ((flags & 512) != 0));
        ASSERT(((RECORD_KIND[m] & KIND_BIT_SPECIAL_SHAPE) != 0) == ((flags & 1024) != 0));
        ASSERT(RECORD_HEADER_LETTER[m] == (unsigned char) ENCODE_CHARS[(flags >> 11) & 31]);
        ASSERT(RECORD_SMART_DIV[m] == (unsigned short) ((unsigned int) flags >> 16));
    }
#endif
```

(`m` is already declared at the top of `initCompanionTables`, so no new declarations needed.)

- [ ] **Step 2: Build and run tests in BOTH O3 and DEBUG modes**

```bash
# -O3 release (no debug assertions)
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o && ./unittest 2>&1 | tail -5

# DEBUG build (with assertions)
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O0 -DDEBUG -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O0 -DDEBUG unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o && ./unittest 2>&1 | tail -5
```

Both must report `Unit tests passed`. The DEBUG run additionally exercises the new `ASSERT` block in `initCompanionTables` — if any precomputed value diverges from the macro-derived one, the DEBUG build aborts (which is exactly what we want).

- [ ] **Step 3: Re-build -O3 (so the binary in test/ is the release one for timing)**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/mapcodelib && gcc -O3 -c mapcoder.c
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && gcc -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
```

- [ ] **Step 4: Measure timing (3× best)**

Same as Task 2 Step 6. Expected: delta from previous commit is near zero (the new code is `#ifdef DEBUG` gated and absent in -O3).

- [ ] **Step 5: Commit**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add mapcodelib/mapcoder.c
git commit -m "$(cat <<'EOF'
perf: B5 — DEBUG-build sanity check for companion tables

Under -DDEBUG, initCompanionTables asserts that every precomputed value
matches the macro-derived one. Free correctness guard during development;
zero cost in release.

Verified: both -O3 and -O0 -DDEBUG builds pass the unit suite.

  time ./unittest -O3 (best of 3, user):
    baseline = <FILL_IN>s
    after B5 = <FILL_IN>s
    delta    = <FILL_IN>% cumulative

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Task 10: Final summary commit (mandatory)

**Goal:** Single commit summarizing the full optimization arc: baseline, per-step deltas, and the cumulative win. Useful for review and for future archaeology.

**Files:**
- Modify: `docs/superpowers/specs/2026-05-28-mapcodelib-optimize-design.md` — append a results section.

- [ ] **Step 1: Append a results section to the spec**

Open `docs/superpowers/specs/2026-05-28-mapcodelib-optimize-design.md` and append at the bottom:

```markdown
## 8. Results

Measurements taken on branch `feat/optimize`, `-O3` build, `time ./unittest`
(best `user` of 3 back-to-back runs).

| Commit | Description | Best user time | Delta vs. baseline |
|--------|-------------|---------------|--------------------|
| Task 1 | baseline | <FILL_IN>s | 0.0% |
| Task 2 | A1 — cache flags per iteration | <FILL_IN>s | <FILL_IN>% |
| Task 3 | A2 — reorder fitsInsideBoundaries | <FILL_IN>s | <FILL_IN>% |
| Task 4 | A3 — length-tracked result assembly | <FILL_IN>s | <FILL_IN>% |
| Task 5 | A4 — tighten encodeBase31 | <FILL_IN>s | <FILL_IN>% |
| Task 6 | A5 — repack early-exit | <FILL_IN>s | <FILL_IN>% |
| Task 7 | B3 — companion tables init (no hot-path use yet) | <FILL_IN>s | <FILL_IN>% |
| Task 8 | B4 — hot loops use companion tables | <FILL_IN>s | <FILL_IN>% |
| Task 9 | B5 — DEBUG sanity check | <FILL_IN>s | <FILL_IN>% |

**Final cumulative speedup:** <FILL_IN>%

**Success criteria check:**
- [ ] All unit tests passing after every commit (verified)
- [ ] Binary size growth ≤ 100 KB (verified)
- [ ] ≥20% wall-time reduction (target — record whether achieved)
- [ ] ≥30% wall-time reduction (stretch — record whether achieved)
```

Fill in all `<FILL_IN>` values from the prior commit messages.

- [ ] **Step 2: Verify binary size delta**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git stash || true
git checkout 740bfec -- mapcodelib/mapcoder.c
cd mapcodelib && gcc -O3 -c mapcoder.c && BASELINE_SIZE=$(stat -f%z mapcoder.o)
cd ..
git checkout HEAD -- mapcodelib/mapcoder.c
git stash pop 2>/dev/null || true
cd mapcodelib && gcc -O3 -c mapcoder.c && OPTIMIZED_SIZE=$(stat -f%z mapcoder.o)
echo "Baseline .o:   $BASELINE_SIZE bytes"
echo "Optimized .o:  $OPTIMIZED_SIZE bytes"
echo "Delta:         $((OPTIMIZED_SIZE - BASELINE_SIZE)) bytes"
```

If delta > 100,000 bytes (100 KB) the optimization violated a hard constraint — investigate before committing.

- [ ] **Step 3: Final test pass**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp/test && ./unittest 2>&1 | tail -5
```

Expected: `Unit tests passed`.

- [ ] **Step 4: Commit summary**

```bash
cd /Users/rijn/source/mapcode-foundation/mapcode-cpp
git add docs/superpowers/specs/2026-05-28-mapcodelib-optimize-design.md
git commit -m "$(cat <<'EOF'
docs: record final perf results for feat/optimize branch

Appends a results table to the design spec capturing per-commit timing
deltas and the cumulative speedup vs. the Task 1 baseline.

Final speedup: <FILL_IN>% wall-time on `time ./unittest` at -O3.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Self-review

Spec coverage check:
- §1 scope/goals → covered by all tasks; bit-exact guardrail enforced by Step "run tests" in every task.
- §2 measurement methodology → Task 1 establishes baseline; every subsequent task records timing.
- §3 Approach A (A1–A5) → Tasks 2–6.
- §4 Approach B (B1–B5) → Tasks 7–9.
- §4 "Interaction with A1" → Task 8 Step 1 explicitly removes the A1 local `flags` cache when switching to companion tables.
- §5 workflow → matches Tasks 1–9.
- §6 risks → mitigated by "test after every step + revert on failure" in each task; debug-build assertion in Task 9.
- §7 success criteria → Task 10 verifies binary size and produces the final results table.

Placeholder scan: `<FILL_IN>` markers are intentional — they're per-run measurements the implementer fills in at commit time. No `TBD`/`TODO`/"implement later" anywhere.

Type consistency: `unsigned char` used uniformly for KIND/CODEX/REC_TYPE/HEADER_LETTER; `unsigned short` for SMART_DIV; `int` for TERRITORY_FIRST_NAMELESS/COUNT (matches existing code conventions for record indices). Names `KIND_BIT_NAMELESS` / `KIND_BIT_RESTRICTED` / `KIND_BIT_SPECIAL_SHAPE` used consistently across Tasks 7 and 8.
