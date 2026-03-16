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
- Convention : `type(scope): description` (style Conventional Commits)
- Types autorisés : `feat`, `fix`, `docs`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`
- Description : impératif, courte, en minuscules, sans point final
- Scope : optionnel mais recommandé (ex: `zorbpawn`, `hud`, `tuning`, `docs`)

Exemples:
- `feat(zorbpawn): add boost energy drain and heat gain`
- `fix(input): correct move axis inversion on camera-relative controls`
- `docs(roadmap): mark overheat feedback phase as validated`

## Règles de commit
- 1 commit = 1 intention fonctionnelle
- Ne pas mélanger refactor large + feature dans le même commit
- Committer sur une branche de travail puis merger vers `main`
- Push fréquent pour éviter les divergences locales

## Workflow rapide
1. `git checkout -b feature/<topic>`
2. coder + tester
3. `git add -A`
4. `git commit`
5. `git push -u origin feature/<topic>`
6. ouvrir PR vers `main`

## Setup
- Ouvrir `UnrealProject/ZorbEarthRun.uproject`.
- Vérifier Unreal Engine version.
- Installer dependencies Python pour outils dans `Tools/HeightmapProcessing`.
