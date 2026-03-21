---
name: everything-search
description: High-performance file searching utility using the 'Everything' command-line interface (es.exe).
---

# Everything File Search 🔍

This skill utilizes the `es.exe` tool to perform incredibly fast file and folder searches across the system using the "Everything" search syntax. It is extremely useful when you need to rapidly locate files by name, extension, or path, especially in large workspaces.

## ⚠️ Prerequisites & Initialization (CRITICAL)

1. **The Everything Service**: `es.exe` depends on the background "Everything" service. If the user complains about connection errors, IPC errors, or missing results, **remind them to download and install Everything from https://www.voidtools.com/ and ensure the "Everything Service" is enabled in the application settings (Tools -> Options -> General -> Check "Everything Service")**.
2. **First-Time Setup (Tool Acquisition) & Upgrading**: Before running a search, you **MUST** verify that the executable exists at `.gemini/skills/everything-search/tools/es.exe`. If it does not exist, or if the user requests an upgrade, **do not write custom download logic**. Instead, simply execute the provided installation script without thinking twice:
   ```powershell
   pwsh -ExecutionPolicy Bypass -File .gemini/skills/everything-search/scripts/upgrade_es.ps1
   ```

## Usage

You can execute searches by running the bundled `es.exe` executable via PowerShell (`pwsh`). A security policy is configured to allow this command to run without confirmation.

```powershell
# Basic search
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe 'search text'"
```

## Common Search Examples

**1. Find specific file types (e.g., all `.exe` or `.ini` files):**
```powershell
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe ext:exe;ini"
```

**2. Find files in a specific directory:**
```powershell
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe -path 'C:\Path\To\Search' 'filename'"
```

**3. Search for folders only (`/ad`) or files only (`/a-d`):**
```powershell
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe /ad 'folder name'"
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe /a-d 'file name'"
```

**4. Limit the number of results:**
```powershell
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe -n 10 'search text'"
```

**5. Match exact whole words or case:**
```powershell
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe -whole-word 'ExactName'"
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe -case 'CaseSensitiveName'"
```

**6. Use regular expressions:**
```powershell
pwsh -Command ".\.gemini\skills\everything-search\tools\es.exe -regex '^test.*\.cpp$'"
```

## Useful Display & Export Options

*   **Format output:** You can export results to a file (e.g., `-export-csv out.csv`, `-export-txt out.txt`).
*   **Show attributes:** Use `-size`, `-date-modified`, `-attributes` to show more details about the found files.
*   **Sort results:** Sort by name, path, size, or date using `-sort-size`, `-sort-date-modified`, or descending with `-sort-size-descending`.
