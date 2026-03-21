---
name: ue-toolchain
description: Optimized toolchain for Unreal Engine development. Handles tool discovery, building, running, live coding, and project generation.
---

# Unreal Engine Toolchain 🌟

This skill provides a suite of scripts to manage the full development lifecycle of an Unreal Engine project. It automatically discovers your local engine and IDE installations.

## AUTOMATION RULE: LIVE CODING AFTER C++ EDITS
**CRITICAL**: After modifying any `.h` or `.cpp` files in the `Source/` directory, you **MUST** automatically execute `live_coding.ps1`.

## Optimized Development Workflow

### 1. Discovery & Setup
Initializes and caches the paths for UE, VS, Rider, and VS Code.
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-toolchain/scripts/find_tools.ps1
```

### 2. Live Coding (Hot-Reload)
Use this for **90% of your C++ iterations**. It effectively triggers compilation and monitors the editor logs.
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-toolchain/scripts/live_coding.ps1
```

### 3. Generate Project Files
Use this after **modifying `.Build.cs`, `.Target.cs`, or adding/removing files**. It terminates the editor to avoid file locks.
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-toolchain/scripts/generate_vs_project.ps1
```

### 4. Full Build
Use this if **Live Coding fails**, after generating project files, or for a clean build. 
> **Warning**: It is recommended to close the Unreal Editor before running a full build.
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-toolchain/scripts/build.ps1
```

### 5. Run Project
Launches the project in the Unreal Editor.
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-toolchain/scripts/run.ps1
```

### 6. Clean Project Cache
Removes `Binaries`, `Intermediate`, `DerivedDataCache`, and IDE folders to reset the project state.
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-toolchain/scripts/clean_project.ps1
```

## Best Practices
- **Iteration**: Edit Code -> Run `live_coding.ps1` -> Test in Editor.
- **Config Changes**: Edit `.Build.cs` -> Run `generate_vs_project.ps1` -> Run `build.ps1` -> Run `run.ps1`.
- **Errors**: If `live_coding.ps1` fails with linker errors, try a full `build.ps1`.
- **Cleanup**: Run `clean_project.ps1` before committing to source control or when encountering unrecoverable editor/build glitches.
