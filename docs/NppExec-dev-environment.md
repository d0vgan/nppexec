# NppExec — development environment (64-bit)

Canonical paths for build, deploy, and manual/automated testing. **Use in all console-plan steps and agent sessions.**

Modern Notepad++ loads NppExec from a **plugin subfolder** (`plugins\NppExec\`), not directly under `plugins\`.

| Item | Path |
|------|------|
| **Build script** (repo root) | `NppExec_build_vc2017_x64.bat` |
| **Build output** | `.\NppExec\x64\Release\NppExec.dll` |
| **Notepad++ x64** | `C:\Progs\Progs\Notepadpp-x64\notepad++.exe` |
| **Plugin deploy target** | `C:\Progs\Progs\Notepadpp-x64\plugins\NppExec\NppExec.dll` |

## Deploy after build

From repository root:

```bat
copy /Y "NppExec\x64\Release\NppExec.dll" "C:\Progs\Progs\Notepadpp-x64\plugins\NppExec\NppExec.dll"
```

Restart Notepad++ (or reload plugins if supported) before manual console tests.

## Automated pipe baseline (no GUI)

`docs\tools\nppe_readloop_harness.py` — simulates `CChildProcess` peek/sleep/read loop (CFG-A pipes only).  
Full **PseudoConsole** (CFG-B) and RichEdit checks require N++ + deployed DLL.

## Baseline scripts (N++)

`docs\baseline\P0.3\*.npe` — open in NppExec or run via `npp_exec` after deploy.
