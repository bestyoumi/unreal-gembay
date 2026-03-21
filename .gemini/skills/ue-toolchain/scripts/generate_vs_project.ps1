$UProjectFile = Get-ChildItem -Path (Get-Location) -Filter "*.uproject" | Select-Object -First 1

if (-not $UProjectFile) {
    Write-Host "Error: No .uproject file found in the current directory." -ForegroundColor Red
    exit 1
}

$FullUProjectPath = $UProjectFile.FullName
Write-Host "Project Found: $FullUProjectPath" -ForegroundColor Cyan

# 1. Terminate Unreal Engine and related processes
Write-Host "Terminating Unreal Editor and Live Coding Console..." -ForegroundColor Yellow
$ProcessesToKill = @("UnrealEditor", "LiveCodingConsole")
foreach ($ProcName in $ProcessesToKill) {
    $Proc = Get-Process $ProcName -ErrorAction SilentlyContinue
    if ($Proc) {
        Write-Host "Terminating $ProcName (ID: $($Proc.Id))..."
        $Proc | Stop-Process -Force
        Sleep 1
    }
}

# 2. Re-generate Project Files
$UVSPath = "${env:ProgramFiles(x86)}\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"
if (-not (Test-Path $UVSPath)) {
    $UVSPath = "${env:ProgramFiles}\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"
}

if (Test-Path $UVSPath) {
    Write-Host "Regenerating project files with UnrealVersionSelector..." -ForegroundColor Cyan
    Start-Process -FilePath $UVSPath -ArgumentList "/projectfiles", "`"$FullUProjectPath`"" -Wait
    Write-Host "Successfully regenerated Visual Studio project files." -ForegroundColor Green
} else {
    Write-Host "Error: UnrealVersionSelector.exe not found. Please manually right-click the .uproject and select 'Generate Visual Studio project files'." -ForegroundColor Red
    exit 1
}
