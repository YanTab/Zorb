# Next Session Plan — Procedural Heightmaps

## Thème
Construire un pipeline procédural orienté fun gameplay pour Zorb.

## Pivot validé en fin de session précédente
- Les essais de génération de heightmaps globales par scripts Python n'ont pas donné de terrains suffisamment jouables.
- Nouvelle piste prioritaire à investiguer: génération procédurale Unreal via `Landscape Patch`, pilotée par `seed + recipe + patch stack`.
- Référence de continuité: `Docs/ProceduralLandscapePatchWorkflow.md`.

## Question centrale
- Qu'est-ce qu'un terrain fun à jouer avec un Zorb ?
- Comment le générer de manière reproductible et mesurable ?

## Objectifs de session
1. Vérifier la faisabilité technique de `Landscape Patch` dans la version Unreal du projet.
2. Définir la structure `TerrainRecipe v1` (seed + profil + paramètres de patchs).
3. Spécifier un pipeline procédural v1: `recipe -> patch stack -> landscape -> qualification -> go/no-go`.
4. Produire un premier terrain simple jouable (pente globale + vallée principale + smoothing).

## Sorties attendues
- Spécification "Fun Terrain Metrics v1".
- Checklist QA terrain v2 (si besoin extension de celle de la roadmap).
- Plan d'implémentation en tâches courtes (tooling + import Unreal).
- Document de workflow procédural Landscape Patch v1.
- Validation d'un premier prototype de descente jouable généré par recipe.

## Contraintes
- Garder compatibilité avec import Unreal actuel (PNG 16-bit / R16).
- Maintenir temps "heightmap -> map jouable" <= 15 min.
- Préserver la reproductibilité (seed et paramètres enregistrés).

## Inputs nécessaires en début de session
- État actuel du pipeline HeightmapProcessing.
- Paramètres d'import Unreal utilisés (XY/Z scale, orientation).
- Exemples de runs sur maps actuelles (si disponibles).
- Disponibilité effective du plugin `Landscape Patch` et compatibilité avec Edit Layers.

## Backlog priorisé (proposition)
1. Vérifier/activer `Landscape Patch` sur la map template cible.
2. Définir `TerrainRecipe v1` sérialisable.
3. Générer automatiquement une patch stack minimale (slope + main corridor + smoothing).
4. Valider un premier terrain GO/NO-GO avec le runbook QA terrain.

## Critères de succès de la session
- Une descente simple pilotée par seed/recipe peut être générée dans Unreal avec pente lisible et vallée principale stable.
- La map générée peut être qualifiée GO/NO-GO avec critères objectifs.
- Le pipeline `recipe -> patch stack -> landscape` est documenté et reproductible pour l'équipe.

## Commandes utiles (rappel)
- Lancer génération outils: voir Tools/HeightmapProcessing/*.py
- Runner scénarios: powershell -ExecutionPolicy Bypass -File .\Tools\run_preset_tests.ps1
- Analyse matrice: python .\Tools\TuningAnalysis\analyze_matrix.py

## Notes de continuité
- Tuning final presets reporté volontairement.
- Capacités spéciales par profil à traiter après stabilisation terrain + tuning de base.
- Ne pas repartir d'abord sur un générateur de heightmap globale bruité.
- Prioriser la structure de parcours et la reproductibilité de la recipe plutôt que la richesse topographique brute.
