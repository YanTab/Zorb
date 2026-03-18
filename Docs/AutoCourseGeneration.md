# Auto Course Generation (Start / Checkpoints / Finish)

## Objectif
Supprimer la preparation manuelle repetitve des parcours sur chaque map procedurale candidate.

## Pipeline
1. Generer les heightmaps candidates.
2. Generer automatiquement les marqueurs de parcours (JSON + script Unreal Python).
3. Executer le script Unreal sur la map ouverte pour placer Start/Checkpoints/Finish.

## Commande batch (3 maps candidates v1)

```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\HeightmapProcessing\generate_course_candidates_v1.ps1
```

Sorties dans `Tools/HeightmapProcessing/output/candidates_v1/`:
- `valley_flow_course.json`
- `valley_flow_apply_course.py`
- `valley_switchback_course.json`
- `valley_switchback_apply_course.py`
- `valley_risk_reward_course.json`
- `valley_risk_reward_apply_course.py`

## Application dans Unreal
Prerequis:
- Plugin Python Unreal actif.
- Map cible ouverte dans l editor.
- Blueprints disponibles:
  - `/Game/Blueprints/BP_CheckPointTrigger`
  - `/Game/Blueprints/BP_FinishTrigger`

Dans la console Python Unreal, executer par exemple:

```python
exec(open(r"D:/Game_Project/ZorbEarthRun/Tools/HeightmapProcessing/output/candidates_v1/valley_flow_apply_course.py", "r", encoding="utf-8").read())
```

Effet:
- Deplace ou cree un `PlayerStart` (`AUTO_PlayerStart`).
- Supprime les anciens marqueurs auto (`AUTO_CP_*`, `AUTO_Finish`).
- Place les checkpoints et l arrivee automatiquement.

## Parametres importants
Le generateur utilise par defaut:
- `XYScale=100`
- `ZScale=50`
- `CheckpointCount=5`

Si la map importee utilise une autre echelle, regenera les fichiers avec:

```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\HeightmapProcessing\generate_course_candidates_v1.ps1 -XYScale 100 -ZScale 80 -CheckpointCount 6
```

## Limites v1
- Les positions sont heuristiques (basees height/slope) et doivent etre valides rapidement en jeu.
- La logique d ordre des checkpoints est spatiale (ordre de placement), pas un graphe de navigation avance.

## Usage recommande
- Utiliser l auto-generation pour 80-90% du setup.
- Faire une passe QA courte (2-5 min) pour detecter un checkpoint mal place.