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
    Write-Host "Error: No .uproject file found." -ForegroundColor Red
    exit 1
}

$UnrealEditor = Join-Path $Config.UEPath "Engine\Binaries\Win64\UnrealEditor.exe"

Write-Host "Launching Project: $($UProjectFile.Name)..." -ForegroundColor Cyan

Start-Process -FilePath $UnrealEditor -ArgumentList "`"$($UProjectFile.FullName)`""

Write-Host "Unreal Editor is opening in the background." -ForegroundColor Green
