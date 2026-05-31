# P0.2 — Console child-process repro matrix

Appendix for [CChildProcess-console-improvement-plan.md](CChildProcess-console-improvement-plan.md).  
Use with [CChildProcess-callers-P0.1.md](CChildProcess-callers-P0.1.md).  
**P0.3** records baseline metrics in the **Results** column below.

---

## 1. Environment (record once per baseline run)

See **[NppExec-dev-environment.md](NppExec-dev-environment.md)** for canonical paths.

| Field | Value (fill when running P0.3) |
|-------|--------------------------------|
| Date | |
| NppExec build | `NppExec_build_vc2017_x64.bat` → `NppExec\x64\Release\NppExec.dll` |
| Deploy | `C:\Progs\Progs\Notepadpp-x64\plugins\NppExec\NppExec.dll` |
| Notepad++ x64 | `C:\Progs\Progs\Notepadpp-x64\notepad++.exe` |
| Windows version | |
| Python on PATH (for R02) | `python --version` |
| Git on PATH (for R04) | `git --version` |
| INI: `ChildProcess_PseudoConsole` default | |
| INI: `Console_AnsiEscSeq` (or `e0`/`e1`) | |
| INI: `ChildProcess_CycleTimeout_ms` (default 120) | |

**Plugin load:** copy DLL to deploy path above; restart N++.

**Console visibility:** run tests with NppExec Console open (`NPP_CONSOLE 1` in script or visible Console dialog).

---

## 2. Configuration presets

Apply **before** the child command in the same script (or set in Advanced Options / `NppExec.ini` and note INI used).

| Preset ID | `npe_console` / options | Purpose |
|-----------|-------------------------|---------|
| **CFG-A** | `u-` (pipes only) | Classic anonymous pipes, no ConPTY |
| **CFG-B** | `u+` (PseudoConsole) | ConPTY path (`CreatePseudoConsole`) |
| **CFG-C** | `e0` (ANSI raw) | Keep escape sequences in output (may show garbage in RichEdit) |
| **CFG-D** | `e1` (ANSI remove) | Default strip (`escRemove`) |
| **CFG-E** | `u+` + `e1` | PseudoConsole + strip (current forced combo when `u+` and raw) |
| **CFG-F** | `u+` interactive input | Detect local-input + child-echo duplication behavior |

Script prefix example:

```nppexec
npp_console 1
npe_console u- e1
// child command on next line
```

Toggle PseudoConsole for **running** child only: `npe_console u+` / `u-` (see help in Console).

---

## 3. Repro matrix (core cases)

| ID | Scenario | What it stresses | Primary plan phase |
|----|----------|------------------|-------------------|
| **R01** | Line-buffered stdout | `PeekNamedPipe` / cycle timeout, line split in `readPipesAndOutput` | P1 |
| **R02** | Python **`-u`** (unbuffered) streaming | Best-case pipe read; not typical for most EXEs | P1 (regression) |
| **R02b** | **Buffered** stdout (default Python, many CLIs) | Output held until buffer full or process exit — **primary P1 target** | P1 |
| **R03** | `\r` overwrite | `ProcessSlashR`, partial lines | P1, P4 |
| **R04** | ANSI colors | `RemoveAnsiEscSequencesFromLine`, encoding | P3, P4 |
| **R05** | Large burst | `CONSOLEPIPE_BUFSIZE` (8000), multi-chunk read | P1 |
| **R06** | Interactive stdin | `WriteInput` + Enter, prompt return | P1, P5 |
| **R07** | `proc_input` | Script path to `WriteChildProcessInput` | P5 (regression) |
| **R08** | Long-running + **`-u`/`flush`** while alive | May pass even when pipe read is weak | P1 (regression) |
| **R08b** | Long-running **buffered** (no `-u`) | [4.8.3](NppExec_Manual/4.8.3.html) §9 class — **primary stall repro** | P1 |
| **R09** | `u+` interactive input echo | Line appears twice after Enter in apps that echo terminal input (`gawk getline`, `python -i`) | P5 |
| **R10** | `u+` + `cmd` + `cls` | Console clear not reflected in RichEdit without VT/screen-buffer emulation | P4 |

---

## 4. Test procedures

### R01 — Line-buffered (`cmd /c echo`)

**Script:**

```nppexec
npp_console 1
npe_console u- e1
cmd /c echo hello
```

Repeat with **CFG-B** (`u+`).

| Check | Pass (baseline expectation) |
|-------|------------------------------|
| Console shows `hello` before `Process finished` | Yes |
| Line appears within ~1 s (no long hang with child already exited) | Usually yes |
| Internal `Process started` / `Process finished` messages | If `m+/m-` default shows them |

**Failure signature:** no `hello` until process exits or never — suspect pipe peek/read (R08 overlap).

**Results (P0.3):** CFG-A: ___ | CFG-B: ___

---

### R02 — Python unbuffered (`python -u`) — optimistic case

**Requires:** `python` on PATH.

`-u` forces unbuffered stdout. Many programs have **no equivalent flag**; they behave like Python **without** `-u` (see **R02b**). A pass here does **not** prove the console is fine for typical tools.

**Script:**

```nppexec
npp_console 1
npe_console u- e1
python -u -c "import time; [print(i, flush=True) or time.sleep(0.3) for i in range(5)]"
```

Repeat **CFG-B**.

| Check | Pass (baseline expectation) |
|-------|------------------------------|
| Digits `0`..`4` appear incrementally (~0.3 s apart) | Often **passes** with `-u` |
| All lines visible before exit | Yes |

**Results (P0.3):** CFG-A delay ___ s max | CFG-B: ___

---

### R02b — Buffered stdout (typical program behavior)

**Requires:** `python` on PATH.

Same logic as R02 but **omit `-u`** (and no `flush=True`). Stdout is block-buffered when connected to a pipe; output commonly appears only when the buffer fills or the process exits.

**Script:**

```nppexec
npp_console 1
npe_console u- e1
python -c "import time\nfor i in range(5):\n print(i)\n time.sleep(0.3)"
```

| Check | Baseline (expected problem) |
|-------|---------------------------|
| Any digit visible **before** process exit | **Often no** |
| All lines `0`..`4` appear at exit | Yes |

**Results (P0.3):** CFG-A: ___ | CFG-B: ___

---

### R03 — Carriage return overwrite

**Script (PowerShell):**

```nppexec
npp_console 1
npe_console u- e1
powershell -NoProfile -Command "1..5 | %% { Write-Host ('Progress: ' + $_ + ' / 5') -NoNewline; Start-Sleep -Milliseconds 400; Write-Host ('`rProgress: ' + $_ + ' / 5   ') }"
```

**Simpler CMD-friendly variant:**

```nppexec
npp_console 1
npe_console u- e1
cmd /c "for /L %%i in (1,1,5) do @(set /P=Step %%i / 5 <nul & timeout /t 1 /nobreak >nul)"
```

| Check | Pass (baseline expectation) |
|-------|------------------------------|
| Single logical line updates OR multiple `Step` lines | RichEdit `\r` handling via `ProcessSlashR` |
| No garbled infinite scroll | |

**Results (P0.3):** CFG-A: ___ | CFG-B: ___

---

### R04 — ANSI colors

**Option A — git (repo with changes):**

```nppexec
npp_console 1
npe_console u- e1
cd $(CURRENT_DIRECTORY)
git diff --color=always HEAD~1 --no-pager 2>nul || git log -1 --oneline
```

**Option B — PowerShell:**

```nppexec
npp_console 1
npe_console u- e1
powershell -NoProfile -Command "Write-Host 'Error' -ForegroundColor Red; Write-Host 'OK' -ForegroundColor Green"
```

Run subsets: **CFG-A + e1**, **CFG-A + e0**, **CFG-B + e1**.

| Check | `e1` | `e0` |
|-------|------|------|
| Escape codes stripped / no ``[91m`` visible | Expected | N/A |
| Red/green visible as colors | **No** (baseline) | Raw escapes may show |
| Readable text content | Yes | Yes |

**Results (P0.3):** e0/e1/u+/u- notes: ___

---

### R05 — Large burst (>8 KB)

**Script:**

```nppexec
npp_console 1
npe_console u- e1
python -u -c "print('X'*9000); print('END')"
```

If Python missing:

```nppexec
npp_console 1
npe_console u- e1
powershell -NoProfile -Command "'X'*9000; 'END'"
```

| Check | Pass (baseline expectation) |
|-------|------------------------------|
| ~9000 `X` chars (or wrapped lines) appear | Yes, possibly after chunking |
| Trailing `END` line visible | Yes |
| Console responsive during print | May lag |

**Results (P0.3):** time to first char ___ | time to `END` ___

---

### R06 — Interactive stdin (cooked)

**Script (start interactive cmd):**

```nppexec
npp_console 1
npe_console u- e1
cmd /k
```

**Manual steps (while child running, `$(IS_PROCESS)` = 1):**

1. Type `echo from_keyboard` in Console input line; press **Enter**.
2. Expect line echoed by cmd and new prompt.

| Check | Pass |
|-------|------|
| Input appears in child output | Yes |
| `WriteInput` + newline (`ExecuteChildProcessCommand`) | Uses `\r\n` or configured Enter; PseudoCon uses `\r` |

Repeat with **CFG-B** (`npe_console u+` before `cmd /k`).

**Results (P0.3):** CFG-A: ___ | CFG-B: ___

---

### R07 — `proc_input`

**Script:**

```nppexec
npp_console 1
npe_console u- e1
cmd /k
```

While running, execute in Console (or second script via `npe_queue`):

```nppexec
proc_input echo via_proc_input
```

| Check | Pass |
|-------|------|
| `echo via_proc_input` in output | Yes |
| Fails with “child console process is not running” when idle | Yes |

**Results (P0.3):** ___

---

### R08 — Long run with `python -u` + `flush` — optimistic case

**Script:**

```nppexec
npp_console 1
npe_console u- e1
python -u -c "import time; print('start', flush=True); time.sleep(30); print('end', flush=True)"
```

**During the 30 s sleep** (child alive):

| Check | Baseline |
|-------|----------|
| `start` visible promptly | **Often yes** (flush + `-u`) — B1 user run passed |
| `end` after 30 s | Yes |

Does **not** model programs that buffer until exit. Use **R08b** for that.

**Results (P0.3):** delay `start` vs child start ___ s

---

### R08b — Long run **buffered** (primary stall repro)

**Script:**

```nppexec
npp_console 1
npe_console u- e1
python -c "import time; print('start'); time.sleep(30); print('end')"
```

No `-u`, no `flush`. Matches “normal” Python and many other runtimes on a pipe.

**During the 30 s sleep:**

| Check | Baseline (expected problem) |
|-------|---------------------------|
| `start` visible while child alive | **Often no** — nothing until buffer flush or exit |
| `end` after 30 s | Appears with or after `start` at exit |

**Results (P0.3):** stall Y/N | when `start` appeared ___

---

## 5. Cross-matrix (minimum set for P0.3 baseline)

Run at least these **six** combinations once and record measurements in **P0.3** (progress file Notes or `CChildProcess-baseline-P0.3.md` when created):

| Run | Config | Cases |
|-----|--------|-------|
| B1 | CFG-A | R01, R02, R08 (`-u` — optimistic) |
| **B1b** | CFG-A | **R02b, R08b** (buffered — **run for real baseline**) |
| B2 | CFG-B | R01, R02, R08 |
| **B2b** | CFG-B | **R02b, R08b** |
| B3 | CFG-A | R05 |
| B4 | CFG-A | R03, R06 |
| B5 | CFG-A + e0 | R04 (PowerShell option B) |
| B6 | CFG-B | R04 (PowerShell option B) |

After **P1**, re-run **B1** and **B2** and compare to baseline.

---

## 6. Pass/fail criteria for later phases

| Phase | Regression gate |
|-------|-----------------|
| **P1** | **R02b, R08b**: output while child alive without exit; R02/R08 (`-u`) remain OK; R05: no truncation |
| **P3** | R04 under CFG-B: improved fidelity (colors or correct strip) |
| **P4** | R04: colors with `e1`→`process`; R03: stable `\r` |
| **P5** | R06, R07 unchanged; optional raw-key subcase added later |

---

## 7. P0.3 metrics (link)

For each run ID, P0.3 should record in progress file or a short `CChildProcess-baseline-P0.3.md`:

- **TTFB:** time from Enter on child command until first output character in Console.
- **Tail:** time from process exit to last byte shown (drain quality).
- **Stall:** Y/N for R08 during sleep window.
- **Subjective:** output “stuck in pipe” Y/N.

---

## 8. Changelog

| Date | Change |
|------|--------|
| 2026-05-25 | P0.2 initial matrix and procedures. |
