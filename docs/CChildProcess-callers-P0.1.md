# P0.1 — `CChildProcess` / `WriteInput` caller inventory

Appendix for [CChildProcess-console-improvement-plan.md](CChildProcess-console-improvement-plan.md).  
Generated for step **P0.1** (read-only analysis; no functional code changes).

**Implementation files:** `NppExec/src/ChildProcess.cpp`, `NppExec/src/ChildProcess.h`  
**Related:** `CProcessKiller` in the same translation unit (used via `CChildProcess::Kill` / `DoProcSignal`).

---

## 1. Public API (`CChildProcess`)

| Method | Role |
|--------|------|
| `CChildProcess(CScriptEngine*)` | Construct; holds `CNppExec*`, `CScriptEngine*` |
| `~CChildProcess()` | Destroy |
| `Create(HWND, LPCTSTR)` | Legacy: `Start` + `WaitForExit(INFINITE)` |
| `Start` / `WaitForExit` | Spawn (`CreateProcess`) then poll/read/kill/drain loop until exit |
| `Kill(...)` | Terminate via `CProcessKiller` |
| `MustBreak(unsigned)` | Sets break flag; read loop checks `isBreaking()` |
| **`WriteInput(const TCHAR*, bool bFFlush)`** | **Stdin:** encode → queue → writer thread `WriteFile`; `FlushFileBuffers` only if `bFFlush` (default false, §4.1) |
| `GetOutput()` | Accumulated stdout capture (`OPTB_CONSOLE_SETOUTPUTVAR`) |
| `GetExitCode()` / `GetProcessId()` / `GetProcessInfo()` | After run |
| `IsPseudoCon()` / `GetNewLine()` / `GetEncoding()` / `GetAnsiEscSeq()` | Console mode for UI/scripts |
| `RemoveAnsiEscSequencesFromLine` (static) | Used from `readPipesAndOutput` when `escRemove` |

**Protected / internal (change impact for later phases):** `readPipesAndOutput`, `applyOutputFilters`, `applyReplaceFilters`, `closePipes`, `closePseudoConsole`, `reset`, `applyCommandLinePolicy`.

---

## 2. Instance lifecycle

| Location | What |
|----------|------|
| `NppExecScriptEngine.cpp` — `CScriptEngine::Do()` | `std::shared_ptr<CChildProcess> proc(new CChildProcess(this))`; `m_execState.pChildProcess = proc`; `proc->Start(...)` + `proc->WaitForExit(INFINITE)`; then `proc->GetOutput()` / `GetExitCode()`; `m_execState.pChildProcess.reset()` |
| `NppExecScriptEngine.h` — `ExecState` | `pChildProcess`; `GetRunningChildProcess()` / `IsChildProcessRunning()` use `pChildProcess->IsRunning()` |

**Only `Do()` (unknown/external command) allocates `CChildProcess` today.**

---

## 3. `GetRunningChildProcess()` chain

Resolves child from **running script engine**, walking **parent** script engines:

| Location | Usage |
|----------|--------|
| `CScriptEngine::GetRunningChildProcess()` | Walks `GetCommandExecutor().GetRunningScriptEngine()` → parent chain → `ExecState::GetRunningChildProcess()` (non-null only while `IsRunning()`) |
| `CNppExecCommandExecutor::GetRunningChildProcess()` (const/non-const) | Delegates to `m_RunningScriptEngine->GetRunningChildProcess()` |
| `IScriptEngine` (`NppExecCommandExecutor.h`) | Virtual `GetRunningChildProcess()` on interface |

**Callers of `GetRunningChildProcess()` / indirect child access:**

| File | Purpose |
|------|---------|
| `NppExecCommandExecutor.cpp` | `WriteChildProcessInput*`, `ChildProcessMustBreak`, `GetChildProcessNewLine/Encoding/AnsiEscSeq`, `IsChildProcessPseudoCon`, `ShareLocalVars` PID restore |
| `NppExecScriptEngine.cpp` | `DoProcSignal` → `Kill`; `ChildProcessMustBreakAll` → `MustBreak` |
| `NppExecCommandExecutor.cpp` | `OnShareLocalVars` / `rfShareLocalVars` — restore `$(PID)` |

---

## 4. Stdin: `WriteInput` call graph

All stdin goes through **`CNppExecCommandExecutor::WriteChildProcessInput`** → **`pChildProc->WriteInput`** (unless no running child → `false`).

| Caller | File (approx.) | Trigger |
|--------|----------------|---------|
| `WriteChildProcessInput` / `WriteChildProcessInputNewLine` | `NppExecCommandExecutor.cpp` | Facade to `WriteInput` / `WriteInput(GetNewLine())` |
| `ExecuteChildProcessCommand` | `NppExecCommandExecutor.cpp` | Console Enter: non-prefixed cmd → `WriteChildProcessInput` + `WriteChildProcessInputNewLine`; `@exit_cmd` paths |
| `ScriptEngineRunner` (after collateral script) | `NppExecCommandExecutor.cpp` ~57 | Prompt newline when parent output enabled |
| `ExpiredCommand` / shutdown helpers | `NppExecCommandExecutor.cpp` ~1229, ~1419 | `WriteChildProcessInputNewLine` when child running |
| **`DoProcInput`** | `NppExecScriptEngine.cpp` ~7663 | Script `proc_input` → `WriteChildProcessInput` (+ newline if needed) |
| **`CScriptEngine::ExecText`** (console text) | `NppExecScriptEngine.cpp` ~6299 | Selected/console text to running child |
| **`CNppExec::execConsoleText`** (name approximate) | `NppExec.cpp` ~3699–3710 | N++ “send to console” while child running |
| **ConsoleDlg** — Enter | `DlgConsole.cpp` ~4286 | `ExecuteChildProcessCommand(cmd, true, true)` |
| **ConsoleDlg** — Ctrl+Z | `DlgConsole.cpp` ~3857–3858 | `WriteChildProcessInput("\x1A")` + newline |
| **ConsoleDlg** — paste/newline helpers | `DlgConsole.cpp` ~4255, ~4290, ~5495 | `GetChildProcessNewLine()` for EOL substitution |
| Collateral cmd builder | `NppExecScriptEngine.cpp` ~2772–2798 | Wraps buffered lines as `proc_input ...` for `nppexec:` prefix lines |

**Not active (commented):** `DlgConsole.cpp` ~3876 `WriteChildProcessInput("\x04")` (^D) via `Runtime::GetNppExec().WriteChildProcessInput` — block inside `/* ... */`.

### 4.1 P1.7 — `FlushFileBuffers` / `bFFlush` audit (2026-05-25)

| Item | Result |
|------|--------|
| API | `WriteInput(szLine, bFFlush=false)` / `WriteChildProcessInput(..., bFFlush=false)` |
| In-tree callers passing `bFFlush=true` | **None** (all use default) |
| Runtime behavior | `WriteFile` always; `FlushFileBuffers` only if `bFFlush==true` (on stdin writer thread after P1.6) |
| Hang risk | Documented in `ChildProcess.h` — flush waits until child reads stdin when pipe buffer is full |
| Child-side buffering | Unchanged — use `python -u`, `fflush()`, etc. (manual §4.8.2–4.8.3) |

**Policy:** Do not add default `bFFlush=true` on Console Enter or `proc_input`; callers that need flush must pass `true` explicitly and accept blocking on the writer thread.

---

## 5. Stdout / control (output and signals)

| API | Called from | Effect |
|-----|-------------|--------|
| **`readPipesAndOutput`** | `CChildProcess::WaitForExit()` loop + final drain | `GetConsole().PrintOutput`, `ProcessSlashR/B`, `m_strOutput`, filters |
| `MustBreak` | `CNppExecCommandExecutor::ChildProcessMustBreak`; `ChildProcessMustBreakAll`; user break paths | Sets `m_nBreakMethod` |
| `Kill` | `CChildProcess::WaitForExit` (script abort); **`DoProcSignal`** | `CProcessKiller` |
| `GetOutput` | `CScriptEngine::Do()` | `$(OUTPUT)` macros when option enabled |
| `GetExitCode` | `CScriptEngine::Do()` | `$(EXITCODE)` |
| `GetProcessId` | `Start()` (macro), executor share-vars | `$(PID)` |

**Console output surface (not direct `CChildProcess`, but fed only by `readPipesAndOutput` today):**

- `CNppExecConsole::PrintOutput` / `ProcessSlashR` / `ProcessSlashB` (`NppExec.cpp`)
- Postponed via `postponeThisCall` for collateral scripts

---

## 6. `IsChildProcessRunning()` (no `WriteInput`, but gates input)

| File | Usage |
|------|--------|
| `CNppExecCommandExecutor::IsChildProcessRunning` | Delegates to running `CScriptEngine` |
| `CScriptEngine::IsChildProcessRunning` | Walks parent engines |
| `DlgConsole.cpp` | Enter routing, Ctrl+C/Break, focus, help availability |
| `NppExec.cpp` | `execConsoleText` branch (~3637) |
| `NppExecScriptEngine.cpp` | `DoProcInput` / `DoProcSignal`; `$(IS_PROCESS)` (~8937); queue guards (~10054) |
| `NppExecPluginInterface.cpp` | ~579 — async/plugin vs child |
| `NppExecCommandExecutor.cpp` | `CanStartScriptOrCommand`, expired cmd, exit dialog |

---

## 7. Child metadata (encoding / ANSI / PseudoCon)

Used while child is running (or option fallback if no child):

| API | Primary callers |
|-----|-----------------|
| `GetChildProcessNewLine` | `DlgConsole.cpp` (input EOL), executor input |
| `GetChildProcessEncoding` | `NppExecScriptEngine.cpp` — `npe_console` display (~5621) |
| `GetChildProcessAnsiEscSeq` | `npe_console` (~5576); options init in `NppExec.cpp` (~4591) |
| `IsChildProcessPseudoCon` | `npe_console` (~5581) |
| `GetAnsiEscSeq` / `GetEncoding` / `GetNewLine` / `IsPseudoCon` | Inside `ChildProcess.cpp`; forwarded by executor |

**Options touched:** `OPTB_CHILDP_PSEUDOCONSOLE`, `OPTU_CONSOLE_ENCODING`, `OPTI_CONSOLE_ANSIESCSEQ`, `savedConf` in `npe_console` (`NppExecScriptEngine.cpp` ~5147+).

---

## 8. Exit / break UX

| Feature | Entry |
|---------|--------|
| `@exit_cmd` / `$(EXIT_CMD)` | `CNppExecCommandExecutor::SendChildProcessExitCommand` → `ExecuteChildProcessCommand` |
| Exit dialog | `ShowChildProcessExitDialog` → `ExecuteChildProcessCommand` or `ChildProcessMustBreak` |
| Console Ctrl+C / Break | `DlgConsole.cpp` → `ChildProcessMustBreak` |
| `proc_signal` | `DoProcSignal` → `pChildProc->Kill(...)` |
| Npp shutdown | `CChildProcess::WaitForExit` — empty read timeout → `MustBreak` |

---

## 9. Documentation / help only (no runtime call)

- `docs/NppExec_Manual/4.8.3.html`, `topics.js`
- `docs/NppExec.txt`, `NppExec_HelpAll.txt`
- `DlgConsole.cpp` help strings; `NppExec.cpp` / `NppExec.h` changelog comments

---

## 10. Summary diagram

```
CScriptEngine::Do()
  └─ CChildProcess::Start()
  └─ CChildProcess::WaitForExit(INFINITE)  [blocks]
        └─ readPipesAndOutput() → CNppExecConsole::PrintOutput / ProcessSlashR|B
        └─ (on abort) Kill / MustBreak

User / script input:
  ConsoleDlg Enter ──► ExecuteChildProcessCommand ──► WriteChildProcessInput ──► WriteInput
  proc_input       ──► DoProcInput              ──► WriteChildProcessInput ──► WriteInput
  ExecText/NppExec ──► WriteChildProcessInput*  ──► WriteInput
  @exit_cmd         ──► ExecuteChildProcessCommand ──► WriteInput*

proc_signal ──► GetRunningChildProcess() ──► Kill()
Ctrl+C/Break  ──► ChildProcessMustBreak() ──► MustBreak()
```

---

## 11. Implications for planned phases

| Phase | Touch these caller groups |
|-------|---------------------------|
| P1 I/O | `Create` / `readPipesAndOutput`; queue `WriteInput`; all `WriteChildProcessInput*` callers unchanged at API |
| P2 Lifecycle | `CScriptEngine::Do`, `pChildProcess` lifetime, `IsChildProcessRunning` |
| P3 PseudoCon | `Create`, `GetNewLine`/`GetEncoding`/`GetAnsiEscSeq`, `npe_console`, options |
| P4 VT | `readPipesAndOutput`, `PrintOutput`, `RemoveAnsiEscSequencesFromLine`, `escProcess` |
| P5 Input | `ExecuteChildProcessCommand`, `DlgConsole` `RichEditWndProc`, `DoProcInput`, raw keys → `WriteInput` |

**Highest coupling for testing after P1:** `DlgConsole.cpp` Enter path, `DoProcInput`, unbuffered child output (manual P0.2 matrix).
