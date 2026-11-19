# Author attribution: jingran Cheng (confirmed by project owner).
# For internal learning use only. Original source notices retained.
[CmdletBinding()]
param(
    [string]$InputBin,
    [string]$OutputBin,
    [ValidateRange(16, 16777215)]
    [int]$PayloadOffset = 16,
    [ValidateRange(1, 255)]
    [int]$SpiClockDivider = 2
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($InputBin)) {
    $InputBin = Join-Path $projectRoot 'output\firmware_f1_0.bin'
}
if ([string]::IsNullOrWhiteSpace($OutputBin)) {
    $OutputBin = Join-Path $projectRoot 'output\firmware_f1_0_andar_flash.bin'
}
$InputBin = [IO.Path]::GetFullPath($InputBin)
$OutputBin = [IO.Path]::GetFullPath($OutputBin)
if (-not (Test-Path -LiteralPath $InputBin -PathType Leaf)) {
    throw "Raw application binary not found: $InputBin"
}

[byte[]]$payload = [IO.File]::ReadAllBytes($InputBin)
if ($payload.Length -eq 0 -or $payload.Length -gt 0xFFFF) {
    throw "ANDAR Boot V3 stores payload length in 16 bits; got $($payload.Length) bytes."
}
if (($PayloadOffset + $payload.Length) -gt 0x10010) {
    throw 'Packaged image exceeds the ADT6101P 64 KiB application payload limit.'
}

function Get-Crc32([byte[]]$data) {
    [uint64]$crc = 0xFFFFFFFFL
    foreach ($value in $data) {
        $crc = $crc -bxor [uint64]$value
        for ($bit = 0; $bit -lt 8; $bit++) {
            if (($crc -band 1) -ne 0) {
                $crc = (($crc -shr 1) -bxor 0xEDB88320L) -band 0xFFFFFFFFL
            } else {
                $crc = ($crc -shr 1) -band 0xFFFFFFFFL
            }
        }
    }
    return [uint32](($crc -bxor 0xFFFFFFFFL) -band 0xFFFFFFFFL)
}

$size = $payload.Length
$crc32 = Get-Crc32 $payload
[byte[]]$image = New-Object byte[] ($PayloadOffset + $size)
$image[0] = [byte]($size -band 0xFF)
$image[1] = [byte](($size -shr 8) -band 0xFF)
$image[2] = $image[0]
$image[3] = $image[1]
$image[4] = 0x5A
$image[5] = [byte]($PayloadOffset -band 0xFF)
$image[6] = [byte](($PayloadOffset -shr 8) -band 0xFF)
$image[7] = [byte](($PayloadOffset -shr 16) -band 0xFF)
$image[8] = [byte]$SpiClockDivider
$image[9] = [byte](($crc32 -shr 24) -band 0xFF)
$image[10] = [byte](($crc32 -shr 16) -band 0xFF)
$image[11] = [byte](($crc32 -shr 8) -band 0xFF)
$image[12] = [byte]($crc32 -band 0xFF)
[Array]::Copy($payload, 0, $image, $PayloadOffset, $size)

$parent = Split-Path -Parent $OutputBin
New-Item -ItemType Directory -Path $parent -Force | Out-Null
[IO.File]::WriteAllBytes($OutputBin, $image)

# Re-open and validate every field before reporting success.
[byte[]]$check = [IO.File]::ReadAllBytes($OutputBin)
$storedSizeA = [int]$check[0] -bor ([int]$check[1] -shl 8)
$storedSizeB = [int]$check[2] -bor ([int]$check[3] -shl 8)
$storedOffset = [int]$check[5] -bor ([int]$check[6] -shl 8) -bor ([int]$check[7] -shl 16)
$storedCrc = ([uint32]$check[9] -shl 24) -bor ([uint32]$check[10] -shl 16) -bor ([uint32]$check[11] -shl 8) -bor [uint32]$check[12]
if ($storedSizeA -ne $size -or $storedSizeB -ne $size -or
    $check[4] -ne 0x5A -or $storedOffset -ne $PayloadOffset -or
    $check[8] -ne $SpiClockDivider -or $storedCrc -ne $crc32) {
    throw 'Generated ANDAR Boot V3 header failed verification.'
}

Write-Host "ANDAR Boot V3 image: $OutputBin"
Write-Host ('Payload={0} bytes, offset=0x{1:X6}, SPI divider=0x{2:X2}, CRC32=0x{3:X8}' -f $size, $PayloadOffset, $SpiClockDivider, $crc32)

