param([string]$KeilRoot = 'E:/Keil_v5')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'mcu_frontend/05_application/firmware_app/keil_project/firmware_app.uvprojx'
$uv = Join-Path $KeilRoot 'UV4/UV4.exe'
$fromelf = Join-Path $KeilRoot 'ARM/ARMCC/bin/fromelf.exe'
foreach ($targetName in @('radar_sync_mcu','radar_sync_selftest')) {
    $logFile = Join-Path $projectRoot ('evidence/build_' + $targetName + '.txt')
    $buildArgs = @('-r', ('"'+$projectFile+'"'), '-t', $targetName, '-o', ('"'+$logFile+'"'))
    $proc = Start-Process -FilePath $uv -ArgumentList $buildArgs -WindowStyle Hidden -PassThru
    $proc.WaitForExit()
    $logText = Get-Content -LiteralPath $logFile -Raw
    if ($logText -notmatch '0 Error\(s\), 0 Warning\(s\)') {throw "Keil build failed or has warnings; inspect $logFile"}
    $objectDir = Join-Path $projectRoot 'mcu_frontend/output/keil_objects'
    $axf = Join-Path $objectDir ($targetName+'.axf')
    $binFile = Join-Path $projectRoot ('hardware/'+$targetName+'.bin')
    & $fromelf --bin --output $binFile $axf
    if ($LASTEXITCODE -ne 0) {throw 'fromelf failed'}
    foreach ($ext in @('axf','hex','map')) {
        Copy-Item -LiteralPath (Join-Path $objectDir ($targetName+'.'+$ext)) -Destination (Join-Path $projectRoot ('hardware/'+$targetName+'.'+$ext)) -Force
    }
    Write-Output ($targetName + ': BUILD PASS (no board access)')
}
