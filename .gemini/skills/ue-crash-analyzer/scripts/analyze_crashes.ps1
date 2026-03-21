$CrashDir = Join-Path (Get-Location) "Saved\Crashes"

if (-not (Test-Path $CrashDir)) {
    Write-Host "No crashes found in $CrashDir"
    exit 0
}

# Get latest 3 crash folders
$Crashes = Get-ChildItem -Path $CrashDir -Directory | Sort-Object LastWriteTime -Descending | Select-Object -First 3

if (-not $Crashes) {
    Write-Host "No crash reports found."
    exit 0
}

Write-Host "--- Latest Crash Reports ---" -ForegroundColor Yellow

foreach ($Crash in $Crashes) {
    Write-Host "`n[Crash: $($Crash.Name)] ($($Crash.LastWriteTime))" -ForegroundColor Cyan
    
    $ContextFile = Join-Path $Crash.FullName "CrashContext.runtime-xml"
    $LogFile = Get-ChildItem -Path $Crash.FullName -Filter "*.log" | Select-Object -First 1

    if (Test-Path $ContextFile) {
        $Xml = [xml](Get-Content $ContextFile)
        $ErrorMsg = $Xml.FGenericCrashContext.RuntimeProperties.ErrorMessage
        $CallStack = $Xml.FGenericCrashContext.RuntimeProperties.CallStack
        
        Write-Host "Error: $ErrorMsg" -ForegroundColor Red
        Write-Host "CallStack Summary:"
        # Show first 5 frames of the callstack
        $Frames = $CallStack -split "`n"
        $Frames | Select-Object -First 8 | ForEach-Object { Write-Host "  $_" }
    }

    if ($LogFile -and (Test-Path $LogFile.FullName)) {
        Write-Host "Log Tail (Last 5 lines):" -ForegroundColor Gray
        Get-Content $LogFile.FullName -Tail 5 | ForEach-Object { Write-Host "  $_" }
    }
}
