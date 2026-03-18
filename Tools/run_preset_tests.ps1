# Matrix runner for Zorb preset comparative tests
# Usage (quick): powershell -ExecutionPolicy Bypass -File .\Tools\run_preset_tests.ps1
# Usage (full):  powershell -ExecutionPolicy Bypass -File .\Tools\run_preset_tests.ps1 -FullMatrix

param(
    [switch]$FullMatrix
)

$projectRoot = Join-Path $PSScriptRoot ".."
$uproject = Join-Path $projectRoot "ZorbEarthRun.uproject"
$engineBinaries = "D:\Unreal_Engine\UE_5.5\Engine\Binaries\Win64"
$editor = Join-Path $engineBinaries "UnrealEditor.exe"
$configFile = Join-Path $projectRoot "Config\DefaultGame.ini"

if (-Not (Test-Path $editor)) {
    Write-Error "Unreal Editor not found at: $editor"
    exit 1
}

if (-Not (Test-Path $configFile)) {
    Write-Error "Config file not found at: $configFile"
    exit 1
}

$presets = @(
    @(0, "classic"),
    @(1, "agile"),
    @(2, "heavy"),
    @(3, "wild")
)

$quickScenarios = @(
    "ground_cp_run",
    "ground_valley_sprint",
    "ground_valley_switchback"
)

$fullScenarios = @(
    "air_no_input",
    "air_forward_hold",
    "air_turn_left_hold",
    "air_brake_pulse",
    "air_boost_window",
    "ground_cp_run",
    "ground_valley_sprint",
    "ground_valley_switchback"
)

$scenarios = if ($FullMatrix) { $fullScenarios } else { $quickScenarios }

$originalContent = Get-Content $configFile -Raw

function Update-ConfigForRun {
    param(
        [string]$Content,
        [int]$PresetValue,
        [string]$ScenarioName,
        [string]$OutputPrefix
    )

    $updated = $Content
    $updated = $updated -replace '(ZorbTypePreset=)\d+', "`$1$PresetValue"
    $updated = $updated -replace '(Automation=\(bEnableScenarioRunner=True,ScenarioName=")[^"]+(")', "`$1$ScenarioName`$2"
    $updated = $updated -replace '(OutputFilePrefix=")[^"]+(")', "`$1$OutputPrefix`$2"
    return $updated
}

Write-Host "=== Zorb Preset x Scenario Matrix ===" -ForegroundColor Cyan
Write-Host "Mode: $(if ($FullMatrix) { 'Full (8 scenarios)' } else { 'Quick (3 ground scenarios)' })"
Write-Host "Total runs: $($presets.Count * $scenarios.Count)"
Write-Host ""
Write-Host "For each launch:" -ForegroundColor Yellow
Write-Host "  1. Press PLAY"
Write-Host "  2. Let the automated scenario finish"
Write-Host "  3. Stop PIE and close Unreal Editor"
Write-Host ""

$runIndex = 1
$totalRuns = $presets.Count * $scenarios.Count

try {
    foreach ($scenario in $scenarios) {
        foreach ($preset in $presets) {
            $presetValue = $preset[0]
            $presetName = $preset[1]
            $prefix = "zorb_${presetName}_${scenario}"

            Write-Host "───────────────────────────────────────────" -ForegroundColor Cyan
            Write-Host ("Run {0}/{1} - preset={2} ({3}) - scenario={4}" -f $runIndex, $totalRuns, $presetName, $presetValue, $scenario) -ForegroundColor Green
            Write-Host "───────────────────────────────────────────" -ForegroundColor Cyan

            $newContent = Update-ConfigForRun -Content $originalContent -PresetValue $presetValue -ScenarioName $scenario -OutputPrefix $prefix
            Set-Content $configFile -Value $newContent -NoNewline

            $proc = Start-Process -FilePath $editor -ArgumentList $uproject -PassThru
            $proc.WaitForExit()

            Write-Host "Saved telemetry prefix: $prefix" -ForegroundColor DarkGreen
            Start-Sleep -Seconds 1
            $runIndex += 1
        }
    }
}
finally {
    Set-Content $configFile -Value $originalContent -NoNewline
    Write-Host ""
    Write-Host "Config restored to original values." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== Matrix completed ===" -ForegroundColor Cyan
Write-Host "Telemetry files: $projectRoot\Saved\Telemetry\" -ForegroundColor Green
Write-Host ""
Write-Host "Tip: filter by prefix zorb_classic_ / zorb_agile_ / zorb_heavy_ / zorb_wild_"
