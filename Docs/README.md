# Documentation du projet Zorb Earth Run

Ce dossier contient les documents de conception, de spécification et de suivi du projet.

## Documents principaux
- `GCD.md` - Vision produit, piliers gameplay, identité du jeu.
- `GFD.md` - Spécification fonctionnelle des mécaniques et systèmes de jeu.
- `TDD.md` - Architecture technique et état d'implémentation.
- `Roadmap.md` - Plan de production et priorités de développement.

## Documents de suivi
- `BenchmarkLog.md` - Historique des campagnes de test, résultats et conclusions.
- `SessionSummary_2026-03-17.md` - Compte-rendu historique de session (référence).
- `ProceduralLandscapePatchWorkflow.md` - Workflow cible pour la génération procédurale de terrains via Landscape Patch.
- `Conventions.md` - Conventions de nommage, workflow et discipline projet.
- `DevSessionProtocol.md` - Protocole de session dev/build/test.

## Règle de cohérence documentaire
- Toute décision produit majeure doit être reportée dans `GCD.md` et `GFD.md`.
- Toute décision technique impactant code/test doit être reportée dans `TDD.md`.
- Toute priorité et jalon doivent être reportés dans `Roadmap.md`.
- Toute campagne de mesure/tuning doit être archivée dans `BenchmarkLog.md`.

## Processus de mise à jour
- Mettre à jour la version/date dans les docs impactés.
- Conserver un changelog implicite via Git (commits atomiques par bloc de décisions).
- Vérifier l'alignement terminologique: profil, preset, scénario, benchmark, capacité spéciale.
