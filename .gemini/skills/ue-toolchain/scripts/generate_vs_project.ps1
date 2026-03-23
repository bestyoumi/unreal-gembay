$ConfigPath = Join-Path $PSScriptRoot "tools_config.json"
if (-not (Test-Path $ConfigPath)) {
    powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "find_tools.ps1")
}
$Config = Get-Content $ConfigPath | ConvertFrom-Json

if (-not $Config.UEPath) {
    Write-Host "Error: Unreal Engine path not configured. Run find_tools.ps1 first." -ForegroundColor Red
    exit 1
}

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
$UBTPath = Join-Path $Config.UEPath "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

if (Test-Path $UBTPath) {
    Write-Host "Regenerating project files with UnrealBuildTool..." -ForegroundColor Cyan
    Start-Process -FilePath $UBTPath -ArgumentList "-projectfiles", "-project=`"$FullUProjectPath`"", "-game", "-rocket", "-progress" -Wait -NoNewWindow
    Write-Host "Successfully regenerated Visual Studio project files." -ForegroundColor Green
} else {
    Write-Host "Error: UnrealBuildTool.exe not found at $UBTPath. Please verify your Unreal Engine installation." -ForegroundColor Red
    exit 1
}
