param (
    [Parameter(Mandatory=$true)]
    [string]$Name,
    [Parameter(Mandatory=$true)]
    [string]$JsonParams
)

$Url = "http://127.0.0.1:30010/remote/object/call"
$TempFile = Join-Path $env:TEMP "unreal_mat_gen_body.json"

$Body = @{
    objectPath = "/Script/GemBay.Default__GemBaySubsystem"
    functionName = "GenerateMaterial"
    parameters = @{
        Name = $Name
        JsonParams = $JsonParams
    }
    generateTransaction = $false
} | ConvertTo-Json -Compress

$Body | Out-File -FilePath $TempFile -Encoding ascii

try {
    Write-Host "Sending Remote Control request to generate material: $Name..." -ForegroundColor Cyan
    $Result = & curl.exe -s -X PUT $Url -d "@$TempFile" -H "Content-Type: application/json"
    if ($LASTEXITCODE -ne 0) { throw "curl failed" }
    Write-Host "Success: Material generation request sent." -ForegroundColor Green
} catch {
    Write-Host "Error: Failed to connect to Unreal Remote Control API." -ForegroundColor Red
    exit 1
} finally {
    if (Test-Path $TempFile) { Remove-Item $TempFile }
}
