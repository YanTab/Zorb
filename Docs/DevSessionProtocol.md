# Protocole de session dev-build-test

Ce protocole standardise le demarrage de chaque session pour reduire les oublis et gagner du temps.

## Objectif
- Avoir un environnement coherent a chaque reprise.
- Verifier rapidement que le projet est buildable.
- Lancer le test en jeu avec la derniere version C++.

## Routine recommandee
1. Ouvrir un terminal a la racine du repo.
2. Lancer le batch: `Tools\session_bootstrap.bat`.
3. Attendre la fin du build.
4. Ouvrir Unreal Editor et tester sur la map de reference `Villard`.
5. En fin de session: commit avec convention `type(scope): description`.

## Ce que fait le bootstrap
- Verifie la presence de `ZorbEarthRun.uproject` et `ZorbEarthRun.sln`.
- Detecte automatiquement `Build.bat` Unreal (UE 5.5 cible en priorite).
- Verifie/prepare `.venv` Python pour les outils de `Tools/HeightmapProcessing`.
- Installe les dependances Python necessaires (`numpy`, `pillow`, ou requirements local).
- Affiche un resume Git (branche + statut court).
- Compile `ZorbEarthRunEditor Win64 Development` via Unreal Build Tool.

## Commandes utiles
- Bootstrap complet (defaut):
  - `Tools\session_bootstrap.bat`
- Bootstrap sans build:
  - `powershell -ExecutionPolicy Bypass -File .\Tools\session_bootstrap.ps1 -SkipBuild`
- Bootstrap + ouverture automatique de l editor apres build:
  - `powershell -ExecutionPolicy Bypass -File .\Tools\session_bootstrap.ps1 -LaunchEditor`

## Notes
- Le build passe par `Build.bat` (UBT), pas par l interface Visual Studio.
- Visual Studio peut etre ouvert en parallele, mais n est pas requis pour compiler.
- Si plusieurs versions UE sont installees, le script priorise d abord la version associee au `.uproject`.
