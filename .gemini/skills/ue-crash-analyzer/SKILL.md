---
name: ue-crash-analyzer
description: Scan and analyze the latest Unreal Engine crash logs to identify root causes and plan fixes.
---

# Unreal Engine Crash Analyzer ðŸ› ï¸?

This skill helps you diagnose stability issues by parsing the technical data stored in Unreal's `Saved/Crashes` directory.

## Capabilities
1.  **Auto-Scan**: Finds the most recent crash directories.
2.  **Context Extraction**: Parses `CrashContext.runtime-xml` for the specific Error Message and CallStack.
3.  **Log Review**: Tail-reads the associated engine log to find the events leading up to the crash.

## When to use
- Immediately after the Unreal Editor crashes.
- When you encounter "Access Violation" or "Assertion Failed" errors.
- To gather technical data for a bug report.

## Usage
Run the script to see a summary of the last 3 crashes:
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-crash-analyzer/scripts/analyze_crashes.ps1
```

## Workflow Rule
After running the analysis, the agent should:
1.  Identify the module/file causing the crash (e.g., `GemBay` or `Slate`).
2.  Propose a specific C++ or config fix based on the CallStack.
3.  Execute the fix and verify.
