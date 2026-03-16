# Zorb Earth Run

Projet Unreal : Zorb Earth Run.

## Structure du dépôt
- `Docs/` : documentation de conception et roadmap.
- `AI/` : prompts, outputs, logs IA.
- `Source/` : code C++ du projet.
- `Content/` : assets Unreal.
- `Config/` : configurations d’Unreal.
- `Docs/` : documentation de conception.
- `AI/` : prompts / outputs / logs IA.
- `Tools/` : scripts utilitaires (heightmap, setup/start).

## Version Unreal
Le projet est configuré pour **Unreal Engine 5.5** (EngineAssociation 5.5 dans `ZorbEarthRun.uproject`).

## Comment démarrer
1. Ouvrir `ZorbEarthRun.uproject` dans Unreal Engine 5.5.
2. Lire les documents dans `Docs/` pour la vision / features / design.
3. Ajouter votre branche / workflow Git et commits avec messages clairs.
4. Exécuter `.	ools\setup.ps1` pour initialiser l’environnement Python de traitement de heightmap.
5. Exécuter `.	ools\start.ps1` pour ouvrir Unreal + VS Code.
