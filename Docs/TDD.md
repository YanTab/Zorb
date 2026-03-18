 Technical Design Document — “ZORB EARTH RUN” (v1.1)

## Addendum état réel (2026-03-18)

Ce document reste la référence d'architecture, avec les précisions suivantes sur l'état implémenté:

- Système de profils (presets) actif en runtime: Classic, Agile, Heavy, Wild.
- Runner de scénarios automatisés actif via `Zorb Automation` (Project Settings).
- Pipeline telemetry actif: export CSV runtime + analyse offline Python.
- Replay ghost actif pour comparaison de trajectoires sur scénarios automatiques.
- Campagnes de benchmark matrice effectuées (air + ground vallée) avec synthèse archivée.

### Système de profils (implémentation)
- Sélection preset exposée dans les settings projet.
- Application via multiplicateurs de comportement (vitesse, turn, grip, freinage, dynamique interne).
- Objectif engineering: maintenir des profils différenciés sans preset dominant global.

### Scenario runner + telemetry (implémentation)
- Scenarios définis dans la config projet (position, rotation, durée, timeline d'inputs).
- Exécution automatique d'un scénario par session de test.
- Telemetry enregistrée dans `Saved/Telemetry/` avec prefix configurable.
- Fin de session telemetry forcée à la fin du scénario pour reproductibilité des durées.

### Correctifs techniques validés pendant la phase benchmark
- Correctif override d'input: les callbacks input joueur ne doivent plus écraser les inputs scenario en automation.
- Correctif scenarios ground benchmark: suppression des respawns parasites dus à la surchauffe critique.
- Validation de runs exploitables via contrôle des colonnes telemetry (inputs non-zero, heat sous seuil critique, durée cohérente).

### Outillage actuel
- `Tools/run_preset_tests.ps1`: orchestration preset x scenario.
- `Tools/TuningAnalysis/analyze_telemetry.py`: lecture d'un run.
- `Tools/TuningAnalysis/analyze_matrix.py`: comparaison matrice (dernier run par couple preset/scenario).

### Etape suivante (planifiée)
- Conserver le tuning fin pour une itération dédiée.
- Introduire des capacités spéciales par profil après stabilisation de l'équilibrage de base.

1. Architecture générale
- Moteur: Unreal Engine (version récente LTS)
- Langage:
- Blueprints pour le gameplay, prototypage rapide
- C++ pour les systèmes critiques (terrain, physique custom, réseau si besoin)
- Couches principales :
- Core Systems: Terrain, Physique, Énergie/Surchauffe, Input
- Game Systems: Modes de jeu, Multijoueur, Progression
- Presentation: UI, VFX, Audio

2. Modules principaux
- TerrainSystem
- ZorbPhysicsSystem
- EnergyOverheatSystem
- TrackGenerationSystem
- GameModeSystem (Time Attack, Score, Survie, Multi)
- MultiplayerSystem
- UI/HUDSystem
- VFX/AudioSystem
Chaque module doit être isolé, testable, et exposer une API claire (BlueprintCallable si possible).

3. Terrain & Heightmaps
3.1. Pipeline heightmap → mesh
- Côté serveur (offline) :
- Récupération DEM (SRTM, etc.)
- Traitement (crop, normalisation, lissage léger)
- Export en heightmap optimisée (ex: PNG 256×256 / 512×512)
- Stockage avec un ID de piste (seed / coordonnée)
- Côté client (runtime) :
- Requête au serveur avec ID / coordonnée
- Téléchargement de la heightmap
- Conversion en mesh low-poly :
- UProceduralMeshComponent ou plugin Runtime Mesh
- Génération des vertices à partir des valeurs de hauteur
- Génération des triangles + UVs
- LODs simples (1–2 niveaux)
3.2. TrackGenerationSystem
- Input : mesh terrain brut
- Étapes :
- Analyse de la pente (scan des hauteurs)
- Détection du point de départ (zone haute, pente raisonnable)
- Détection de l’arrivée (zone basse, accessible)
- Génération d’une ligne idéale (pathfinding simplifié)
- Placement procédural :
- obstacles (rochers, arbres, falaises)
- tremplins
- zones spéciales (glue, surfaces glissantes, vents)
- checkpoints
- Output :
- ATrackData (ScriptStruct ou Actor) contenant :
- Start/End
- Liste de checkpoints
- Liste d’obstacles
- Liste de zones spéciales

4. Physique du Zorb
4.1. Implémentation
- Base : APawn ou ACharacter custom avec :
- USphereComponent (collision)
- UStaticMeshComponent (visuel)
- UProjectileMovementComponent ou physique custom via forces
4.2. Forces appliquées
- Gravité (UE par défaut)
- Force de roulement (si besoin d’arcade)
- Friction dynamique selon surface
- Drag (résistance à l’air)
- Forces latérales (vents, vortex, collisions)
4.3. États du Zorb
- EZorbState :
- Idle
- Rolling
- Braking
- Boosting
- Overheating
- Exploding
- Respawning

5. Système Énergie & Surchauffe
5.1. EnergyOverheatComponent
Un UActorComponent attaché au Zorb :
- Variables :
- float Energy
- float MaxEnergy
- float Overheat
- float MaxOverheat
- EOverheatLevel {None, Warning, Danger, Explode}
- Fonctions :
- AddEnergy(float Amount)
- ConsumeEnergy(float Amount)
- AddOverheat(float Amount)
- CoolDown(float DeltaTime)
- GetOverheatLevel()
- CanBoost() / CanBrake()
5.2. Logique
- Accumulation d’énergie :
- Si Rolling sans Boost ni Brake
- Si trajectoire stable (peu de collisions, angle limité)
- Surchauffe :
- Si vitesse > seuil
- Si Boost actif
- Si collisions violentes
- Si surfaces “chaudes”
- Réduction :
- Si vitesse < seuil
- Si freinage
- Si zones “fraîches”
- Explosion :
- Si Overheat >= MaxOverheat → Explode() :
- VFX + SFX
- Reset Energy et Overheat
- Respawn au dernier checkpoint

6. Perte de contrôle
6.1. Instabilité
- En fonction de la vitesse et de la surchauffe :
- Ajout de petites forces latérales aléatoires
- Augmentation de la sensibilité de l’input
- Drift automatique (force tangentielle)
6.2. Rebond amplifié
- Lors de collisions :
- Calcul de la vitesse d’impact
- Si > seuil → multiplier la force de rebond
- Ajouter surchauffe + perte d’énergie
6.3. Déséquilibre
- Après choc violent :
- Appliquer un “wobble” (oscillation) sur la rotation
- Réduire la précision de l’input pendant X secondes

7. Multijoueur
7.1. Modèle réseau
- Option 1 (simple) :
- Multijoueur synchrone léger (4–8 joueurs)
- Autorité serveur
- Réplication des positions + états du Zorb
- Option 2 (mobile-friendly) :
- Ghosts asynchrones (replay des inputs ou trajectoires)
- Interactions simulées localement
7.2. Interactions offensives
Implémentées via un UCombatComponent ou similaire :
- Bump latéral :
- Détection de collision latérale
- Application d’une force latérale sur l’autre Zorb
- Choc énergétique :
- Input joueur (bouton)
- Vérification énergie disponible
- Application d’une impulsion + surchauffe sur l’adversaire
- Glue :
- Spawn d’une zone au sol (Actor avec UBoxComponent)
- Modifie friction + vitesse + surchauffe
- Vortex :
- Zone autour du Zorb
- Force d’attraction légère sur les autres

8. Modes de jeu
8.1. GameModeSystem
- AGameModeBase dérivés :
- ATimeAttackGameMode
- AScoreGameMode
- ASurvivalGameMode
- AMultiplayerRaceGameMode
Chaque mode :
- configure les règles (surchauffe, énergie, respawn)
- gère la fin de partie
- communique avec l’UI

9. UI / HUD
9.1. Tech
- UMG / WidgetBlueprint
- Widgets principaux :
- WBP_HUD
- WBP_MainMenu
- WBP_PauseMenu
- WBP_Results
9.2. HUD
- Vitesse (barre ou chiffre)
- Énergie (barre)
- Surchauffe (barre + couleur + effets)
- Timer
- Position (multi)

10. VFX & Audio
10.1. VFX
- Niagara Systems :
- Traînée de vitesse
- Étincelles de collision
- Effets de surchauffe (glow, fumée)
- Explosion cartoon
10.2. Audio
- Sound Cues :
- Roulement
- Glissade
- Boost
- Freinage
- Surchauffe
- Explosion
- Ambiance terrain

11. Optimisation mobile
- Mesh terrain low-poly + LOD
- Limiter le nombre d’Actors (obstacles groupés)
- Physique simplifiée (tick réduit, substepping limité)
- VFX légers (LOD VFX, désactivation sur vieux devices)
- Résolution dynamique si nécessaire
