$ProjectDir = Get-Location
$UProjectFile = Get-ChildItem -Path $ProjectDir -Filter "*.uproject" | Select-Object -First 1

if (-not $UProjectFile) {
    Write-Host "Error: No .uproject file found in the current directory." -ForegroundColor Red
    exit 1
}

Write-Host "--- Cleaning Project: $($UProjectFile.Name) ---" -ForegroundColor Cyan

# 1. Terminate Unreal Engine and related processes to release file locks
Write-Host "Closing Unreal Editor and Live Coding Console..." -ForegroundColor Yellow
$ProcessesToKill = @("UnrealEditor", "LiveCodingConsole")
foreach ($ProcName in $ProcessesToKill) {
    $Proc = Get-Process $ProcName -ErrorAction SilentlyContinue
    if ($Proc) {
        Write-Host "Terminating $ProcName (ID: $($Proc.Id))..."
        $Proc | Stop-Process -Force
        Sleep 1
    }
}

# 2. Define directories and files to remove
$TargetsToRemove = @(
    "Binaries",
    "Intermediate",
    "DerivedDataCache",
    ".vs",
    ".idea",
    "*.sln"
)

foreach ($Target in $TargetsToRemove) {
    $Path = Join-Path $ProjectDir $Target
    if (Test-Path $Path) {
        Write-Host "Removing: $Target..." -ForegroundColor Gray
        Remove-Item -Path $Path -Recurse -Force -ErrorAction SilentlyContinue
    }
}

Write-Host "Successfully cleaned project cache and temporary files." -ForegroundColor Green
Write-Host "Note: You must run 'generate_vs_project.ps1' before rebuilding." -ForegroundColor Yellow
