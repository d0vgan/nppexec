---
name: code-modification
description: >-
  Safe C/C++ edits for NppExec: analyze callers first, preserve style and behavior,
  enforce CChildProcess I/O invariants (no deadlocks), build with NppExec_build_vc2017_x64.bat,
  fix compile errors automatically. Use for features, refactors, bug fixes, or low-level changes in this repo.
disable-model-invocation: true
---

# Code Modification

## Console child-process plan

When implementing `docs/CChildProcess-console-improvement-plan.md`:

1. Read `docs/CChildProcess-console-improvement-progress.md` first.
2. Read **�2.8 Implementation invariants** and **�2.9 P1 regression lessons** in the plan before touching `ChildProcess*.cpp`.
3. Work only the step marked `in_progress`, or set the next `not_started` step to `in_progress` before coding.
4. On completion: set step `done` only after the plan�s **Phase 1 Verification gate** (if applicable); update progress commit/PR, verification, session log.

**Do not mark a Phase 1 step `done` with only `build` verification** � V3 (break while alive) and V4 (clean exit) are mandatory for any `CChildProcess` / `CChildProcessIo` change.

---

## CChildProcess / pipe I/O � deadlock prevention (required)

Refactors here blocked the **script thread** and **Notepad++ shutdown** (gawk regression, 2026-05-25). Treat the plan �2.8 contract as hard requirements.

### Before editing (trace, do not skip)

1. Read callers: `docs/CChildProcess-callers-P0.1.md`.
2. List every **blocking** call you add or move (`ReadFile`, `WaitForSingleObject(INFINITE)`, `drainStdoutToQueue`).
3. For each, write: **which thread** and **what unblocks it** when the child is alive / when the child has exited / on `MustBreak`.
4. If the script thread can block on stdout while `GetExitCodeProcess == STILL_ACTIVE` without legacy `readPipesDirect` semantics, **stop and redesign** (kill-first ordering; no `drainStdoutToQueue` while child alive).

### Non-negotiable invariants (summary)

| Rule | Detail |
|------|--------|
| **Close inherited pipe ends** | After `CreateProcess`, parent closes `m_hStdOutWritePipe` and `m_hStdInReadPipe` (`onChildProcessStarted`). |
| **Blocking stdout read (pipe mode)** | Legacy `readPipesDirect` on script thread; do not start stdout reader in `Create()`. |
| **PrintOutput** | Script thread only � never reader thread. |
| **Break / close console** | `stopStdoutReader` ? **kill** while child running � **never** `drainStdoutToQueue` first. |
| **Exit drain** | `readPipesAndOutput(true)` only if child **not** `STILL_ACTIVE`. |
| **stopStdoutReader** | `CancelSynchronousIo` + bounded `WaitForSingleObject` � never infinite join without cancel. |
| **Liveness** | Use `GetExitCodeProcess != STILL_ACTIVE`; do not use peek/empty queue as �still running�. |

Full text: `docs/CChildProcess-console-improvement-plan.md` �2.8�2.9.

### After editing (mandatory manual tests)

Deploy per `docs/NppExec-dev-environment.md`, **restart Notepad++** if copy says DLL in use.

| ID | Command / action | Must |
|----|------------------|------|
| V2 | `cmd /c echo hello` | Finish message |
| V3 | `gawk.exe` or `cmd /k` ? Ctrl+C or close Console | Child killed; N++ responsive |
| V4 | `gawk -e "print 1"` | `<<< Process finished` soon after exit |

Record V3/V4 in progress **Verified** column (`both` = build + V2�V4). Note B1b separately � buffered burst-at-exit is expected, not a deadlock regression.

### Red flags (reject the change)

- `drainStdoutToQueue()` or blocking `ReadFile` on stdout from `Create()`�s script thread before kill.
- Parent still holds child's stdout-write handle after start.
- `WaitForSingleObject(readerThread, INFINITE)` with no timeout/cancel path.
- �Build passes� as sole verification for pipe/thread changes.

---

## Before editing (general)

1. Read the target code and its callers; trace helpers and control paths.
2. Extend existing functions and helpers; avoid parallel duplicate logic.

## Change rules

- Match existing style, naming, and patterns.
- Preserve behavior and call paths unless the user requested otherwise.
- Reuse existing helpers; do not contradict established logic or add unintended side effects.
- **Never add process-specific crutches** (e.g. helpers that detect `python`, `cmd`, `>>> ` prompts, or similar). Fix pipe/console I/O generically; match legacy `CChildProcess` behavior instead of special-casing programs.
- **Stderr in pipe mode:** merge into the stdout pipe (legacy HEAD); do not use a separate stderr pipe or error text colour for routine child stderr.
- Fix undefined behavior, leaks, and out-of-range/bounds issues in code you touch.
- Keep edits minimal and efficient.
- For behavior-visible fixes or API changes: ask the user before proceeding.
- **ChildProcess refactors:** minimal diff per step; do not combine P1.x steps unless the user explicitly asks � each step needs its own V3/V4 pass.

## Build and deploy (64-bit)

Environment details: `docs/NppExec-dev-environment.md`.

After code changes, from the **repository root**:

```bat
NppExec_build_vc2017_x64.bat
copy /Y "NppExec\x64\Release\NppExec.dll" "C:\Progs\Progs\Notepadpp-x64\plugins\NppExec\NppExec.dll"
```

| Item | Path |
|------|------|
| Build script | `NppExec_build_vc2017_x64.bat` |
| Output DLL | `NppExec\x64\Release\NppExec.dll` |
| Notepad++ x64 | `C:\Progs\Progs\Notepadpp-x64\notepad++.exe` |
| Deploy target | `C:\Progs\Progs\Notepadpp-x64\plugins\NppExec\NppExec.dll` |

Restart Notepad++ before manual console tests. Requires VS 2017 with `vcvarsall.bat`.

## Compile errors (required)

**Fix compilation errors automatically** � do not stop at the first failed build and ask the user to fix them.

1. Run `NppExec_build_vc2017_x64.bat` and capture the **first** meaningful compiler/linker error.
2. **Fix that error** with the smallest correct edit; ignore cascading diagnostics until the primary error clears.
3. Rebuild and repeat until the build succeeds or the limit below is reached.
4. Assume the toolchain is right by default; before blaming the compiler, check: saved/touched files, includes, types/signatures, stale `.obj`/outputs, platform (x64), renamed APIs.
5. One hypothesis ? one edit ? rebuild. Roll back the edit if the same primary error persists.
6. **Hard limit:** after **3** failed fix-rebuild cycles on the **same** primary error, stop, summarize (error text, files changed, hypotheses), and ask for human review. Do not disable warnings or invent workarounds.

A successful step is not complete until the build passes **and** applicable manual gates (V2�V4 for ChildProcess) pass unless the user explicitly waived them.
