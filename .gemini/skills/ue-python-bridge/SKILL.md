---
name: ue-python-bridge
description: Rapidly execute Python scripts and commands within the currently running Unreal Editor process.
---

# Unreal Engine Python Bridge âš?

This skill provides a high-speed communication link to the Unreal Editor's Python environment. It avoids the long startup times of `UnrealEditor-Cmd` by sending commands directly to the active process.

## AUTOMATION RULE: USE FOR ALL UE PYTHON
**CRITICAL**: All other skills in this project that require Unreal Python (e.g., `ue-material-generator`) **MUST** use this bridge to execute their commands. Never attempt to run `python` standalone for `unreal` module tasks.

## Usage
The bridge is a Python client that uses Unreal's Remote Execution protocol.

```powershell
# Execute a single command
python .gemini/skills/ue-python-bridge/scripts/ue_bridge.py "import unreal; print('Hello from Gemini!')"

# Execute a complex script from another skill
python .gemini/skills/ue-python-bridge/scripts/ue_bridge.py "import material_gen; mgr = material_gen.MaterialManager(); mgr.create_base_material('M_RapidTest', {})"
```

## Requirements
1.  **Unreal Editor must be running**.
2.  **Enable Remote Execution**:
    - Open **Project Settings**.
    - Go to **Plugins > Python Script Plugin**.
    - Check **Enable Remote Execution**.
    - (Default port is 30010).

## Expected Outcomes
- **Success**: The command is executed instantly, and any console output from Unreal is reflected in the Gemini CLI.
- **Node Not Found**: Ensure the editor is open and the setting above is enabled.
