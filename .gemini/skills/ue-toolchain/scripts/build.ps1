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

# Safety check: Cannot build while editor is running (unless using live coding)
if (Get-Process UnrealEditor -ErrorAction SilentlyContinue) {
    Write-Host "Warning: Unreal Editor is running. Full builds may fail due to file locks." -ForegroundColor Yellow
    Write-Host "Please close the editor or use live_coding.ps1 instead." -ForegroundColor Yellow
}

$BuildBat = Join-Path $Config.UEPath "Engine\Build\BatchFiles\Build.bat"
$ProjectName = $UProjectFile.BaseName

Write-Host "Building $ProjectName (Win64 Development Editor)..." -ForegroundColor Cyan

Start-Process -FilePath $BuildBat -ArgumentList "$ProjectName`Editor Win64 Development -Project=`"$($UProjectFile.FullName)`" -WaitMutex" -Wait -NoNewWindow

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build Succeeded!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "Build Failed with Exit Code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}
