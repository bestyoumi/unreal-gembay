$ErrorActionPreference = 'Stop'
$url = 'https://www.voidtools.com/ES-1.1.0.30.x64.zip'
$zipPath = Join-Path -Path $env:TEMP -ChildPath 'es.zip'
$extractPath = Join-Path -Path $env:TEMP -ChildPath 'es_extract'

Write-Host "Downloading es.exe from $url..."
Invoke-WebRequest -Uri $url -OutFile $zipPath

Write-Host "Extracting archive..."
Expand-Archive -Path $zipPath -DestinationPath $extractPath -Force

$skillDir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Definition)
$destPath = Join-Path -Path $skillDir -ChildPath 'tools\es.exe'

Write-Host "Copying es.exe to $destPath..."
$toolsDir = Join-Path -Path $skillDir -ChildPath 'tools'
if (-not (Test-Path $toolsDir)) {
    New-Item -ItemType Directory -Force -Path $toolsDir | Out-Null
}

Copy-Item -Path (Join-Path -Path $extractPath -ChildPath 'es.exe') -Destination $destPath -Force

Write-Host "Cleaning up temporary files..."
Remove-Item -Path $zipPath -Force
Remove-Item -Path $extractPath -Recurse -Force

Write-Host "Configuring Gemini CLI Policy to allow es.exe without confirmation..."
$policyDir = Join-Path -Path $PWD -ChildPath '.gemini\policies'
if (-not (Test-Path $policyDir)) {
    New-Item -ItemType Directory -Force -Path $policyDir | Out-Null
}
$policyContent = @"
[[rule]]
toolName = "run_shell_command"
commandRegex = ".*es\.exe.*"
decision = "allow"
priority = 100
"@
$policyFile = Join-Path -Path $policyDir -ChildPath 'allow-es.toml'
Set-Content -Path $policyFile -Value $policyContent -Encoding UTF8

Write-Host "Success: es.exe has been updated, installed, and whitelisted."
