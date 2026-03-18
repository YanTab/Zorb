# Terrain QA Checklist v2

## But
Valider rapidement si une map terrain est admissible au benchmark presets/scenarios.

## Entrees requises
- Heightmap source (PNG 16-bit ou R16)
- Nom map
- Seed et parametres de generation
- Parametres import Landscape (XY/Z scale, orientation)

## Checklist executable (15 min cible)

### 1. Preparation (2 min)
- [ ] Build editor OK
- [ ] Map cible ouverte
- [ ] Heightmap et dimensions valides (1009, 2017, 2049)
- [ ] Parametres import notes

### 2. Import et generation terrain (5 min)
- [ ] Import sans erreur bloquante
- [ ] Landscape genere avec collisions actives
- [ ] Aucun trou critique
- [ ] Aucune marche invalide evidente
- [ ] Orientation coherente avec l'intention de parcours

### 3. Habillage gameplay minimal (4 min)
- [ ] Start place et spawn Zorb valide
- [ ] Finish place et atteignable
- [ ] Checkpoints minimum poses et ordre valide

### 4. Run de validation (5 min)
- [ ] Roulage continu start -> finish sans blocage critique
- [ ] Respawn teste sur au moins 1 checkpoint
- [ ] Camera lisible sur sections de pente majeures
- [ ] Aucun crash/erreur bloquante pendant le run

### 5. Scoring v1 (2 min)
- [ ] Score Flow (/35)
- [ ] Score RiskReward (/25)
- [ ] Score Recoverability (/25)
- [ ] Score Readability (/15)
- [ ] Score total >= 70/100

## Decision
- GO benchmark si toutes les cases critiques sont valides + score seuil atteint.
- NO-GO sinon, avec defaut majeur et action corrective.

## Rapport a archiver
- Date
- MapName
- Source + seed
- ImportScale XY/Z
- Temps import -> map jouable
- GO/NO-GO
- 3 defauts max + priorite (haute/moyenne/faible)