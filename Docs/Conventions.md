# Conventions de projet Zorb Earth Run

## Conventions de fichiers
- Docs : `GCD.md`, `GFD.md`, `TDD.md`, `Roadmap.md`.
- Code Unreal C++ : `Source/ZorbEarthRun/<module>/<fichier>.h|.cpp`.
- Blueprint/Content : dossiers séparés dans `UnrealProject/Content`.

## Branching Git
- `main` : code stable.
- `feature/<something>` : nouvelles features.
- `fix/<issue>` : correctifs.
- `hotfix/<issue>` : corrections critiques.

## Nom de commits
- `[feature]` / `[fix]` / `[docs]` + phrase courte.

## Setup
- Ouvrir `UnrealProject/ZorbEarthRun.uproject`.
- Vérifier Unreal Engine version.
- Installer dependencies Python pour outils dans `Tools/HeightmapProcessing`.
