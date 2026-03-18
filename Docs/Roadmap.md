ROADMAP DÉTAILLÉE — “ZORB EARTH RUN” (v1.1)

## Suivi d'avancement (mis à jour le 2026-03-18)

### Mise à jour session 2026-03-18 (tests comparatifs + équilibrage)
- [x] Campagne comparative full exécutée sur 8 scénarios (5 air + 3 ground vallée) et 4 presets.
- [x] Pipeline de benchmark stabilisé: sélection du dernier run par couple preset/scenario + synthèse objective.
- [x] Scénarios vallée ajoutés depuis le même point de départ/orientation:
	- `ground_cp_run`
	- `ground_valley_sprint`
	- `ground_valley_switchback`
- [x] Correctif critique automation: les inputs scénarisés ne sont plus écrasés par l'input joueur pendant les runs.
- [x] Correctif scénario ground: suppression des surchauffes critiques pendant benchmark (plus de respawn parasite).
- [x] Journalisation des résultats de campagne consolidée dans `Docs/BenchmarkLog.md`.

### Décision de session
- [x] Reporter le tuning final des presets à une itération dédiée.
- [x] Conserver l'objectif "pas de meilleur preset global", avec différenciation par type de terrain.

### Future étape prioritaire — Capacités spéciales par profil
**Objectif**: renforcer l'identité de chaque preset via une capacité active/passive avec contrepartie claire.

Backlog v1 (à spécifier et implémenter plus tard):
- `Classic`: capacité de régularité/contrôle (ex: réduction ponctuelle des pertes en virage).
- `Agile`: capacité de relance/maniabilité (ex: impulsion courte orientée trajectoire).
- `Heavy`: capacité thermique/stabilité (ex: réduction temporaire du gain de chaleur).
- `Wild`: capacité burst risquée (ex: double boost court avec coût thermique élevé).

Contraintes d'équilibrage prévues:
- fenêtre d'activation + cooldown explicite,
- trade-off mesurable (vitesse, chaleur, énergie ou contrôle),
- validation obligatoire sur matrice multi-terrains (pas seulement un scénario).

### Révision de tâche prioritaire — Génération Heightmap -> Mesh -> Map jouable
**Constat**:
- Le pipeline existe en prototype, mais la tâche reste trop large pour piloter la qualité terrain de manière fiable.
- Les étapes techniques, les sorties attendues et les critères de validation doivent être explicitement séparés.

**Objectif révisé**:
- Passer d'un prototype "ça marche" à un pipeline reproductible, mesurable, et exploitable en production de pistes test.

**Sources de heightmap (décision produit)**:
Deux sources sont retenues, avec usages complémentaires:

1. **Heightmaps réelles (DB/API DEM)**
- Avantages: crédibilité géographique, variété naturelle des reliefs, valeur produit "piste du monde réel".
- Risques: qualité hétérogène des données, latence réseau/API, zones parfois peu ludiques sans post-traitement.
- Usage cible: contenu live, mode exploration, pistes partagées par coordonnées.

2. **Heightmaps procédurales artificielles**
- Avantages: contrôle total du fun design, reproductibilité parfaite, génération rapide pour benchmark et tuning.
- Risques: moins d'authenticité géographique, risque de patterns répétitifs si générateur pauvre.
- Usage cible: scénarios de test, calibration gameplay, contenu "curated" orienté fun.

**Décision recommandée**:
- Approche **hybride**: utiliser le procédural pour la boucle d'itération gameplay + utiliser le réel pour la promesse produit et la rejouabilité.
- Exiger une même étape de "qualification jouabilité" avant qu'une map (réelle ou procédurale) entre dans la boucle benchmark.

**Découpage v2 (livrables techniques)**:
1. **Ingestion Heightmap**
- Format d'entrée canonique défini (16-bit grayscale PNG + fallback R16).
- Validation automatique des dimensions autorisées (ex: 1009, 2017, 2049).
2. **Conversion Mesh jouable**
- Génération mesh terrain stable (collisions fiables, échelle correcte, orientation validée).
- Contrôle des artefacts majeurs (marches, trous, zones non parcourables).
3. **Habillage gameplay minimal**
- Définition Start/Finish + checkpoints minimum.
- Spawn Zorb valide + test de roulage continu sans blocage critique.
4. **Validation outillée**
- Checklist de run standardisée (spawn, roulage, pente, checkpoint, finish, respawn).
- Export d'un rapport de validation terrain (manuel ou scripté) avant usage tuning.

**Critères d'acceptation (DoD)**:
- Une nouvelle heightmap peut être importée et rendue jouable en moins de 15 minutes.
- Le Zorb peut rouler du start au finish sans rupture majeure de collision.
- Les checkpoints et respawn fonctionnent sur la map générée.
- La map est qualifiée pour entrer dans la boucle de benchmark/tuning.

**Terrain QA Runbook v1 (checklist exécutable)**:
1. **Préparation (2 min)**
- [ ] Build editor OK et map cible ouverte.
- [ ] Heightmap source identifiée (PNG 16-bit ou R16) + dimensions conformes.
- [ ] Paramètres d'import notés (scale XY/Z, orientation).

2. **Import & génération (5 min)**
- [ ] Import sans erreur bloquante.
- [ ] Mesh/landscape généré avec collisions actives.
- [ ] Aucun artefact critique visible (trou, surface inversée, marche invalide).

3. **Habillage gameplay minimal (4 min)**
- [ ] Start défini et spawn Zorb valide.
- [ ] Finish défini et atteignable.
- [ ] Checkpoints minimum posés et ordre validé.

4. **Run de validation (5 min)**
- [ ] Roulage continu start -> finish sans blocage critique.
- [ ] Respawn fonctionne sur au moins 1 checkpoint.
- [ ] Caméra reste lisible sur pentes majeures.
- [ ] Aucun crash/erreur bloquante pendant le run.

5. **Go / No-Go**
- [ ] **GO benchmark** si toutes les cases ci-dessus sont validées.
- [ ] **NO-GO** sinon, avec défaut majeur documenté + action corrective.

6. **Rapport minimal à archiver**
- [ ] Nom map + source heightmap + date.
- [ ] Temps import -> map jouable (objectif <= 15 min).
- [ ] Résultat GO/NO-GO.
- [ ] 3 défauts max observés + priorité (haute/moyenne/faible).

**Priorité**: Haute (bloquant qualité pour tuning Movement/Energy/Profiles).

**Préparation prochaine session**:
- Voir le plan dédié: `Docs/NextSession_ProceduralHeightmaps.md`.

### Phase 0 — Pré-production
- [x] Roadmap définie
- [x] Setup du projet Unreal (base C++ fonctionnelle)
- [x] Structure de dossiers `Content` initialisée (Blueprints, Materials, Meshes, Textures, UI, VFX, Audio)
- [x] Contrôles de base du Zorb validés en jeu (déplacement + caméra)
- [x] Choix final du système de génération de mesh (ProceduralMeshComponent vs plugin) — HeightmapProcessing pipeline
- [x] Format des heightmaps serveur figé — 16-bit grayscale PNG + R16 raw
- [x] Prototype énergie/surchauffe implémenté

### Phase 1 — Prototype jouable
- [x] Terrain issu d'une heightmap réelle — pipeline procédural + map perso importée en conditions réelles
- [x] Zorb contrôlable sur map test
- [x] Gameplay départ/arrivée: trigger d'arrivée fonctionnel (message + logs)
- [x] Checkpoints: backend C++ + test en jeu validé (respawn fonctionnel)
- [x] Timer de course backend (start en BeginPlay, stop au finish)
- [x] HUD minimal C++ validé en jeu (timer + état de course + vitesse)
- [x] Physique arcade de base validée: gravité, roulage réaliste, contrôle caméra-référencé, respawn stable
- [x] Tuning arcade v1 validé en jeu: inertie directionnelle, axes corrigés, puissance maîtrisée, paramètres regroupés en `Zorb Physics` dans le BP
- [x] Système énergie + surchauffe + explosion: prototype C++ implémenté (HUD + overheat critique + respawn), tuning validé
- [x] Organisation des paramètres ajustables: profils structurés (Movement/Energy/Respawn/Feedback) + Project Settings `Zorb Tuning` + override local dans `BP_ZorbPawn`

### En cours: TUNING PHASE 1 (conditions réelles sur map personnalisée)
**Objectif** : affiner les paramètres Movement/Energy/Feedback sur terrain réel.
**Map test**: `Villard` (heightmap source: `Content/Textures/HeightMaps/Villard_1K_HeightMap.png`, resolution 1009, import scale 100 x 100 x 50).
**État validé (2026-03-17, Villard)**:
- Caméra validée pour le prototype (base stable et lisible sur Villard).
- Énergie validée sur règle au sol (gain/perte uniquement au contact du sol).
- Stabilité mouvement validée (plus de tremblement critique ni traversée du landscape sur changements de direction répétés).
- Vitesse baseline validée sur map sans obstacles; retuning vitesse reporté après intégration obstacles/checkpoints/zones de danger.
**Paramètres à valider** :
1. **Movement** : MaxSpeed, DirectionResponse, LateralGripForce (arcade fluide vs technique exigeante).
2. **Energy** : EnergyGainRate/DrainRate/HeatGainRate comparés aux temps de jeu réel (session type).
3. **Feedback** : HeatHaloBlendSpeed, intensités visuelles (halo, flash critique).
4. **Boost** : BoostAccelerationMultiplier, coût énergétique vs temps d'utilisation moyenne.

**Checklist tuning** :
- [ ] Vitesse maximale stable et prévisible en ligne droite
- [ ] Virage serré récupérable sans déséquilibre
- [ ] Surchauffe pour raison gameplay (abus boost), pas hasard
- [ ] Feedback halo progressif réactif
- [ ] Flash critique visible sans saturer

### Préparation outillage tuning (future étape)
1. Créer un écran Admin runtime pour éditer les paramètres critiques (movement/energy/respawn).
2. Ajouter sauvegarde/chargement de presets (prototype, arcade, hardcore).

### Nouvelle piste prioritaire: télémétrie + auto-tuning assisté
**Constat**:
- Le fine tuning manuel atteint ses limites: trop de paramètres, tests hétérogènes, ressenti subjectif, et comparaison difficile entre deux itérations.
- Le comportement cible n'est pas une "physique réelle pure", mais un compromis mesurable entre crédibilité physique, lisibilité gameplay, et fun arcade.

**Objectif produit**:
- Construire un pipeline qui enregistre les runs réels sur `Villard`, calcule des métriques objectives, puis aide à ajuster les paramètres de tuning à partir de scénarios reproductibles.

**Principe retenu**:
- Ne pas chercher d'abord à reproduire une simulation physique parfaite.
- Définir un ensemble de comportements cibles mesurables.
- Comparer le run réel à ces métriques cibles.
- Utiliser l'optimisation offline pour proposer des paramètres candidats, puis valider en jeu.

**Architecture visée**:
1. **Instrumentation Unreal runtime**
- Export CSV/JSON par tick ou sous-échantillonnage fixe.
- Données minimales: temps, position, vitesse 3D, vitesse horizontale, vitesse verticale, grounded/airborne, pente locale, inputs bruts, inputs lissés, boost, énergie, chaleur, paramètres actifs.
2. **Bibliothèque de scénarios standardisés**
- Exemples: saut sans input, freinage à vitesse donnée, maintien A ou D à vitesse donnée, alternance A-D, descente roue libre sur pente, sortie de virage.
- Chaque scénario part d'un état initial contrôlé ou d'une zone de piste connue.
3. **Analyse offline des runs**
- Lecture des CSV/JSON.
- Découpage des runs en segments utiles (air, freinage, virage, roue libre, boost).
- Calcul de scores et d'écarts par rapport aux cibles.
4. **Auto-tuning offline**
- Première passe: grid search / random search sur un petit sous-ensemble de paramètres.
- Deuxième passe (si utile): Bayesian optimization ou CMA-ES.
- Sortie: classement des presets candidats + rapport de métriques.

**Métriques cibles initiales**:
- **Air control**: aucun effet d'input en l'air, perte horizontale faible et contrôlée, accélération verticale due à la gravité uniquement.
- **Braking**: distance d'arrêt mesurable et stable pour une vitesse d'entrée donnée.
- **Turning**: rayon de virage, temps d'entrée, temps de sortie, perte de vitesse limitée.
- **Free roll**: conservation d'inertie cohérente et gain/perte d'énergie conforme au design.
- **Stability**: absence d'oscillation divergente, de tremblement, ou de trajectoires absurdes sous stress input.

**Pré-requis méthodologiques**:
- Fixer une map de référence (`Villard`).
- Fixer des zones ou segments de test.
- Définir des scripts d'input reproductibles.
- Réduire autant que possible la non-déterminisme entre runs.
- Distinguer clairement:
	- validation physique crédible,
	- validation gameplay,
	- validation confort caméra.

**Plan de réalisation (ordre recommandé)**:
1. **Telemetry v1**
- Export runtime de la télémétrie Zorb dans `Saved/Telemetry/`.
- Fichier exploitable hors Unreal, versionné uniquement comme outil local (pas comme donnée de build).
2. **Analyzer v1**
- Script Python offline qui résume un run.
- Extraction de métriques simples: durée, distance, vitesse max, segments airborne, pertes de vitesse, freinage, virages.
3. **Scenario runner v1**
- Définir 3 à 5 scénarios de test de référence.
- Standardiser les entrées et conditions initiales.
4. **Optimizer v1**
- Balayer automatiquement quelques paramètres seulement (ex: `BrakeDeceleration`, `TurnRateScale`, `AirHorizontalDeceleration`).
- Produire un score global et une shortlist de presets.
5. **Preset loop**
- Réinjecter les meilleurs candidats dans les `Zorb Tuning Settings`.
- Revalider en jeu avant tout commit de tuning.

**Découpage technique court terme**:
- Étape A: télémétrie CSV embarquée dans `AZorbPawn`.
- Étape B: analyseur Python sans dépendances externes.
- Étape C: formalisation des scénarios de test.
- Étape D: optimisation semi-automatique.

**Critères de validation de cette piste outillage**:
- Un run de test peut être enregistré et relu hors Unreal.
- Un script peut produire un résumé chiffré stable du run.
- Deux presets différents peuvent être comparés objectivement sur le même scénario.
- L'outil aide réellement la décision de tuning au lieu d'ajouter de la confusion.

🧱 PHASE 0 — Pré‑production (2 à 4 semaines)
Objectif : verrouiller la vision, les systèmes, les outils, et préparer le pipeline technique.
🎯 Livrables
- GCD final
- GFD final
- TDD final
- Roadmap (ce document)
- Choix des outils (Unreal, VS Code, IA associées)
- Setup du dépôt Git
- Setup du projet Unreal (templates, plugins, structure)
🔧 Tâches
- Créer la structure du projet Unreal (folders, naming conventions)
- Choisir le système de génération de mesh (ProceduralMeshComponent ou plugin)
- Définir le format des heightmaps côté serveur
- Créer un premier test de conversion heightmap → mesh
- Définir les paramètres physiques du Zorb (première version)
- Créer un premier test de surchauffe (barre simple + explosion)
✔️ Critères de validation
- Le pipeline heightmap → mesh fonctionne sur une heightmap simple
- Le Zorb roule sur un terrain test
- Le système d’énergie et surchauffe existe en version prototype
- Le projet est stable et versionné

🚀 PHASE 1 — Prototype jouable (4 à 6 semaines)
Objectif : obtenir un prototype fun, minimal, jouable sur une seule piste.
🎯 Livrables
- Terrain généré depuis une heightmap réelle
- Zorb contrôlable
- Physique arcade
- Système d’énergie + surchauffe + explosion
- Checkpoints
- Timer
- HUD minimal
🔧 Tâches
Terrain
- Importer une heightmap réelle (préparée)
- Générer un mesh low-poly
- Ajouter un matériau simple (herbe/roche)
- Ajouter un micro‑relief procédural
Zorb
- Implémenter les forces (gravité, friction, drag)
- Implémenter le drift involontaire
- Implémenter le rebond amplifié
- Implémenter le déséquilibre après choc
Énergie & Surchauffe
- Accumulation en roue libre
- Boost
- Freinage assisté
- Surchauffe (3 niveaux)
- Explosion + respawn
Gameplay
- Départ / arrivée
- Checkpoints
- Timer
- Respawn
UI
- HUD minimal : vitesse, énergie, surchauffe, timer
✔️ Critères de validation
- Le jeu est jouable du début à la fin
- Le Zorb est fun à contrôler
- La surchauffe explosive fonctionne
- Le joueur ne peut pas descendre tout droit sans risque
- Le prototype tourne à 60 FPS sur mobile milieu de gamme

🧩 PHASE 2 — Vertical Slice (6 à 10 semaines)
Objectif : démontrer la vision complète du jeu sur une seule piste, avec DA, VFX, audio, et gameplay complet.
🎯 Livrables
- Une piste complète générée automatiquement
- Direction artistique stylisée
- VFX (vitesse, collisions, surchauffe, explosion)
- Audio (roulement, boost, explosion)
- Obstacles procéduraux
- Zones spéciales (glue, surfaces glissantes, vents)
- Mode Time Attack complet
- Ghosts (asynchrones)
- Menu principal
🔧 Tâches
Terrain
- Génération automatique du départ/arrivée
- Placement procédural d’obstacles
- Placement de zones spéciales
- Amélioration du mesh (LOD, collisions)
Gameplay
- Mode Time Attack complet
- Ghost du meilleur temps
- Système de trajectoire idéale (bonus d’énergie)
VFX / Audio
- Traînée de vitesse
- Effets de surchauffe
- Explosion cartoon
- Sons dynamiques
UI
- Menu principal
- Sélecteur de piste (coordonnée / seed)
- Écran de résultats
✔️ Critères de validation
- Le jeu ressemble à un vrai jeu
- Le fun est présent
- Le pipeline heightmap → piste → gameplay est fluide
- Le vertical slice peut être montré à un investisseur

⚔️ PHASE 3 — Multijoueur (6 à 12 semaines)
Objectif : implémenter les interactions multijoueurs et stabiliser le réseau.
🎯 Livrables
- Multijoueur synchrone (4–8 joueurs)
- Interactions offensives (bump, glue, vortex, choc énergétique)
- Classements en temps réel
- Lobby simple
🔧 Tâches
Réseau
- Réplication du Zorb
- Réplication des états (boost, frein, surchauffe)
- Réplication des collisions
- Optimisation mobile (compression, interpolation)
Interactions offensives
- Bump latéral
- Choc énergétique
- Glue automatique
- Vortex
- Boost partagé
UI
- Position en course
- Classement live
- Lobby / matchmaking
✔️ Critères de validation
- Une course multijoueur fonctionne sans lag majeur
- Les interactions sont fun et lisibles
- Le jeu reste stable sur mobile

🌍 PHASE 4 — Génération globale & contenu (8 à 12 semaines)
Objectif : permettre au joueur de générer n’importe quelle piste du monde.
🎯 Livrables
- Génération de pistes via coordonnées GPS
- Génération via seed
- Piste du jour
- Système de cache local des heightmaps
- Optimisation du streaming
🔧 Tâches
- API serveur pour récupérer heightmaps
- Compression/décompression optimisée
- Gestion du cache local
- Sécurité (limiter les requêtes)
- Optimisation du mesh runtime
- Détection automatique des zones injouables
✔️ Critères de validation
- Le joueur peut entrer une coordonnée et jouer en 5–10 secondes
- Le système est stable et rapide
- Les pistes générées sont fun et jouables

🎨 PHASE 5 — Contenu, skins, progression (6 à 10 semaines)
Objectif : ajouter la méta, la progression, les récompenses.
🎯 Livrables
- Skins de Zorb
- Traces au sol
- Effets de boost
- Succès
- Progression du joueur
- Saisons
🔧 Tâches
- Système de skins (matériaux dynamiques)
- Système de progression
- Succès / défis
- Boutique (si monétisation)
- Système de saisons
✔️ Critères de validation
- Le joueur a une raison de revenir
- Le contenu est cohérent et léger pour mobile

🧪 PHASE 6 — Alpha (4 à 6 semaines)
Objectif : tout est jouable, mais pas encore optimisé.
🎯 Livrables
- Tous les modes
- Toutes les features
- Multijoueur stable
- Génération globale fonctionnelle
🔧 Tâches
- Fix bugs majeurs
- Ajustements gameplay
- Ajustements physiques
- Ajustements surchauffe / énergie
- Tests sur plusieurs devices
✔️ Critères de validation
- Le jeu est complet
- Le fun est constant
- Le jeu tourne sur 80% des devices ciblés

🧹 PHASE 7 — Beta (4 à 8 semaines)
Objectif : optimisation, polish, stabilité.
🎯 Livrables
- VFX finalisés
- Audio finalisé
- UI finalisée
- Optimisation mobile
- Optimisation réseau
- Correction des bugs
🔧 Tâches
- Optimisation CPU/GPU
- Optimisation du mesh terrain
- Optimisation physique
- Optimisation réseau
- Tests intensifs
✔️ Critères de validation
- Le jeu est fluide
- Le jeu est stable
- Le jeu est beau
- Le jeu est prêt pour release

🚀 PHASE 8 — Release (2 semaines)
Objectif : publier le jeu.
🎯 Livrables
- Build iOS
- Build Android
- Build PC
- Trailer
- Page store
✔️ Critères de validation
- Le jeu est disponible
- Le pipeline de mise à jour est prêt

🔄 PHASE 9 — Post‑release (continu)
Objectif : améliorer, équilibrer, ajouter du contenu.
🎯 Livrables
- Nouvelles zones
- Nouveaux skins
- Nouveaux modes
- Équilibrage
- Événements saisonniers


Total : 38 à 70 semaines, selon équipe, IA, et automatisation.
