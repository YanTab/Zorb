# Setup script pour Zorb Earth Run
# Usage : powershell -ExecutionPolicy Bypass -File .\tools\setup.ps1

Write-Host "[1/3] Vérification de la version Unreal..."
$uproject = "ZorbEarthRun.uproject"
if (-Not (Test-Path $uproject)) {
    Write-Error "Le fichier $uproject est introuvable. Vérifie le chemin."; exit 1
}
$json = Get-Content $uproject -Raw | ConvertFrom-Json
if ($json.EngineAssociation -ne "5.5") {
    Write-Warning "EngineAssociation = $($json.EngineAssociation). Nous recommandons 5.5." 
} else {
    Write-Host "EngineAssociation est bien 5.5." 
}

Write-Host "[2/3] Initialisation de l’environnement Python pour Tools/HeightmapProcessing..."
$pythonExe = $null
$pythonArgs = @()
if (Get-Command py -ErrorAction SilentlyContinue) {
    $pythonExe = "py"
    $pythonArgs = @("-3")
} elseif (Get-Command python -ErrorAction SilentlyContinue) {
    $pythonExe = "python"
    $pythonArgs = @()
}

if (-not $pythonExe) {
    Write-Error "Python n'est pas trouvé. Installe Python 3.11+ et relance le script."
    Write-Host "Télécharger Python: https://www.python.org/downloads/"
    Write-Host "Après installation, relance: powershell -ExecutionPolicy Bypass -File .\\tools\\setup.ps1"
    exit 1
}

Write-Host "Utilisation de Python : $pythonExe $($pythonArgs -join ' ')"
& $pythonExe @pythonArgs -m venv .venv
$pip = ".\.venv\Scripts\pip.exe"
if (-Not (Test-Path $pip)) {
    Write-Host "pip introuvable dans .venv, tentative d'installation via ensurepip..."
    & $pythonExe @pythonArgs -m ensurepip --upgrade
    & $pythonExe @pythonArgs -m pip install --upgrade pip
    if (-Not (Test-Path $pip)) { Write-Error "pip introuvable dans .venv après ensurepip. Vérifie l'installation Python."; exit 1 }
}
& $pip install --upgrade pip
& $pip install numpy pillow
Write-Host "Environnement Python créé dans .venv et packages installés.";

Write-Host "[3/3] Création des fichiers de configuration nécessaires..."
if (-Not (Test-Path "Tools\HeightmapProcessing\requirements.txt")) {
    @("numpy", "pillow") | Out-File -FilePath "Tools\HeightmapProcessing\requirements.txt" -Encoding UTF8
    Write-Host "requirements.txt généré.";
}

Write-Host "Setup terminé. Ouvre Unreal Engine 5.5 et charge le projet `ZorbEarthRun.uproject`."
