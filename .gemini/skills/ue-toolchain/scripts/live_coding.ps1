$ProjectDir = Get-Location
$UProjectFile = Get-ChildItem -Path $ProjectDir -Filter "*.uproject" | Select-Object -First 1
if (-not $UProjectFile) {
    Write-Host "Error: No .uproject file found." -ForegroundColor Red
    exit 1
}
$ProjectName = $UProjectFile.BaseName
$LogPath = Join-Path $ProjectDir "Saved\Logs\$($ProjectName).log"

Write-Host "Triggering Live Coding via Remote Control API for project: $ProjectName..." -ForegroundColor Cyan

# 1. Check Editor process
$EditorProc = Get-Process UnrealEditor -ErrorAction SilentlyContinue
if (-not $EditorProc) {
    Write-Host "Error: Unreal Editor is not running." -ForegroundColor Red
    exit 1
}

# 2. Get current log state
$InitialLineCount = 0
if (Test-Path $LogPath) {
    $InitialLineCount = (Get-Content $LogPath).Count
}

# 3. Call Remote Control API via curl
$TempFile = Join-Path $env:TEMP "unreal_rc_body.json"
$Body = @{
    objectPath = "/Script/GemBay.Default__GemBaySubsystem"
    functionName = "TriggerLiveCoding"
    generateTransaction = $false
} | ConvertTo-Json -Compress

$Body | Out-File -FilePath $TempFile -Encoding ascii

try {
    # Use curl.exe directly as Invoke-RestMethod can be unreliable with some local listener configurations
    $Result = & curl.exe -s -X PUT "http://127.0.0.1:30010/remote/object/call" -d "@$TempFile" -H "Content-Type: application/json"
    if ($LASTEXITCODE -ne 0) { throw "curl failed" }
    Write-Host "Remote Control request sent successfully." -ForegroundColor Green
} catch {
    Write-Host "Error: Failed to connect to Unreal Remote Control API." -ForegroundColor Red
    Write-Host "Ensure 'Remote Control' plugin is enabled and port 30010 is open." -ForegroundColor Yellow
    exit 1
} finally {
    if (Test-Path $TempFile) { Remove-Item $TempFile }
}

# 4. Monitor Live Coding Console and Logs
Write-Host "Waiting for build results..." -ForegroundColor Cyan
$TimeoutSeconds = 60
$StartTime = Get-Date

while (((Get-Date) - $StartTime).TotalSeconds -lt $TimeoutSeconds) {
    if (Test-Path $LogPath) {
        $AllLines = Get-Content $LogPath
        $CurrentLineCount = $AllLines.Count
        
        if ($CurrentLineCount -gt $InitialLineCount) {
            for ($i = $InitialLineCount; $i -lt $CurrentLineCount; $i++) {
                $Line = $AllLines[$i]
                if ($Line -like "*LogLiveCoding: Display: Live coding succeeded*") {
                    Write-Host "Success: Live coding succeeded!" -ForegroundColor Green
                    exit 0
                }
                if ($Line -like "*LogLiveCoding: Error: Live coding failed*") {
                    Write-Host "Error: Live coding failed." -ForegroundColor Red
                    exit 1
                }
            }
        }
    }
    Sleep 1
}

Write-Host "Error: Timeout waiting for Live Coding results." -ForegroundColor Red
exit 1
