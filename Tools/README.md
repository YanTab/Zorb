# Outils du projet Zorb Earth Run

## HeightmapProcessing
- Scripts pour normaliser/crop/générer preview des heightmaps.

### Exécution
```powershell
python Tools\HeightmapProcessing\generate_heightmap.py --output Tools\HeightmapProcessing\output\zorb_test_hills_2049.png --size 2049 --seed 42
python Tools\HeightmapProcessing\normalize_heightmap.py --input Tools\HeightmapProcessing\output\zorb_test_hills_2049.png --output Tools\HeightmapProcessing\output\zorb_test_hills_2049_norm.png
python Tools\HeightmapProcessing\generate_preview.py --input Tools\HeightmapProcessing\output\zorb_test_hills_2049_norm.png --output Tools\HeightmapProcessing\output\zorb_test_hills_preview.png
python Tools\HeightmapProcessing\crop_heightmap.py --input Tools\HeightmapProcessing\output\zorb_test_hills_2049_norm.png --output Tools\HeightmapProcessing\output\zorb_test_hills_1009.png --size 1009
```

### Scripts disponibles
- `generate_heightmap.py`: génère une heightmap procédurale 16-bit (collines, vallées, bosses).
- `normalize_heightmap.py`: normalise une heightmap sur toute la dynamique 16-bit.
- `crop_heightmap.py`: crop centré et resize carré (ex: 1009, 2017, 2049).
- `generate_preview.py`: crée une preview couleur lisible pour contrôle visuel.
- `export_r16.py`: convertit une image heightmap 16-bit en `.r16` brut (format Unreal très fiable).

### Import Unreal (Landscape)
- Dans Unreal: `Modes > Landscape > Import from File`.
- Fichier test versionné: `Tools/HeightmapProcessing/output/zorb_test_hills_1009.png`.
- Pour plus grand relief: générer ensuite une version `2049` avec le script.
- Garder un format 16-bit grayscale (`PNG I;16`) pour éviter le banding.
- Ajuster surtout `Z Scale` selon le relief voulu (commencer à 120-220).
- Si un PNG refuse l'import: convertir en `.r16` puis importer ce fichier (`Width/Height`: 1009 x 1009).

## Ajouter des outils
- Stocker les nouveaux scripts dans `Tools/`.
- Documenter l'usage ici.
