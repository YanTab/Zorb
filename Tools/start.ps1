# Lancer le projet Unreal + ouvrir VS Code
# Usage : powershell -ExecutionPolicy Bypass -File .\tools\start.ps1

$uproject = Join-Path $PSScriptRoot "..\ZorbEarthRun.uproject"
if (-Not (Test-Path $uproject)) {
    Write-Error "Fichier uproject introuvable : $uproject"
    exit 1
}

# Essayer d'ouvrir Unreal Editor via engine association (double cliquez sur uproject)
Write-Host "Ouverture d'Unreal Engine..."
Start-Process -FilePath $uproject

# Ouvrir VS Code sur le dossier racine
Write-Host "Ouverture de VS Code sur le projet..."
code $PSScriptRoot\.. 2>$null
Write-Host "Démarrage lancé."
