# P0.3 — Baseline measurement results (B1–B6)

Cross-matrix from [CChildProcess-repro-matrix-P0.2.md](CChildProcess-repro-matrix-P0.2.md).  
Environment: [NppExec-dev-environment.md](NppExec-dev-environment.md).

**Date:** 2026-05-25  
**Build:** `NppExec_build_vc2017_x64.bat` → `NppExec\x64\Release\NppExec.dll` (1263616 bytes, 2026-05-25 21:09)  
**Deploy:** copied to `C:\Progs\Progs\Notepadpp-x64\plugins\NppExec\NppExec.dll` (same size/time)  
**OS:** Windows 10 (agent host)  
**Python:** 3.10.10  
**Git:** present (`C:\Program Files\Git\cmd\git.exe`)

---

## Measurement methods

| Method | Covers | Notes |
|--------|--------|-------|
| **A — Pipe harness** | R01, R02, R03, R05, R08 (CFG-A semantics) | `docs/tools/nppe_readloop_harness.py` mimics `readPipesAndOutput` + outer `WaitForSingleObject(120ms)` + startup 240 ms |
| **B — N++ manual** | B2 (PseudoCon), B4–B6 (RichEdit, stdin, ANSI) | Run `docs/baseline/P0.3/*.npe` in deployed Notepad++; record subjective + wall clock below |

Harness does **not** load NppExec.dll; it validates the **pipe-read algorithm** NppExec uses. Full UI baseline still needs method **B** (checkboxes in § Manual NppExec runs).

---

## A — Automated pipe harness (CFG-A proxy for B1 / B3 / parts of B4)

Command: `python docs/tools/nppe_readloop_harness.py`

| Case | TTFB (ms) | Total (ms) | Tail after exit (ms) | Bytes | Stall* | Pass vs expectation |
|------|-----------|------------|----------------------|-------|--------|---------------------|
| **R01** | 269 | 269 | 0 | 7 (`hello`) | N | OK — immediate |
| **R02** | 250 | 1564 | 0 | 15 (lines 0–4) | N | **Degraded** — first byte only after ~startup; lines not visible incrementally at 300 ms cadence in one read quantum |
| **R08** | 250 | 5093 | 0 | 12 (`start`+`end`) | N | **Harness OK** — `start` within 250 ms; NppExec UI may still differ (see manual) |
| **R05** | 250 | 381 | 0 | 9007 | N | OK — full burst + `END` |
| **R03** | 382 | 1963 | 0 | 18 | N | Partial — `\r` overwrite not simulated; raw PowerShell `-NoNewline` concat |

\*Stall = at t=5.5 s while child alive, zero bytes received (R08 script uses 5 s sleep in harness).

**Interpretation for P1:**

- Outer **120 ms** poll + inner **10 ms** sleep + **PeekNamedPipe** gate produces ~**250 ms** minimum TTFB even when child exits instantly (R01).
- **R02** total ~1.5 s matches child runtime but **TTFB** is dominated by startup wait, not first flushed line — aligns with [4.8.3](NppExec_Manual/4.8.3.html) risk.
- **R05** shows large payload drains in one pass after first successful peek (no truncation at 9000 B).

---

## B — Manual NppExec runs (fill after executing in Notepad++)

**Host:** `C:\Progs\Progs\Notepadpp-x64\notepad++.exe`  
**Scripts:** `docs\baseline\P0.3\`

| Run | Script file | Status | TTFB / notes | Stall (R08) | ANSI colors (R04) |
|-----|-------------|--------|--------------|-------------|-------------------|
| **B1** | `B1-pipes-R01-R02-R08.npe` | **done** (2026-05-25) | See § B1 detail below | **N** — `start` before 30 s sleep | n/a |
| **B2** | `B2-pseudocon-R01-R02-R08.npe` | *pending user* | | | n/a |
| **B3** | `B3-R05-burst.npe` | *pending user* | | n/a | n/a |
| **B4** | `B4-R03-R06.npe` | *pending user* | R06: type `echo from_keyboard` | n/a | n/a |
| **B5** | `B5-R04-ansi-raw.npe` | *pending user* | e0 raw escapes visible? | n/a | Y/N |
| **B6** | `B6-R04-ansi-pseudocon.npe` | *pending user* | | n/a | Y/N |

**Quick manual procedure:**

1. Restart Notepad++ after DLL deploy.
2. Plugins → NppExec → Show Console.
3. `npp_exec` path to script, or paste script into Execute dialog.
4. For **R08** in B1/B2: note whether `start` appears before 30 s sleep ends.

### B1 — NppExec Console results (user run, CFG-A `u- e1`)

Settings echoed each block: `pseudocon: off`, `esc_seq: remove`, `out_enc: OEM`, `in_enc: OEM`.

| Case | Result | Stall? | Notes |
|------|--------|--------|-------|
| **R01** `cmd /c echo hello` | **Pass** | — | `hello` between `Process started` and `Process finished` (PID 11360) |
| **R02** `python -u` × 5 lines | **Pass** | — | Lines `0`–`4` before exit (PID 35532) — **optimistic**: `-u` forces unbuffered I/O |
| **R08** `python -u` + `flush=True` | **Pass** | **No** | `start` before 30 s sleep (PID 47816) — **optimistic** for same reason |

**Important (user note):** `-u` is Python-specific. Most other programs have **no** unbuffered mode; they behave like Python **without** `-u` — stdout may not reach the pipe until the buffer fills or the process **ends**. B1 does **not** validate that common case.

### B1b — Buffered stdout (user run, CFG-A `u- e1`)

Typical program behavior: **no** `python -u`. Matches [4.8.3](NppExec_Manual/4.8.3.html) §9 — **primary P1 target**.

| Case | Result | Stall? | Notes |
|------|--------|--------|-------|
| **R02b** buffered loop | **Fail** (unchanged after P1.3) | **Yes** | PID 41448: `0`…`4` only with `Process finished` in one burst (no live lines during 0.3 s sleeps) |
| **R08b** `print('start'); sleep(30); print('end')` | **Fail** (unchanged after P1.3) | **Yes** | PID 9768 (P0.3), 41448 (post-P1.3): silence after `Process started` until exit burst |

**R08b — observed sequence:**

1. Command echoed, `Process started (PID=9768) >>>`
2. **No output for ~30 s** while child alive
3. At exit: `start`, `end`, finish message in one burst

Contrast **B1 R08** (`python -u` + `flush`): `start` appeared before sleep.

**Post-P1.3 (blocking reader, no Peek):** B1b **unchanged** — not a pipe-read bug. Python (and most CRT apps) keep stdout **fully buffered** when connected to a pipe; bytes are not written until the buffer fills or the process exits. NppExec cannot display what the child never sends.

**P1.4+** (exit drain, less sleep) improves **leftover bytes at exit**, not live streaming of buffered stdout.

### B1c — gawk: buffered `print` while blocked on `getline` (user, CFG-A `u- e1`)

Command:

```bat
C:\Progs\Progs\gawk\gawk.exe "BEGIN { print 123; getline s }"
```

| Field | Value |
|-------|--------|
| **Result** | **Fail** (live output) — **not** a deadlock (V3/V4 pass after pipe-close fix) |
| **Stall?** | Yes — no `123` until stdin activity |
| **PID example** | 35260 |

**Observed sequence:**

1. `Process started (PID=35260) >>>` — no `123` yet (gawk blocked in `getline`, `print 123` still in stdio buffer).
2. User types `aaa` + Enter in Console → NppExec `WriteInput` → gawk receives stdin.
3. Then console shows: `aaa`, then `123`, then `<<< Process finished`.

**Why:** With stdout connected to a **pipe** (not a TTY), gawk uses **full buffering** for `print`. `getline` waits for a line on stdin; nothing flushes stdout until stdin I/O runs (or exit). NppExec already read the pipe correctly once bytes were sent — the child had not sent them yet.

**Contrast:** In a real terminal, stdout is often **line-buffered** and/or the user sees prompts differently; ConPTY (Phase 3) may change this class of behavior.

**Workarounds (child side):** `print 123 > "/dev/stderr"` (not ideal on Windows), or `fflush()` via `system("cmd /c")`, or restructure so `print` follows stdin; **not** an NppExec read-loop bug.

---

## Cross-matrix rollup

| Run | Config | Harness (A) | NppExec UI (B) |
|-----|--------|-------------|----------------|
| **B1** | u- e1 | R01/R02/R08 table above | **R01/R02/R08 pass** — R02/R08 used `-u` (optimistic) |
| **B1b** | u- e1 | — | **R02b/R08b fail** — same after P1.3 (PID 41448) |
| **B2** | u+ e1 | PseudoCon not in harness | *pending* |
| **B3** | u- e1 | R05 table above | *pending* (confirm in Console) |
| **B4** | u- e1 | R03 harness only | *pending* R06 keyboard |
| **B5** | u- e0 | n/a | *pending* |
| **B6** | u+ e1 | n/a | *pending* |

**Phase P0 complete when:** B column filled for B1–B6 OR explicitly marked “same as harness / not tested” with reason.

---

## Comparison baseline for P1

Store after P1 implementation:

| Metric | P0.3 baseline | P1 target |
|--------|---------------|-----------|
| **R02b** buffered (no `-u`) | **Burst at exit** (P0.3 + post-P1.3) | Lines visible while child runs — needs child unbuffered or ConPTY (P3), not pipe peek |
| **R08b** buffered `start` | **Burst at exit** (P0.3 + post-P1.3) | `start` during sleep — same caveat |
| R02/R08 with `python -u` | B1 **pass** (optimistic only) | Keep passing (regression) |
| R01 TTFB | Harness ~269 ms; B1 pass | Optional: lower latency |

---

## Changelog

| Date | Change |
|------|--------|
| 2026-05-25 | B1c: gawk print+getline — 123 after stdin (PID 35260); stdin/flushes buffer. |
| 2026-05-25 | Post-P1.3 B1b: R02b+R08b unchanged (PID 41448); confirms child-side buffering. |
| 2026-05-25 | B1b R08b recorded: 30 s silence, then start+end at exit (user). |
| 2026-05-25 | Clarified B1 R02/R08 use `python -u`; added R02b/R08b + B1b script. |
| 2026-05-25 | B1 NppExec UI results recorded (user); R08 no stall with python -u. |
| 2026-05-25 | Initial results: build, deploy, harness; N++ manual pending. |
