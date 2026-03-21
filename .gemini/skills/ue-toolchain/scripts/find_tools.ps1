function Get-ToolConfig {
    $ConfigPath = Join-Path $PSScriptRoot "tools_config.json"
    if (Test-Path $ConfigPath) {
        return Get-Content $ConfigPath | ConvertFrom-Json
    }
    return $null
}

function Save-ToolConfig($Config) {
    $Config | ConvertTo-Json | Out-File (Join-Path $PSScriptRoot "tools_config.json")
}

function Find-UnrealEngine {
    $SearchPaths = @(
        "C:\Program Files\Epic Games",
        "D:\Program Files\Epic Games",
        "E:\Program Files\Epic Games"
    )
    foreach ($Path in $SearchPaths) {
        if (Test-Path $Path) {
            $Engines = Get-ChildItem -Path $Path -Filter "UE_5*" | Sort-Object Name -Descending
            if ($Engines) { return $Engines[0].FullName }
        }
    }
    return $null
}

function Find-VisualStudio {
    # 1. Try vswhere
    $VSWherePaths = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\Installer\vswhere.exe"
    )
    foreach ($VSWhere in $VSWherePaths) {
        if (Test-Path $VSWhere) {
            $Path = & $VSWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools -property installationPath
            if ($Path) { return $Path }
        }
    }
    
    # 2. Known specific path from discovery
    $KnownPath = "C:\Program Files\Microsoft Visual Studio\18\Community"
    if (Test-Path $KnownPath) { return $KnownPath }

    # 3. Fallback: common install paths
    $CommonVS = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2026\Community",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community"
    )
    foreach ($Path in $CommonVS) {
        if (Test-Path $Path) { return $Path }
    }
    return $null
}

function Find-Rider {
    # 1. Known specific directory from discovery
    $KnownJetBrains = Get-ChildItem -Path "C:\" -Filter "JetBrains.Rider*" -Directory -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($KnownJetBrains) {
        $Exe = Join-Path $KnownJetBrains.FullName "bin\rider64.exe"
        if (Test-Path $Exe) { return $Exe }
    }

    # 2. Standard paths
    $SearchPaths = @(
        "${env:ProgramFiles}\JetBrains",
        "${env:ProgramFiles(x86)}\JetBrains",
        "${env:LocalAppdata}\JetBrains\Toolbox\apps\Rider",
        "C:\JetBrains",
        "C:\Rider"
    )
    foreach ($Path in $SearchPaths) {
        if (Test-Path $Path) {
            $Exe = Get-ChildItem -Path $Path -Filter "rider64.exe" -Recurse -Depth 4 -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($Exe) { return $Exe.FullName }
        }
    }
    return $null
}

function Find-VSCode {
    $CommonPaths = @(
        "${env:ProgramFiles}\Microsoft VS Code\Code.exe",
        "${env:ProgramFiles(x86)}\Microsoft VS Code\Code.exe",
        "${env:LocalAppdata}\Programs\Microsoft VS Code\Code.exe"
    )
    foreach ($P in $CommonPaths) {
        if (Test-Path $P) { return $P }
    }

    # Fallback to Get-Command but try to resolve the exe from the cmd/bin path
    $CmdPath = (Get-Command code -ErrorAction SilentlyContinue).Source
    if ($CmdPath) {
        $ExePath = $CmdPath -replace '\\bin\\code\.cmd$', '\Code.exe' -replace '\\bin\\code$', '\Code.exe'
        if (Test-Path $ExePath) { return $ExePath }
        return $CmdPath
    }
    
    return $null
}

# Main Discovery Logic
Write-Host "Re-scanning for development tools..." -ForegroundColor Cyan
$Config = [PSCustomObject]@{
    UEPath = Find-UnrealEngine
    VSPath = Find-VisualStudio
    RiderPath = Find-Rider
    VSCodePath = Find-VSCode
    LastUpdated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}
Save-ToolConfig $Config

# Output Results
Write-Host "--- Unreal Toolchain Config (Updated) ---" -ForegroundColor Yellow
if ($null -eq $Config.UEPath) { Write-Host "Unreal Engine:  Not Found" } else { Write-Host "Unreal Engine:  $($Config.UEPath)" }
if ($null -eq $Config.VSPath) { Write-Host "Visual Studio:  Not Found" } else { Write-Host "Visual Studio:  $($Config.VSPath)" }
if ($null -eq $Config.RiderPath) { Write-Host "Rider:          Not Found" } else { Write-Host "Rider:          $($Config.RiderPath)" }
if ($null -eq $Config.VSCodePath) { Write-Host "VS Code:        Not Found" } else { Write-Host "VS Code:        $($Config.VSCodePath)" }
Write-Host "------------------------------------------"
