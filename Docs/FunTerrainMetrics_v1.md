# Fun Terrain Metrics v1

## Objectif
Qualifier une map terrain pour le benchmark Zorb avec des criteres objectifs, rapides, et reproductibles.

## Portee v1
- Source terrain: heightmap 16-bit (PNG ou R16), import Landscape Unreal.
- Qualification: GO / NO-GO avant entree dans la boucle de benchmark presets/scenarios.
- Delai cible: import -> map jouable <= 15 min.

## Axes de score
Score global sur 100:

- Flow (35)
- Risque / Recompense (25)
- Recoverability (25)
- Lisibilite (15)

Formule:

`ScoreFun = Flow + RiskReward + Recoverability + Readability`

## Metrics detaillees

### 1) Flow (0-35)
- F1 Continuite de roulage (0-15)
  - Mesure: ratio du run start->finish sans arret complet non voulu.
  - Cible GO: >= 0.90
- F2 Variabilite de pente utile (0-10)
  - Mesure: presence de sections descendantes, neutres et montantes observables en run.
  - Cible GO: les 3 types presents.
- F3 Ruptures penalites (0-10)
  - Mesure: nombre de blocages critiques (micro falaises, marches invalides, trous).
  - Cible GO: 0 blocage critique.

### 2) Risque / Recompense (0-25)
- R1 Lignes a choix (0-10)
  - Mesure: au moins 2 trajectoires plausibles sur 2 zones cles.
  - Cible GO: oui.
- R2 Valeur du risque (0-10)
  - Mesure: une ligne plus risquee donne un gain clair (temps/vitesse) sur au moins 1 segment.
  - Cible GO: gain mesurable sur au moins 1 segment.
- R3 Penalite lisible (0-5)
  - Mesure: la punition d'une erreur est comprehensible et non aleatoire.
  - Cible GO: oui.

### 3) Recoverability (0-25)
- C1 Respawn utile (0-10)
  - Mesure: respawn checkpoint permet de reprendre sans blocage.
  - Cible GO: 100% des checkpoints testes valides.
- C2 Recuperation post-erreur (0-10)
  - Mesure: apres sortie de ligne ou perte de vitesse, retour au flow en < 8 s.
  - Cible GO: oui sur >= 2 cas.
- C3 Tolerance aux erreurs mineures (0-5)
  - Mesure: une petite faute ne ruine pas tout le run.
  - Cible GO: oui.

### 4) Lisibilite (0-15)
- L1 Lecture camera (0-8)
  - Mesure: lisibilite correcte des pentes et appuis dans les sections majeures.
  - Cible GO: pas de perte de lecture critique.
- L2 Lecture visuelle terrain (0-7)
  - Mesure: crêtes, couloirs et risques perceptibles sans ambiguite.
  - Cible GO: oui.

## Gate GO / NO-GO
Une map est GO benchmark si:

1. Aucun blocage critique (collision/trou/marche invalide).
2. Start, finish et checkpoints fonctionnels.
3. Temps import -> map jouable <= 15 min.
4. ScoreFun >= 70/100.
5. Aucun axe majeur en echec severe:
   - Flow >= 22/35
   - Recoverability >= 15/25

Sinon: NO-GO avec action corrective documentee.

## Format de rapport minimal
- MapName
- HeightmapSource
- Seed
- ImportScaleXY
- ImportScaleZ
- Result (GO/NO-GO)
- Score breakdown: Flow, RiskReward, Recoverability, Readability, Total
- Defauts majeurs (max 3)
- Action corrective proposee

## Notes v1
- v1 privilegie la vitesse de decision et la reproductibilite.
- Les seuils seront recalibres apres 5 a 10 runs compares.