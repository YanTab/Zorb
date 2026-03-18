# Unreal Import Runbook (10 min)

## Objectif
Importer une heightmap candidate et obtenir une map jouable rapidement et de facon reproductible.

## Fichiers cibles (dataset candidates v1)
- `Tools/HeightmapProcessing/output/candidates_v1/*_1009_norm.png`
- `Tools/HeightmapProcessing/output/candidates_v1/*_1009.r16`
- `Tools/HeightmapProcessing/output/candidates_v1/*_preview.png`

## Procedure rapide

1. Ouvrir Unreal Editor et charger la map de travail.
2. Aller dans Landscape > Import from File.
3. Choisir d'abord le fichier PNG 16-bit (`*_1009_norm.png`).
4. Si le PNG est refuse, utiliser le fallback R16 (`*_1009.r16`).
5. Regler la taille selon 1009 x 1009.
6. Utiliser la baseline de session pour tests comparables:
   - Scale X/Y: 100
   - Scale Z: 50 (augmenter a 80-140 si relief trop plat)
7. Valider l'orientation du relief avec la preview.
8. Importer.
9. Poser Start/Finish + checkpoints minimum.
10. Lancer un run court start -> finish.

## Critere de sortie
- Map jouable en <= 15 min.
- Pas de blocage collision critique.
- Checkpoints et respawn fonctionnels.

## Traces a noter
- MapName
- Source heightmap
- Seed
- Scale XY/Z final
- GO/NO-GO