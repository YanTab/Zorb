param(
    [string]$PythonExe = "d:/Game_Project/ZorbEarthRun/.venv/Scripts/python.exe",
    [string]$OutDir = "Tools/HeightmapProcessing/output/candidates_v1"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\")).Path
$outputPath = Join-Path $repoRoot $OutDir
New-Item -ItemType Directory -Path $outputPath -Force | Out-Null

$candidates = @(
    @{
        Name = "valley_flow"
        Seed = 1011
        Profile = "flow"
        FlowAngle = -52
        GlobalSlope = 0.48
        CorridorCount = 1
        CorridorDepth = 0.62
        CorridorWidth = 0.13
        CorridorMeander = 0.035
        CorridorFrequency = 0.9
        SidebankStrength = 0.13
        MacroVariation = 0.08
        LowNoise = 0.012
        Smoothness = 0.86
        SlopeLimit = 0.016
        SlopeLimitPasses = 3
    },
    @{
        Name = "valley_switchback"
        Seed = 2027
        Profile = "switchback"
        FlowAngle = -50
        GlobalSlope = 0.41
        CorridorCount = 1
        CorridorDepth = 0.56
        CorridorWidth = 0.115
        CorridorMeander = 0.085
        CorridorFrequency = 2.0
        SidebankStrength = 0.16
        MacroVariation = 0.09
        LowNoise = 0.014
        Smoothness = 0.82
        SlopeLimit = 0.017
        SlopeLimitPasses = 3
    },
    @{
        Name = "valley_risk_reward"
        Seed = 3091
        Profile = "risk_reward"
        FlowAngle = -56
        GlobalSlope = 0.52
        CorridorCount = 2
        CorridorDepth = 0.60
        CorridorWidth = 0.105
        CorridorMeander = 0.06
        CorridorFrequency = 1.4
        SidebankStrength = 0.22
        MacroVariation = 0.11
        LowNoise = 0.016
        Smoothness = 0.78
        SlopeLimit = 0.018
        SlopeLimitPasses = 4
    }
)

foreach ($c in $candidates) {
    $name = [string]$c.Name
    $seed = [int]$c.Seed

    $raw2049 = Join-Path $outputPath ("{0}_2049_raw.png" -f $name)
    $norm2049 = Join-Path $outputPath ("{0}_2049_norm.png" -f $name)
    $norm1009 = Join-Path $outputPath ("{0}_1009_norm.png" -f $name)
    $preview = Join-Path $outputPath ("{0}_preview.png" -f $name)
    $r16 = Join-Path $outputPath ("{0}_1009.r16" -f $name)

    & $PythonExe (Join-Path $repoRoot "Tools/HeightmapProcessing/generate_heightmap.py") `
        --output $raw2049 `
        --size 2049 `
        --seed $seed `
        --profile $c.Profile `
        --flow-angle-deg $c.FlowAngle `
        --global-slope $c.GlobalSlope `
        --corridor-count $c.CorridorCount `
        --corridor-depth $c.CorridorDepth `
        --corridor-width $c.CorridorWidth `
        --corridor-meander $c.CorridorMeander `
        --corridor-frequency $c.CorridorFrequency `
        --sidebank-strength $c.SidebankStrength `
        --macro-variation $c.MacroVariation `
        --low-noise $c.LowNoise `
        --smoothness $c.Smoothness `
        --slope-limit $c.SlopeLimit `
        --slope-limit-passes $c.SlopeLimitPasses
    & $PythonExe (Join-Path $repoRoot "Tools/HeightmapProcessing/normalize_heightmap.py") --input $raw2049 --output $norm2049
    & $PythonExe (Join-Path $repoRoot "Tools/HeightmapProcessing/crop_heightmap.py") --input $norm2049 --output $norm1009 --size 1009
    & $PythonExe (Join-Path $repoRoot "Tools/HeightmapProcessing/generate_preview.py") --input $norm1009 --output $preview
    & $PythonExe (Join-Path $repoRoot "Tools/HeightmapProcessing/export_r16.py") --input $norm1009 --output $r16
}

$manifest = @{
    version = "v1"
    generated_at_utc = (Get-Date).ToUniversalTime().ToString("s") + "Z"
    size_work = 2049
    size_import = 1009
    candidates = $candidates
} | ConvertTo-Json -Depth 5

$manifestPath = Join-Path $outputPath "manifest_candidates_v1.json"
Set-Content -Path $manifestPath -Value $manifest -Encoding UTF8

Write-Host "Generated candidates in: $outputPath"
Get-ChildItem $outputPath | Select-Object Name, Length