# Author attribution: jingran Cheng (confirmed by project owner).
# For internal learning use only. Original source notices retained.
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$outputDir = Join-Path $projectRoot 'output'
$binaryPath = Join-Path $outputDir 'firmware_f1_0.bin'
$builtHexPath = Join-Path $outputDir 'keil_objects\firmware_f1_0.hex'
$hexPath = Join-Path $outputDir 'firmware_f1_0.hex'
$configPath = Join-Path $projectRoot '05_application\firmware_app\source\inc\project_config.h'

foreach ($required in @($binaryPath, $builtHexPath, $configPath)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required packaging input is missing: $required"
    }
}
Copy-Item -LiteralPath $builtHexPath -Destination $hexPath -Force
$andarImagePath = Join-Path $outputDir 'firmware_f1_0_andar_flash.bin'
& (Join-Path $PSScriptRoot 'make_andar_flash_image.ps1') `
    -InputBin $binaryPath -OutputBin $andarImagePath `
    -PayloadOffset 16 -SpiClockDivider 2
if (-not (Test-Path -LiteralPath $andarImagePath -PathType Leaf)) {
    throw "ANDAR Boot V3 image was not created: $andarImagePath"
}

$config = Get-Content -LiteralPath $configPath -Raw
function Read-ConfigNumber([string]$name) {
    $match = [regex]::Match($config, "(?m)^#define\s+$name\s+\(?([0-9]+)u?\)?\s*$")
    if (-not $match.Success) { throw "Missing numeric configuration: $name" }
    return [int]$match.Groups[1].Value
}
function Get-Sha256Hex([string]$path) {
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        $stream = [IO.File]::OpenRead($path)
        try {
            $hash = $sha256.ComputeHash($stream)
        } finally {
            $stream.Dispose()
        }
    } finally {
        $sha256.Dispose()
    }
    return -join ($hash | ForEach-Object { $_.ToString('x2') })
}

$files = @($binaryPath, $hexPath, $andarImagePath)
$hashLines = foreach ($file in $files) {
    "{0}  {1}" -f (Get-Sha256Hex $file), (Split-Path -Leaf $file)
}
Set-Content -LiteralPath (Join-Path $outputDir 'SHA256SUMS.txt') -Value $hashLines -Encoding ascii

$manifest = [ordered]@{
    product = 'firmware_f1_0'
    version = '{0}.{1}.{2}' -f (Read-ConfigNumber 'PROJECT_VERSION_MAJOR'), (Read-ConfigNumber 'PROJECT_VERSION_MINOR'), (Read-ConfigNumber 'PROJECT_VERSION_PATCH')
    board_variant = Read-ConfigNumber 'PROJECT_BOARD_VARIANT'
    generated_at_utc = (Get-Date).ToUniversalTime().ToString('o')
    artifacts = foreach ($file in $files) {
        $item = Get-Item -LiteralPath $file
        [ordered]@{
            file = $item.Name
            bytes = $item.Length
            sha256 = Get-Sha256Hex $file
        }
    }
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $outputDir 'manifest.json') -Encoding utf8
Write-Host "Package metadata written to $outputDir"
