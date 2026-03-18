# Procedural Landscape Patch Workflow

## Contexte
Les essais de génération de heightmaps globales par scripts Python ont montré une limite claire pour Zorb:
- même avec contrôle de pente et de vallées, le relief résultant reste trop bruité ou trop instable,
- la structure gameplay utile (flow, lignes, recoverability, lisibilité) n'émerge pas de façon suffisamment fiable,
- le coût d'itération pour corriger une mauvaise map reste trop élevé.

Conclusion de session:
- ne plus considérer la génération de heightmap globale comme approche principale pour la production de maps benchmarkables,
- investiguer une approche Unreal native basée sur `Landscape Patch`, pilotée de façon procédurale et reproductible.

## Intention produit
L'objectif n'est pas de "générer un relief plausible".
L'objectif est de générer automatiquement un **parcours jouable** avec:
- une pente générale lisible,
- une vallée principale roulable,
- des lignes secondaires éventuelles,
- des zones risk/reward contrôlées,
- une surface assez propre pour tester le gameplay Zorb sans pollution par du bruit topographique inutile.

## Principe général
Le pipeline visé n'est plus:

`Seed -> Heightmap globale -> Import -> QA`

Le pipeline visé devient:

`Seed + Profile -> TerrainRecipe -> PatchStack -> Landscape -> Auto course -> QA -> GO/NO-GO`

La sortie canonique du générateur n'est donc plus une image PNG, mais une **recette de terrain**.

## Idée centrale: TerrainRecipe
Une `TerrainRecipe` est une description paramétrique et reproductible d'un terrain de test.

Contenu recommandé:
- `seed`
- `profile_name`
- `global_slope_angle`
- `global_slope_strength`
- `main_corridor_width`
- `main_corridor_depth`
- `main_corridor_curvature`
- `branch_count`
- `branch_probability`
- `branch_width_range`
- `branch_depth_range`
- `risk_reward_zone_count`
- `smoothing_pass_strength`
- `playability_limits`

Chaque `TerrainRecipe` doit être sérialisable en JSON pour archivage et replay exact.

## Profils cibles

### 1. Flow
Objectif:
- grande vallée principale,
- pente lisible,
- trajectoire naturelle stable,
- peu d'ambiguïté.

Caractéristiques:
- 1 couloir principal,
- faible courbure,
- peu ou pas de branches,
- transitions douces,
- lissage fort.

### 2. Switchback
Objectif:
- une lecture claire de descente mais plus technique,
- enchaînements directionnels marqués.

Caractéristiques:
- 1 couloir principal,
- courbure modérée à forte,
- quelques basculements latéraux,
- sorties de virage lissées,
- sidebanks plus affirmés.

### 3. RiskReward
Objectif:
- ligne principale sûre,
- lignes secondaires plus rapides ou plus risquées,
- vraie décision de trajectoire.

Caractéristiques:
- 1 tronc principal,
- 1 à 2 branches jouables,
- relief latéral structurant,
- quelques événements locaux plus marqués.

## PatchStack recommandé
Le terrain doit être construit par couches ordonnées.

### Patch 1. Base Slope
But:
- imposer une pente générale d'un bout à l'autre de la carte.

Variables:
- angle,
- force,
- éventuelle composante transversale faible.

### Patch 2. Main Corridor Carve
But:
- créer la vallée principale roulable.

Variables:
- largeur,
- profondeur,
- courbure,
- point d'entrée,
- point de sortie.

### Patch 3. Secondary Branches
But:
- ajouter des lignes alternatives compatibles avec le flow principal.

Variables:
- nombre de branches,
- position de raccordement,
- largeur,
- agressivité,
- niveau de risque.

### Patch 4. Sidebanks / Ridges
But:
- encadrer visuellement les couloirs,
- donner de la lisibilité,
- créer des choix de trajectoires.

Variables:
- intensité,
- largeur,
- distance au couloir.

### Patch 5. Gameplay Smoothing
But:
- enlever les ruptures de pente inutiles,
- lisser la ligne roulable,
- fiabiliser la caméra et les collisions.

Zones concernées:
- tronc principal,
- sorties de virage,
- transitions entre patches,
- zones de réception.

### Patch 6. Local Events
But:
- injecter des signatures de gameplay localisées.

Exemples:
- compression,
- léger drop,
- chicane naturelle,
- relance,
- section plus engagée.

Important:
- ces patches doivent rester rares et lisibles,
- ils ne doivent jamais casser la continuité du run.

## Ce qui doit être procédural / aléatoire
Le système doit randomiser sous contraintes:
- orientation de la pente globale dans une plage donnée,
- largeur/profondeur du tronc principal,
- courbure du tronc principal,
- nombre et position des branches,
- intensité de certains événements locaux,
- placement des zones risk/reward.

## Ce qui ne doit pas être laissé libre
Doit rester borné par des règles dures:
- pente locale max,
- largeur minimale du couloir principal,
- continuité start -> finish,
- absence de bosses haute fréquence,
- recoverability minimale,
- lisibilité caméra minimale.

## Auto-évaluation attendue
Une fois le Landscape généré, le pipeline doit pouvoir enchaîner automatiquement:

1. auto-placement Start / Checkpoints / Finish,
2. génération éventuelle d'un scénario de run simple,
3. calcul de métriques de qualification,
4. verdict GO/NO-GO,
5. archivage de la `TerrainRecipe` et du score.

## Workflow de session recommandé

### Etape A. Vérifier le plugin
- confirmer que `Landscape Patch` est disponible et exploitable dans la version UE du projet,
- confirmer l'usage avec Edit Layers.

### Etape B. Créer une map template
- Landscape vide simple,
- dimensions fixes,
- paramètres d'import/échelle de référence,
- couche(s) prêtes pour les patches.

### Etape C. Définir la structure `TerrainRecipe v1`
- JSON de recette,
- 3 presets de profil,
- seed + paramètres archivés.

### Etape D. Générer une PatchStack automatiquement
- à partir d'une `TerrainRecipe`,
- avec ordre stable des patches.

### Etape E. Auto-placer le parcours
- PlayerStart,
- checkpoints,
- finish.

### Etape F. Qualifier
- run rapide,
- score Fun Terrain Metrics v1,
- GO/NO-GO.

## Première implémentation recommandée
Pour la prochaine session, viser **v1 minimal viable**:

1. 1 patch pente globale
2. 1 patch vallée principale
3. 1 patch smoothing
4. auto-placement start/finish

Puis seulement ensuite:

5. checkpoints auto
6. branches secondaires
7. zones risk/reward
8. génération multi-profils complète

## Risques connus
- API patch potentiellement expérimentale selon version UE,
- automatisation éditeur peut nécessiter C++/Blueprint Editor Utility plutôt que Python seul,
- risque de surinvestir dans des détails de géométrie avant d'avoir validé le tronc principal jouable.

## Décision de continuité
La prochaine session terrain doit tester en priorité **la faisabilité du pipeline procédural Landscape Patch**, pas produire immédiatement une grande quantité de maps.

Critère de succès minimal:
- générer une descente jouable simple, pilotée par seed/recipe,
- avec pente lisible et vallée principale stable,
- en moins de 15 minutes de génération à map testable.