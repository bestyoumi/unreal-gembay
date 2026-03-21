---
name: ue-material-generator
description: Imagine and generate Unreal Engine PBR materials via Remote Control. (Requires Unreal Editor to be running).
---

# Unreal Engine Material Generator 🎨

This skill positions Gemini as the **"Material Brain"**. You analyze natural language descriptions and imagine the perfect set of PBR parameters to achieve that look in Unreal Engine.

## The "Material Brain" Workflow
When a user describes a material:
1.  **Analyze**: Breakdown the visual characteristics (color, surface finish, transparency, glow).
2.  **Imagine**: Map these to full PBR parameters:
    - `BaseColor`: [R, G, B]
    - `Metallic`: 0.0 - 1.0
    - `Specular`: 0.0 - 1.0
    - `Roughness`: 0.0 - 1.0
    - `EmissiveColor`: [R, G, B]
    - `Opacity`: 0.0 - 1.0 (requires Translucent blend mode)
3.  **Execute**: Call the Remote Control script with your imagined parameters.

## Execution Rule
**CRITICAL**: You MUST use the `remote_gen.ps1` script. Provide a clear name (starting with `M_`) and a JSON string of parameters.

### Example: Imagine a "Glowing Sci-Fi Metal"
```powershell
powershell -ExecutionPolicy Bypass -File .gemini/skills/ue-material-generator/scripts/remote_gen.ps1 -Name "M_SciFiMetal" -JsonParams '{\"BaseColor\": [0.1, 0.1, 0.15], \"Metallic\": 1.0, \"Roughness\": 0.2, \"EmissiveColor\": [0, 0.5, 1.0]}'
```

## Internal Execution
The generator calls `UGemBaySubsystem::GenerateMaterial` via the Remote Control API (Port 30010), which executes `Content/Python/material_gen.py` inside the active editor.
