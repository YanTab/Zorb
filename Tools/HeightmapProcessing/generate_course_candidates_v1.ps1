param(
    [string]$PythonExe = "d:/Game_Project/ZorbEarthRun/.venv/Scripts/python.exe",
    [string]$CandidatesDir = "Tools/HeightmapProcessing/output/candidates_v1",
    [int]$CheckpointCount = 5,
    [double]$XYScale = 100.0,
    [double]$ZScale = 50.0
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\")).Path
$candPath = Join-Path $repoRoot $CandidatesDir

if (-not (Test-Path $candPath)) {
    throw "Candidates directory not found: $candPath"
}

$files = Get-ChildItem -Path $candPath -Filter "*_1009_norm.png" | Sort-Object Name
if ($files.Count -eq 0) {
    throw "No candidate PNG files found in $candPath"
}

foreach ($f in $files) {
    $mapName = $f.BaseName -replace "_1009_norm$", ""
    $jsonOut = Join-Path $candPath ("{0}_course.json" -f $mapName)
    $pyOut = Join-Path $candPath ("{0}_apply_course.py" -f $mapName)

    & $PythonExe (Join-Path $repoRoot "Tools/HeightmapProcessing/generate_course_markers.py") `
        --input $f.FullName `
        --output-json $jsonOut `
        --output-unreal-py $pyOut `
        --map-name $mapName `
        --checkpoint-count $CheckpointCount `
        --xy-scale $XYScale `
        --z-scale $ZScale
}

Write-Host "Generated auto course files in: $candPath"
Get-ChildItem $candPath | Where-Object { $_.Name -match "_course\.json$|_apply_course\.py$" } | Select-Object Name, Length