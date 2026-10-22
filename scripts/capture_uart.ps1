param(
    [Parameter(Mandatory=$true)][string]$Port,
    [ValidateSet('t','c','s')][string]$Mode = 't',
    [string]$OutputCsv = ''
)
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputCsv) { $OutputCsv = Join-Path $taskRoot ('capture_' + $Mode + '.csv') }
$taskSerial = [System.IO.Ports.SerialPort]::new($Port,115200,[System.IO.Ports.Parity]::None,8,[System.IO.Ports.StopBits]::One)
$taskSerial.ReadTimeout = 1000
$taskSerial.WriteTimeout = 1000
$taskSerial.NewLine = "`n"
$taskSerial.DtrEnable = $false
$taskSerial.RtsEnable = $false
$taskBytes = [System.Collections.Generic.List[int]]::new()
$taskLines = [System.Collections.Generic.List[string]]::new()
$taskInFrame = $false
$taskComplete = $false
try {
    $taskSerial.Open()
    $taskSerial.DiscardInBuffer()
    $taskSerial.Write($Mode)
    $taskDeadline = [DateTime]::UtcNow.AddSeconds(15)
    while ([DateTime]::UtcNow -lt $taskDeadline) {
        try { $taskLine = $taskSerial.ReadLine().Trim() }
        catch [System.TimeoutException] { continue }
        $taskLines.Add($taskLine)
        Write-Output $taskLine
        if ($Mode -eq 's' -and $taskLine.StartsWith('STATUS ')) { $taskComplete = $true; break }
        if ($taskLine.StartsWith('BEGIN ADC100 ')) {
            $taskBytes.Clear(); $taskInFrame = $true
        } elseif ($taskInFrame -and $taskLine -match '^[0-9a-fA-F]{2}( [0-9a-fA-F]{2})*$') {
            foreach ($taskHex in $taskLine.Split(' ')) { $taskBytes.Add([Convert]::ToInt32($taskHex,16)) }
        } elseif ($taskLine.StartsWith('END ADC100 ')) {
            if ($taskLine -notmatch 'errors=0$') { throw "Board reported invalid data or guard corruption: $taskLine" }
        } elseif ($taskLine -eq 'SELFTEST PASS' -or $taskLine.StartsWith('CAPTURE OK ')) {
            $taskComplete = $true; break
        } elseif ($taskLine.StartsWith('ERROR:') -or $taskLine.StartsWith('CAPTURE FAILED') -or $taskLine.StartsWith('NOT_READY:')) {
            throw $taskLine
        }
    }
    if (-not $taskComplete) { throw 'Serial response timed out. Verify app is running, correct PS UART COM port, and 115200 8N1.' }
    if ($Mode -ne 's') {
        if ($taskBytes.Count -ne 100) { throw "Expected 100 bytes, received $($taskBytes.Count)" }
        if ($Mode -eq 't') {
            for ($taskIndex=0; $taskIndex -lt 100; $taskIndex++) {
                if ($taskBytes[$taskIndex] -ne $taskIndex) { throw "Self-test mismatch at byte $taskIndex" }
            }
        }
        $taskRows = for ($taskIndex=0; $taskIndex -lt 100; $taskIndex++) {
            [pscustomobject]@{index=$taskIndex; time_us=($taskIndex/10.0); code=$taskBytes[$taskIndex]}
        }
        $taskRows | Export-Csv -LiteralPath $OutputCsv -NoTypeInformation -Encoding UTF8
        $taskLines | Set-Content -LiteralPath ($OutputCsv + '.log') -Encoding UTF8
        Write-Output "PC_CAPTURE_PASS bytes=100 CSV=$OutputCsv"
    }
} finally {
    if ($taskSerial.IsOpen) { $taskSerial.Close() }
    $taskSerial.Dispose()
}
