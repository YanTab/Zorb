param(
    [switch]$SkipBuild,
    [switch]$LaunchEditor,
    [string]$Configuration = "Development"
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "[session] $Message"
}

function Resolve-UeBuildBat {
    param([string]$EngineAssociation)

    $candidates = @()

    if ($env:UE_BUILD_BAT) {
        $candidates += $env:UE_BUILD_BAT
    }

    if ($EngineAssociation) {
        $candidates += "D:\Unreal_Engine\UE_$EngineAssociation\Engine\Build\BatchFiles\Build.bat"
        $candidates += "C:\Program Files\Epic Games\UE_$EngineAssociation\Engine\Build\BatchFiles\Build.bat"
    }

    $candidates += @(
        "D:\Unreal_Engine\UE_5.5\Engine\Build\BatchFiles\Build.bat",
        "D:\Unreal_Engine\UE_5.6\Engine\Build\BatchFiles\Build.bat",
        "D:\Unreal_Engine\UE_5.4\Engine\Build\BatchFiles\Build.bat",
        "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat",
        "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat",
        "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat"
    )

    foreach ($path in $candidates) {
        if ($path -and (Test-Path $path)) {
            return $path
        }
    }

    $found = Get-ChildItem -Path "C:\", "D:\" -Filter "Build.bat" -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -like "*Engine\Build\BatchFiles\Build.bat" } |
        Select-Object -First 1 -ExpandProperty FullName

    return $found
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $repoRoot

$uprojectPath = Join-Path $repoRoot "ZorbEarthRun.uproject"
$slnPath = Join-Path $repoRoot "ZorbEarthRun.sln"

Write-Step "Repo root: $repoRoot"

if (-not (Test-Path $uprojectPath)) {
    throw "Missing file: $uprojectPath"
}
if (-not (Test-Path $slnPath)) {
    throw "Missing file: $slnPath"
}

$uprojectJson = Get-Content $uprojectPath -Raw | ConvertFrom-Json
$engineAssociation = [string]$uprojectJson.EngineAssociation

Write-Step "EngineAssociation: $engineAssociation"

$buildBat = Resolve-UeBuildBat -EngineAssociation $engineAssociation
if (-not $buildBat) {
    throw "Unable to locate Unreal Build.bat. Set UE_BUILD_BAT env var or install UE."
}

Write-Step "Using Unreal Build script: $buildBat"

$venvPython = Join-Path $repoRoot ".venv\Scripts\python.exe"
if (-not (Test-Path $venvPython)) {
    Write-Step "Creating Python virtual environment (.venv)"
    if (Get-Command py -ErrorAction SilentlyContinue) {
        & py -3 -m venv (Join-Path $repoRoot ".venv")
    }
    elseif (Get-Command python -ErrorAction SilentlyContinue) {
        & python -m venv (Join-Path $repoRoot ".venv")
    }
    else {
        throw "Python 3 not found. Install Python 3.11+ to use tools."
    }
}

if (Test-Path $venvPython) {
    Write-Step "Installing/updating Python dependencies"
    & $venvPython -m pip install --upgrade pip | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "pip upgrade failed with exit code $LASTEXITCODE"
    }

    $requirementsPath = Join-Path $repoRoot "Tools\HeightmapProcessing\requirements.txt"
    if (Test-Path $requirementsPath) {
        Write-Step "Installing requirements from Tools/HeightmapProcessing/requirements.txt"
        & $venvPython -m pip install -r $requirementsPath | Out-Null
        if ($LASTEXITCODE -ne 0) {
            throw "pip install -r failed with exit code $LASTEXITCODE"
        }
    }
    else {
        Write-Step "Installing default Python deps (numpy, pillow)"
        & $venvPython -m pip install numpy pillow | Out-Null
        if ($LASTEXITCODE -ne 0) {
            throw "pip install default deps failed with exit code $LASTEXITCODE"
        }
    }
}

Write-Step "Git status summary"
& git rev-parse --abbrev-ref HEAD
& git status --short

if (-not $SkipBuild) {
    Write-Step "Building ZorbEarthRunEditor Win64 $Configuration"
    & $buildBat ZorbEarthRunEditor Win64 $Configuration "-Project=$uprojectPath" -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
    Write-Step "Build succeeded"
}
else {
    Write-Step "Build skipped (-SkipBuild)"
}

if ($LaunchEditor) {
    Write-Step "Launching Unreal Editor"
    Start-Process -FilePath $uprojectPath
}

Write-Step "Session bootstrap completed"
