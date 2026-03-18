# Outils du projet Zorb Earth Run

## Session dev-build-test
- Script principal: `Tools/session_bootstrap.ps1`
- Batch de lancement: `Tools/session_bootstrap.bat`

### Usage recommandé (chaque début de session)
```powershell
Tools\session_bootstrap.bat
```

### Options utiles
```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\session_bootstrap.ps1 -SkipBuild
powershell -ExecutionPolicy Bypass -File .\Tools\session_bootstrap.ps1 -LaunchEditor
```

Le bootstrap vérifie l'environnement, prépare `.venv`, installe les dépendances Python des tools, affiche l'état Git, et build `ZorbEarthRunEditor` via Unreal Build Tool.

## HeightmapProcessing
- Scripts pour normaliser/crop/générer preview des heightmaps.

### Exécution
```powershell
python Tools\HeightmapProcessing\generate_heightmap.py --output Tools\HeightmapProcessing\output\zorb_test_hills_2049.png --size 2049 --seed 42
python Tools\HeightmapProcessing\generate_heightmap.py --output Tools\HeightmapProcessing\output\zorb_test_flow_2049.png --size 2049 --seed 42 --profile flow --global-slope 0.48 --corridor-count 1 --corridor-width 0.13
python Tools\HeightmapProcessing\generate_heightmap.py --output Tools\HeightmapProcessing\output\zorb_test_switchback_2049.png --size 2049 --seed 42 --profile switchback --global-slope 0.41 --corridor-meander 0.085 --corridor-frequency 2.0
python Tools\HeightmapProcessing\normalize_heightmap.py --input Tools\HeightmapProcessing\output\zorb_test_hills_2049.png --output Tools\HeightmapProcessing\output\zorb_test_hills_2049_norm.png
python Tools\HeightmapProcessing\generate_preview.py --input Tools\HeightmapProcessing\output\zorb_test_hills_2049_norm.png --output Tools\HeightmapProcessing\output\zorb_test_hills_preview.png
python Tools\HeightmapProcessing\crop_heightmap.py --input Tools\HeightmapProcessing\output\zorb_test_hills_2049_norm.png --output Tools\HeightmapProcessing\output\zorb_test_hills_1009.png --size 1009
```

### Scripts disponibles
- `generate_heightmap.py`: génère une heightmap procédurale 16-bit pilotable (pente globale + corridors orientés + limiteur de pente anti-chaos).
- `normalize_heightmap.py`: normalise une heightmap sur toute la dynamique 16-bit.
- `crop_heightmap.py`: crop centré et resize carré (ex: 1009, 2017, 2049).
- `generate_preview.py`: crée une preview couleur lisible pour contrôle visuel.
- `export_r16.py`: convertit une image heightmap 16-bit en `.r16` brut (format Unreal très fiable).
- `generate_valley_candidates_v1.ps1`: génère 3 candidates vallées reproductibles (PNG norm + preview + R16 + manifest).
- `generate_course_markers.py`: génère automatiquement un parcours (start + checkpoints + finish) depuis une heightmap.
- `generate_course_candidates_v1.ps1`: génère les fichiers de parcours auto pour toutes les candidates v1.

### Dataset procedural candidates v1
```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\HeightmapProcessing\generate_valley_candidates_v1.ps1
```

Sorties:
- Dossier: `Tools/HeightmapProcessing/output/candidates_v1/`
- Fichiers par candidate: `*_2049_raw.png`, `*_2049_norm.png`, `*_1009_norm.png`, `*_preview.png`, `*_1009.r16`
- Manifest: `manifest_candidates_v1.json`

### Contrôles du générateur terrain

Le script `generate_heightmap.py` supporte des presets gameplay et des overrides:

- `--profile` : `flow`, `switchback`, `risk_reward`
- `--flow-angle-deg` : orientation de la pente globale
- `--global-slope` : pente générale (0 à 1)
- `--corridor-count` : nombre de couloirs de descente
- `--corridor-depth` / `--corridor-width`
- `--corridor-meander` / `--corridor-frequency`
- `--sidebank-strength`
- `--low-noise`
- `--smoothness`
- `--slope-limit` / `--slope-limit-passes`

Objectif: éviter les champs de pics/creux chaotiques et générer des parcours lisibles orientés run.

### Auto-génération parcours (Start/Checkpoints/Finish)

Générer le layout et le script Unreal pour toutes les candidates:

```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\HeightmapProcessing\generate_course_candidates_v1.ps1
```

Sorties par candidate:
- `*_course.json`: transforms monde (start/checkpoints/finish)
- `*_apply_course.py`: script Python Unreal pour placer les actors dans la map ouverte

Exécution dans Unreal (Python console):
```python
exec(open(r"D:/Game_Project/ZorbEarthRun/Tools/HeightmapProcessing/output/candidates_v1/valley_flow_apply_course.py", "r", encoding="utf-8").read())
```

Le script Unreal:
- déplace (ou crée) un `PlayerStart` auto,
- supprime les anciens marqueurs auto,
- place des `BP_CheckPointTrigger` (`AUTO_CP_XX`),
- place un `BP_FinishTrigger` (`AUTO_Finish`).

### Import Unreal (Landscape)
- Dans Unreal: `Modes > Landscape > Import from File`.
- Fichier test versionné: `Tools/HeightmapProcessing/output/zorb_test_hills_1009.png`.
- Pour plus grand relief: générer ensuite une version `2049` avec le script.
- Garder un format 16-bit grayscale (`PNG I;16`) pour éviter le banding.
- Ajuster surtout `Z Scale` selon le relief voulu (commencer à 120-220).
- Si un PNG refuse l'import: convertir en `.r16` puis importer ce fichier (`Width/Height`: 1009 x 1009).
- Si Unreal demande "Use tiled image path?": répondre `No` pour ces fichiers (ce ne sont pas des tuiles).
- Option robuste: importer `zorb_test_hills_1009.r16` avec `zorb_test_hills_1009.json` présent dans le même dossier.

## Ajouter des outils
- Stocker les nouveaux scripts dans `Tools/`.
- Documenter l'usage ici.

## TuningAnalysis
- Script d'analyse de télémétrie: `Tools/TuningAnalysis/analyze_telemetry.py`
- Script d'analyse matricielle: `Tools/TuningAnalysis/analyze_matrix.py`

### Objectif
- Lire un export CSV produit par le runtime Unreal (`Saved/Telemetry/`).
- Résumer objectivement un run: vitesse, temps en l'air, segments de freinage, segments de virage.

### Exécution
```powershell
python Tools\TuningAnalysis\analyze_telemetry.py Saved\Telemetry\zorb_run_YYYYMMDD_HHMMSS.csv
python Tools\TuningAnalysis\analyze_telemetry.py Saved\Telemetry\zorb_run_YYYYMMDD_HHMMSS.csv --json
python Tools\TuningAnalysis\analyze_matrix.py
```

### Première boucle visée
- Activer la télémétrie dans `Project Settings > Zorb Tuning > Zorb Telemetry`.
- Lancer un run de référence sur `Villard`.
- Lire le CSV exporté dans `Saved/Telemetry/`.
- Comparer les métriques de plusieurs presets sans dépendre uniquement du ressenti.

### Analyse matrice (preset x scenario)
- Convention de nommage attendue: `zorb_{preset}_{scenario}_YYYYMMDD_HHMMSS.csv`.
- `analyze_matrix.py` conserve automatiquement le dernier run pour chaque couple `(preset, scenario)`.
- Sortie: classement par scénario + synthèse globale.

## Runner de campagnes presets/scénarios

- Script: `Tools/run_preset_tests.ps1`

### Modes
- Quick: scénarios ground (checkpoint + vallée sprint + vallée switchback).
- Full: scénarios air + ground (matrice complète).

### Exécution
```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\run_preset_tests.ps1
powershell -ExecutionPolicy Bypass -File .\Tools\run_preset_tests.ps1 -FullMatrix
```

### Consignes d'utilisation
- Le script met à jour le preset et le scénario dans `DefaultGame.ini` avant chaque run.
- Pour chaque lancement Unreal: Play -> attendre fin scénario -> Stop PIE -> fermer Unreal.
- Le script restaure la configuration à la fin de la campagne.
- Purger `Saved/Telemetry/` avant une campagne si besoin de base propre.

## Automation Scenario Runner

### Objectif
- Exécuter un scénario de test par la machine avec position, rotation, vitesse initiale et timeline d'inputs fixées.

### Configuration
- Ouvrir `Project Settings > Zorb Tuning > Zorb Automation`.
- Activer `Enable Scenario Runner`.
- Renseigner `Scenario Name` avec le nom du scénario à lancer.
- Ajouter une entrée dans `Scenarios` avec:
	- `StartLocation`
	- `StartRotation`
	- `InitialLinearVelocity`
	- `DurationSeconds`
	- `InputTimeline`

### Timeline d'inputs
- Chaque keyframe définit l'état voulu à partir de `TimeSeconds`.
- Exemple:
	- `0.00` -> `ForwardInput=1.0`, `RightInput=0.0`, `bBoostActive=false`
	- `1.20` -> `ForwardInput=0.0`, `RightInput=-1.0`, `bBoostActive=false`
	- `2.00` -> `ForwardInput=0.0`, `RightInput=0.0`, `bBoostActive=false`

### Usage recommandé
- Coupler le runner avec `Zorb Telemetry` activé.
- Lancer un seul scénario par Play session.
- Lire ensuite le CSV dans `Saved/Telemetry/` avec `Tools/TuningAnalysis/analyze_telemetry.py`.

## Ghost Replay (comparaison de runs)

### Objectif
- Rejouer un ancien run comme "ghost" pendant un nouveau test automatisé.
- Comparer visuellement les trajectoires et l'avance/retard sur le terrain.

### Configuration
- Ouvrir `Project Settings > Zorb Tuning > Zorb Automation > Ghost`.
- Activer `Enable Ghost Replay`.
- Choisir une source:
	- `Use Latest Telemetry Ghost = true` pour prendre automatiquement le dernier CSV de `Saved/Telemetry/`.
	- ou `Use Latest Telemetry Ghost = false` + `Ghost Telemetry File` pour pointer un CSV précis.
- Ajuster `Ghost Visual Scale` si besoin.

### Notes
- Le ghost est un replay de position (interpolation temporelle), pas une simulation physique.
- Le ghost s'affiche uniquement quand le `Scenario Runner` est actif.
- Le nouveau run continue d'être enregistré en CSV normalement.
