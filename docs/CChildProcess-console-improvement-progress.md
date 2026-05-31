# CChildProcess Console Plan — Progress Tracker

**This file is the only place to record implementation status.**  
Strategy and architecture: [CChildProcess-console-improvement-plan.md](CChildProcess-console-improvement-plan.md).

---

## How to use (humans and agents)

### Before starting work

1. Read this file top to bottom: **Phase summary**, **Current focus**, **Design decisions**, then the step table for your phase.
2. Do **not** skip ahead to a later phase if earlier required steps are not `done` (unless explicitly `skipped` with a note).
3. Prefer the **lowest-numbered** step that is `not_started` or resume the single `in_progress` step.

### When starting a step

1. Set that step’s **Status** to `in_progress`.
2. Update **Current focus** (below) with step ID and short title.
3. Set **Last updated** to today’s date (ISO `YYYY-MM-DD`).

### When finishing a step

1. Set **Status** to `done`.
2. Fill **Completed**, **Commit** (hash or PR link), **Verified** (`build` / `manual` / `both` / `n/a`).
3. Add **Notes** if the next agent needs context (files touched, caveats, follow-ups).
4. Update the **Phase summary** row (counts and commit).
5. Clear **Current focus** or point it at the next step.
6. Append one line to **Session log** at the bottom.

### Status values

| Value | Meaning |
|-------|---------|
| `not_started` | No work yet |
| `in_progress` | Active now — only one step should use this at a time |
| `done` | Implemented and verified per **Verified** column |
| `blocked` | Waiting on design decision or external dependency — explain in **Notes** |
| `skipped` | Intentionally not doing — explain in **Notes** |
| `deferred` | Postponed to a later release |

### Verification

- **build:** `NppExec_build_vc2017_x64.bat` succeeded (or documented why N/A).
- **manual:** Phase 0 matrix case or step-specific repro run.
- **both:** build + manual.

**Phase 1 (`CChildProcess` / `CChildProcessIo`):** `both` is **not sufficient** unless the plan **Verification gate** V2–V4 passed — see `CChildProcess-console-improvement-plan.md` §5 Phase 1. Minimum:

| ID | Requirement |
|----|-------------|
| V2 | `cmd /c echo hello` + finish message |
| V3 | Break while child alive (`gawk.exe` or `cmd /k` → Ctrl+C or close Console; N++ still exits) |
| V4 | Short command exits with `<<< Process finished` (e.g. `gawk -e "print 1"`) |

Record V3/V4 explicitly in **Notes**. B1b buffered burst-at-exit alone is **not** a failure if V3–V4 pass.

**Agent rule:** Read plan **§2.8 invariants** before coding; use `.cursor/skills/code-modification/SKILL.md` deadlock checklist.

**Deploy (64-bit):** see `docs/NppExec-dev-environment.md`.

---

## Current focus

| Field | Value |
|-------|-------|
| **Step ID** | P5.1 (next) |
| **Title** | Cooked mode preserved (regression check) |
| **Agent / session** | |
| **Started** | |

---

## Design decisions (blockers)

Record answers here; steps marked `blocked` depend on these.

| ID | Question | Answer | Decided | Notes |
|----|----------|--------|---------|-------|
| D1 | Target scope A / A+B / C? | *pending* | | |
| D2 | RichEdit prompt-at-bottom vs separate terminal UI? | *pending* | | |
| D3 | PseudoConsole on by default? | yes | 2026-05-27 | Default `u+`; `npe_console u-` remains opt-out |
| D4 | Split `Create()` → `Start`/`WaitForExit` OK? | yes (Phase 2) | 2026-05-26 | Default `Create()`/`Do()` still block until exit |
| D5 | Filters on raw bytes / lines / post-VT? | *pending* | | |
| D6 | No application-specific child-process logic? | **yes** | 2026-05-27 | Generic transport + VT/input only; no per-app flags/heuristics; **allowed exception**: `cmd` skips Ctrl-Break/Ctrl-C in kill list (plan §1.1.1) |

---

## Phase summary

| Phase | Status | Steps done | Last updated | Last commit / PR |
|-------|--------|------------|--------------|------------------|
| **P0** Baseline | `done` | 3/3 | 2026-05-25 | docs + harness |
| **P1** Pipe I/O | `done` | 8/8 | 2026-05-26 | uncommitted; pipe mode = legacy readPipesDirect |
| **P2** Lifecycle API | `done` | 4/4 | 2026-05-27 | P2.4 boundary audit complete; V2–V4 user OK |
| **P3** PseudoConsole | `done` | 6/6 | 2026-05-27 | P3.5/P3.6 docs audits done: keep global `CreatePseudoConsole` `flags=0`; ConPTY encoding aligned to UTF-8 |
| **P4** VT + RichEdit | `done` | 6/6 | 2026-05-27 | P4.6 docs added: RichEdit/grid limits + current `e2` scope/constraints |
| **P5** Input enhancements | `in_progress` | 0/5 | 2026-05-27 | P5.1 next |
| **P5** Input enhancements | `not_started` | 0/5 | | |
| **P6** Optional tracks | `not_started` | 0/1 | | |
| **M1** Milestone | `not_started` | requires P1 | | |
| **M2** Milestone | `not_started` | requires P1+P3 | | |
| **M3** Milestone | `not_started` | requires P4 | | |
| **M4** Milestone | `not_started` | requires P5 | | |

**Phase status** = `done` only when all non-skipped steps in that phase are `done`.

---

## Step tracker

### Phase 0 — Baseline and tests

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P0.1 | Inventory `CChildProcess` / `WriteInput` callers | `done` | 2026-05-25 | docs only | n/a | See [CChildProcess-callers-P0.1.md](CChildProcess-callers-P0.1.md) |
| P0.2 | Document repro matrix (see plan §5 Phase 0) | `done` | 2026-05-25 | docs only | n/a | [CChildProcess-repro-matrix-P0.2.md](CChildProcess-repro-matrix-P0.2.md) |
| P0.3 | Baseline metrics: latency, unread bytes at exit | `done` | 2026-05-25 | build+deploy+harness | both | [baseline-P0.3](CChildProcess-baseline-P0.3.md); **B1 UI done**; B2–B6 pending |

### Phase 1 — Pipe I/O reliability

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P1.1 | Extract I/O from `Create()` (`CChildProcessIo` or equivalent) | `done` | 2026-05-25 | uncommitted | build | `ChildProcessIo.h/.cpp`; pipes/PseudoCon in `m_Io`; `Create()` delegates |
| P1.2 | Dedicated stdout reader thread | `done` | 2026-05-25 | uncommitted | build | Reader enqueues stdout; script thread drains queue → `processOutputBuffer` → `PrintOutput` |
| P1.3 | Remove `PeekNamedPipe` gate; reliable read | `done` | 2026-05-25 | uncommitted | build+manual | Reader: blocking `ReadFile`; B1b R02b/R08b **unchanged** (child stdout buffering, not peek) |
| P1.4 | Drain pipe on process exit | `done` | 2026-05-25 | uncommitted | both | V3: `gawk` (no args) — no deadlock; V4: `gawk -e "BEGIN { print 1 }"` OK (user 2026-05-25) |
| P1.5 | Reduce `Sleep(10)` / decouple process wait from read | `done` | 2026-05-25 | uncommitted | both | V2: `cmd` prompt OK; partial-line flush; wait decoupling |
| P1.6 | Queued stdin writes for `WriteInput` | `done` | 2026-05-25 | uncommitted | both | V2–V4 + interactive cmd OK (user 2026-05-25); writer thread + queue |
| P1.7 | `FlushFileBuffers` only when explicitly requested | `done` | 2026-05-25 | uncommitted | build | Audit: no caller uses bFFlush=true; §2.8 + callers §4.1 + manual note |
| P1.8 | Stderr / interactive parity | `done` | 2026-05-26 | uncommitted | both | Separate stderr pipe **reverted** (merged into stdout, legacy HEAD). Hot path: `readPipesDirect` on script thread (no reader queue in `Create()`). V2–V4 + `python -u -i` OK; **V5/V6**: B1 + B1b `.npe` same as HEAD (user 2026-05-26) |

**P1 exit criteria:** P0.2 cases pass better than baseline; manual §9 updated if behavior changed. **Met** for pipe-mode parity with HEAD on baseline scripts.

### Phase 2 — Lifecycle API

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P2.1 | `Start` / `WaitForExit` / `IsRunning` / `Stop` API | `done` | 2026-05-27 | uncommitted | both | V2–V4 user OK (2026-05-27); `Create()` = `Start` + `WaitForExit(INFINITE)` |
| P2.2 | Move poll loop out of `Create()` | `done` | 2026-05-27 | uncommitted | both | V2–V4 user OK (2026-05-27); `Do()` → `Start` + `WaitForExit(INFINITE)` |
| P2.3 | `GetRunningChildProcess()` valid for full run | `done` | 2026-05-27 | uncommitted | both | `ExecState` gates on `pChildProcess->IsRunning()`; `pChildProcess` kept until `Do()` reset; V2–V4 user OK |
| P2.4 | `WriteChildProcessInput*` boundary unchanged | `done` | 2026-05-27 | uncommitted | n/a | Boundary unchanged: `WriteChildProcessInput*` still delegates to `GetRunningChildProcess()->WriteInput(...)` with same signatures/defaults |

### Phase 3 — PseudoConsole

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P3.1 | Default PseudoConsole policy (needs D3) | `done` | 2026-05-27 | uncommitted | both | Default `u+`; ConPTY startup must match HEAD (`UpdateProcThreadAttribute` passes `m_hPseudoCon`, not `&`) |
| P3.2 | Dynamic rows/cols from RichEdit/font | `done` | 2026-05-27 | uncommitted | both | Size from RichEdit/font metrics; user OK. Note: `u+` input echo duplication observed (`gawk`, `python -i`), deferred to P5 policy. |
| P3.3 | `ResizePseudoConsole` on resize/font change | `done` | 2026-05-27 | uncommitted | build | `OnSize` + `SetConsoleFont` call `ResizeRunningChildPseudoConsole()` |
| P3.4 | Replace fixed 192×1000 | `done` | 2026-05-27 | uncommitted | build | folded into P3.2 (defaults 192×50, max rows 1000) |
| P3.5 | Review global `CreatePseudoConsole` flags | `done` | 2026-05-27 | docs audit | n/a | `CreatePseudoConsole(..., flags=0, ...)` confirmed in `ChildProcessIo.cpp`; keep `flags=0` (no global need for `PSEUDOCONSOLE_INHERIT_CURSOR`); no per-app branching (§1.1). |
| P3.6 | Encoding alignment / optional auto-detect | `done` | 2026-05-27 | docs audit | n/a | ConPTY path already aligned globally: `GetEncoding()` returns `getPseudoConsoleEncoding()` in pseudo mode, and `getPseudoConsoleEncoding()` is UTF-8 input+output; no per-app auto-detect added. |

### Phase 4 — VT parser + RichEdit

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P4.1 | `CConsoleVtParser` (or equivalent) byte stream | `done` | 2026-05-27 | uncommitted | build | Added `ConsoleVtParser.h/.cpp` (stateful stream parser); `escProcess` now routes through parser state (stdout/stderr instances), while `escRemove` keeps legacy one-shot stripping. |
| P4.2 | Minimum VT: SGR, EL/ED, cursor motion, `\r` | `done` | 2026-05-27 | uncommitted | build | `escProcess` now applies basic CSI actions: SGR foreground colors (incl. reset), EL/ED mapped to line/screen clear, cursor-left (`CSI n D`) mapped to backspace deletes; `\r` remains existing `ProcessSlashR` path. |
| P4.3 | Map VT ops to RichEdit output region | `done` | 2026-05-27 | uncommitted | build | Added output-region-aware VT console ops (`VtEraseLineInOutputRegion`, `VtEraseScreenInOutputRegion`, `VtBackspaceInOutputRegion`) and wired parser actions to them, preserving `nConsoleFirstUnlockedPos` input area. |
| P4.4 | Real `escProcess` (not alias of remove) | `done` | 2026-05-27 | uncommitted | build | `escProcess` already active in runtime; finalized by removing stale “remove-only” TODO wording and updating `npe_console` help/comments to `e0/e1/e2` (raw/remove/process), confirming non-alias semantics. |
| P4.5 | Thin `readPipesAndOutput` → parser queue | `done` | 2026-05-27 | uncommitted | build | `readPipesDirect` now follows read→queue→process: `readOnePipeDirect` enqueues raw chunks into `CChildProcessIo` output queue and `processOutputQueue` drains/decode/renders centrally, preserving prompt/partial-line behavior. |
| P4.6 | Document RichEdit/grid limitations | `done` | 2026-05-27 | docs | n/a | Plan updated with explicit Phase 4.6 limitations table: current VT capabilities, RichEdit/grid constraints, deferred semantics, and operational guidance (`e2`, monospace, `u-` fallback). |

### Phase 5 — Input enhancements (`WriteInput` path)

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P5.1 | Cooked mode preserved (regression check) | `not_started` | | | | |
| P5.2 | Raw mode: keys → bytes via `WriteInput` | `not_started` | | | | |
| P5.3 | Signals: Ctrl+C / Break + ConPTY | `not_started` | | | | |
| P5.4 | `proc_input` / `proc_signal` encoding parity | `not_started` | | | | |
| P5.5 | Resize notifies child (depends P3.3) | `not_started` | | | | |

### Phase 6 — Optional tracks

| ID | Step | Status | Completed | Commit / PR | Verified | Notes |
|----|------|--------|-----------|-------------|----------|-------|
| P6.0 | Choose track: 6a / 6b / 6c / none | `not_started` | | | | default recommendation: 6b = N/A |

---

## Milestones (rollup)

| ID | Milestone | Status | When complete |
|----|-----------|--------|----------------|
| M1 | Immediate output / no pipe backlog | `not_started` | P1 done; B1/B1b parity with HEAD (user 2026-05-26) |
| M2 | `npm` / `git` / PowerShell with ConPTY | `not_started` | P1 + P3 done |
| M3 | Colored build/diff output | `not_started` | P4 done |
| M4 | Partial interactive CLI (arrows, etc.) | `not_started` | P5 done |

---

## Session log

Newest first. One line per session that touched implementation.

```
2026-05-27 | P4.6 | done | docs | Added RichEdit/grid limitation notes and current VT capability boundaries
2026-05-27 | P4.5 | note | user | Verified good after deploy
2026-05-27 | P4.5 | done | build | Refactored hot path to read→queue→process (`processOutputQueue`), behavior preserved
2026-05-27 | P4.4 | note | user | Verified: internal `Process started/finished` lines stay visible; VT erase/backspace now affects child-output region only
2026-05-27 | P4.4 | done | build | `escProcess` finalized as real mode; `npe_console` help/comments updated to e0/e1/e2
2026-05-27 | P4 note | user | `sfk.exe`: `u+` (ConPTY) shows non-terminal fallback text; `u-` works as expected; documented as compatibility caveat (no app-specific code)
2026-05-27 | P4.3 | done | build | VT erase/backspace actions mapped to locked output region; current input region preserved
2026-05-27 | P4.2 | done | build | Basic VT actions wired in parser: SGR fg color, EL/ED clear, cursor-left/backspace mapping
2026-05-27 | P4.1 | done | build | Added `CConsoleVtParser` + `escProcess` path with stream state (no app-specific logic)
2026-05-27 | P3.6 | done | docs audit | ConPTY encoding alignment confirmed (global UTF-8 for input+output via getPseudoConsoleEncoding)
2026-05-27 | P3.5 | done | docs audit | `CreatePseudoConsole` flags reviewed: keep global `flags=0`; no per-app flag policy
2026-05-27 | P3.3 | done | build | Active ConPTY resize wired from Console WM_SIZE and font change
2026-05-27 | P3.3 | note | user | `cmd`/`u+`: `cls` does not clear RichEdit; tracked as R10 for P4
2026-05-27 | P3.2 | note | user | `u+` input duplication repro (`gawk getline`, `python -i`); track as R09, defer to P5
2026-05-27 | P3.2 | done | build | ConPTY size from console RichEdit client rect + font metrics
2026-05-27 | P3.1 | done | user cmd/python | ConPTY startup restored to HEAD logic; default `u+` works
2026-05-27 | P3.1 | done | build | PseudoConsole default enabled (`u+`); opt-out via `npe_console u-` unchanged
2026-05-27 | P2.4 | done | docs audit | WriteInput/WriteChildProcessInput* public boundary unchanged
2026-05-27 | P2.3 | done | user V2–V4 | ExecState GetRunningChildProcess/IsChildProcessRunning use IsRunning(); all passed
2026-05-27 | P2.2 | done | user V2–V4 | Do() Start+WaitForExit; all gates pass
2026-05-27 | P2.2 | done | build | Do() Start+WaitForExit; poll loop not via Create() at call site
2026-05-27 | P2.1 | done | user V2–V4 | Start/WaitForExit/IsRunning/Stop; Create wrapper; all gates pass
2026-05-27 | P2.1 | impl | build+deploy | Start/WaitForExit/IsRunning/Stop API added; Create wraps Start+WaitForExit
2026-05-26 | P1 done | user | V5 B1 + V6 B1b | same as HEAD; P1.8 stderr merge + readPipesDirect; python -u -i OK
2026-05-25 | P1.5 | done | build | adaptive process wait; drain queue per chunk; V3/V4 user verify
2026-05-25 | B1c gawk getline | user | PID 35260 | print 123 hidden until stdin; then aaa+123+finished — buffered stdout, not read bug
2026-05-25 | Plan+skill | docs | §2.8 invariants, V3/V4 gate | mandatory gawk break/exit tests; no done on build-only
2026-05-25 | P1.4 regression | fix | gawk/post-exit hang | close parent stdout-write/stdin-read after spawn; GetExitCodeProcess in wait loop; reader exit on child end
2026-05-25 | P1.7 | done | build | flush policy audit; no behavior change vs P1.6
2026-05-25 | P1.6 | done | user V2–V4 | stdin writer queue verified
2026-05-25 | P1.6 | impl | build+deploy | stdin writer thread; WriteInput enqueues; Flush on writer only
2026-05-25 | V4 | user | `gawk -e "BEGIN { print 1 }"` — output + clean exit
2026-05-25 | V3+V2 | user | cmd prompt OK; gawk no-args no deadlock
2026-05-25 | cmd prompt | fix | build+deploy | reader queue: flush partial line (no trailing newline on prompt)
2026-05-25 | P1.4 regression | fix | gawk hang | drain before kill blocked script thread; drain only if process exited
2026-05-25 | P1.4 | done | build+deploy | drainStdoutToQueue on exit; smoke `cmd /c echo hello`
2026-05-25 | B1b post-P1.3 | user | PID 41448 | R02b loop + R08b: no change; burst at exit only
2026-05-25 | P1.3 | done | build+deploy | blocking ReadFile on reader; B1b confirms not peek-limited
2026-05-25 | P1.1+P1.2 | done | build OK; deploy blocked if N++ holds DLL | smoke: restart N++, `cmd /c echo hello`; R08b unchanged until P1.3
2026-05-25 | B1b R08b | done | user Console | 30s stall; start+end at exit (buffered)
2026-05-25 | B1 manual | done | user Console log | R01-R02-R08 pass; R08 start before sleep
2026-05-25 | P0.3 | done | build+deploy+harness; baseline-P0.3.md | B1-B3 harness; B4-B6 .npe scripts
2026-05-25 | P0.2 | done | docs: CChildProcess-repro-matrix-P0.2.md | repro matrix, no code
2026-05-25 | P0.1 | done | docs: CChildProcess-callers-P0.1.md | inventory, no code
```

---

## Changelog of this tracker

| Date | Change |
|------|--------|
| 2026-05-27 | P4 complete: P4.6 documented RichEdit/grid limitations and current VT scope. |
| 2026-05-27 | P4.5 done: read/decode path thinned to queue-based processing (`readOnePipeDirect` + `processOutputQueue`). |
| 2026-05-27 | P4.4 done: `escProcess` finalized; updated `npe_console` wording/docs from e0/e1 to e0/e1/e2. |
| 2026-05-27 | Added compatibility note: `sfk.exe` differs between `u+` and `u-`; documented as app-side ConPTY caveat. |
| 2026-05-27 | P4.3 done: VT actions operate in output region only; input area preserved. |
| 2026-05-27 | P4.2 done: basic VT behavior added (SGR fg, EL/ED, cursor-left); built successfully. |
| 2026-05-27 | P4.1 done: added stateful `CConsoleVtParser` and wired `escProcess` to parser stream state. |
| 2026-05-27 | P3.6 done: ConPTY encoding already aligned (UTF-8 input/output); no auto-detect needed now. |
| 2026-05-27 | P3.5 done: global `CreatePseudoConsole` flags reviewed; keep `flags=0`. |
| 2026-05-27 | Plan §1.1: generic console only; no per-app code; P3.5 reframed. |
| 2026-05-27 | P2.3 done: `GetRunningChildProcess` / `IsChildProcessRunning` use `IsRunning()`. |
| 2026-05-27 | P2.4 done: `WriteInput` / `WriteChildProcessInput*` public boundary unchanged (audit). |
| 2026-05-27 | R09 added: ConPTY input echo duplication tracked for Phase 5 policy. |
| 2026-05-27 | D6: generic-only policy — no per-app flags/heuristics; **allowed cmd exception** for Ctrl-Break/Ctrl-C in kill list (plan §1.1.1). |
| 2026-05-27 | R10 added: `cmd cls` clear behavior tracked for Phase 4 VT/screen-buffer work. |
| 2026-05-27 | P3.3 done: active ConPTY resized on Console size/font changes. |
| 2026-05-27 | P3.1 verified: ConPTY matches HEAD startup; `cmd`/`python` OK with default `u+`. |
| 2026-05-27 | D3 decided yes; P3.1 done: PseudoConsole is default (`u+`). |
| 2026-05-27 | P2.2 done: `Do` uses `Start` + `WaitForExit`; V2–V4 verified by user. |
| 2026-05-27 | P2.1 done: lifecycle API; V2–V4 verified by user. |
| 2026-05-26 | Phase P1 marked `done`; P1.8 notes updated; V5/V6 B1+B1b user parity with HEAD. |
| 2026-05-25 | P1.7 done: FlushFileBuffers policy documented; default remains false. |
| 2026-05-25 | P1.6 done: queued stdin writes (writer thread). |
| 2026-05-25 | P1.5 done: decoupled process wait from read latency. |
| 2026-05-25 | Added §2.8–2.9 invariants + Phase 1 V3/V4 gate; skill deadlock checklist. |
| 2026-05-25 | P1.4 done: exit drain via drainStdoutToQueue; final read includes kill path. |
| 2026-05-25 | P1.3 done: Peek removed from reader thread; CancelSynchronousIo on stop. |
| 2026-05-25 | Deploy path corrected: `plugins\NppExec\NppExec.dll` (see dev-environment). |
| 2026-05-25 | P1.1+P1.2 done (build); CChildProcessIo + stdout reader thread. |
| 2026-05-25 | P0.3 done; dev-environment, harness, baseline results. Phase P0 complete. |
| 2026-05-25 | P0.2 done; repro matrix appendix added. |
| 2026-05-25 | P0.1 done; appendix added. |
| 2026-05-25 | Created tracker; all steps `not_started`. |
