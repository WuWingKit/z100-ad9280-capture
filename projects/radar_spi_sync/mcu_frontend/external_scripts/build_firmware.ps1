# Author attribution: jingran Cheng (confirmed by project owner).
# For internal learning use only. Original source notices retained.
[CmdletBinding()]
param(
    [string]$KeilUv4 = $env:KEIL_UV4
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot '05_application\firmware_app\keil_project\firmware_app.uvprojx'
$outputDir = Join-Path $projectRoot 'output'
$objectDir = Join-Path $outputDir 'keil_objects'
$buildLog = Join-Path $outputDir 'build.log'

if ([string]::IsNullOrWhiteSpace($KeilUv4)) {
    $candidates = @((Join-Path $env:LOCALAPPDATA 'Keil_v5\UV4\UV4.exe'), 'C:\Keil_v5\UV4\UV4.exe', 'C:\Keil\UV4\UV4.exe')
    $KeilUv4 = $candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
}
if (-not (Test-Path -LiteralPath $KeilUv4 -PathType Leaf)) {
    throw "Keil UV4 was not found at '$KeilUv4'. Set KEIL_UV4 or pass -KeilUv4."
}
if (-not (Test-Path -LiteralPath $projectFile -PathType Leaf)) {
    throw "Keil project not found: $projectFile"
}

New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
if (Test-Path -LiteralPath $objectDir) {
    $resolvedOutput = (Resolve-Path -LiteralPath $outputDir).Path
    $resolvedObjects = (Resolve-Path -LiteralPath $objectDir).Path
    if (-not $resolvedObjects.StartsWith($resolvedOutput, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clean an object directory outside output: $resolvedObjects"
    }
    Remove-Item -LiteralPath $objectDir -Recurse -Force
}
New-Item -ItemType Directory -Path $objectDir -Force | Out-Null
if (Test-Path -LiteralPath $buildLog) {
    Remove-Item -LiteralPath $buildLog -Force
}

$process = Start-Process -FilePath $KeilUv4 `
    -ArgumentList ('-r "' + $projectFile + '" -j0 -o "' + $buildLog + '"') `
    -WorkingDirectory (Split-Path -Parent $projectFile) `
    -WindowStyle Hidden -Wait -PassThru

if (-not (Test-Path -LiteralPath $buildLog)) {
    throw "Keil did not produce a build log (exit code $($process.ExitCode))."
}
$logText = Get-Content -LiteralPath $buildLog -Raw
if ($process.ExitCode -ge 2 -or $logText -notmatch '0 Error\(s\)') {
    Get-Content -LiteralPath $buildLog -Tail 100
    throw "Firmware build failed with exit code $($process.ExitCode)."
}

foreach ($artifact in @('firmware_f1_0.axf', 'firmware_f1_0.hex')) {
    $artifactPath = Join-Path $objectDir $artifact
    if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
        throw "Expected build artifact is missing: $artifactPath"
    }
}
$binaryPath = Join-Path $outputDir 'firmware_f1_0.bin'
if (-not (Test-Path -LiteralPath $binaryPath -PathType Leaf)) {
    throw "Expected post-build binary is missing: $binaryPath"
}
$andarImagePath = Join-Path $outputDir 'firmware_f1_0_andar_flash.bin'
if (-not (Test-Path -LiteralPath $andarImagePath -PathType Leaf)) {
    throw "Expected ANDAR Boot V3 image is missing: $andarImagePath"
}

Write-Host 'Firmware build succeeded.'
Write-Host "Log: $buildLog"
Write-Host "Binary: $binaryPath"
Write-Host "ANDAR Boot V3 image: $andarImagePath"
