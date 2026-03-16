ROADMAP DÉTAILLÉE — “ZORB EARTH RUN” (v1.0)

## Suivi d'avancement (mis à jour le 2026-03-16)

### Phase 0 — Pré-production
- [x] Roadmap définie
- [x] Setup du projet Unreal (base C++ fonctionnelle)
- [x] Structure de dossiers `Content` initialisée (Blueprints, Materials, Meshes, Textures, UI, VFX, Audio)
- [x] Contrôles de base du Zorb validés en jeu (déplacement + caméra)
- [ ] Choix final du système de génération de mesh (ProceduralMeshComponent vs plugin)
- [ ] Format des heightmaps serveur figé
- [ ] Prototype énergie/surchauffe implémenté

### Phase 1 — Prototype jouable
- [ ] Terrain issu d'une heightmap réelle
- [x] Zorb contrôlable sur map test
- [x] Gameplay départ/arrivée: trigger d'arrivée fonctionnel (message + logs)
- [x] Checkpoints: backend C++ + test en jeu validé (respawn fonctionnel)
- [x] Timer de course backend (start en BeginPlay, stop au finish)
- [x] HUD minimal C++ validé en jeu (timer + état de course + vitesse)
- [x] Physique arcade de base validée: gravité, roulage réaliste, contrôle caméra-référencé, respawn stable
- [x] Tuning arcade v1 validé en jeu: inertie directionnelle, axes corrigés, puissance maîtrisée, paramètres regroupés en `Zorb Physics` dans le BP
- [x] Système énergie + surchauffe + explosion: prototype C++ implémenté (HUD + overheat critique + respawn), tuning validé
- [x] Organisation des paramètres ajustables: profils structurés (Movement/Energy/Respawn/Feedback) + Project Settings `Zorb Tuning` + override local dans `BP_ZorbPawn`

### Prochaines étapes immédiates (ordre recommandé)
1. Valider et tuner en jeu les taux énergie/surchauffe (gain, cooldown, seuil critique). [validé]
2. Ajouter un feedback visuel d'explosion (VFX/flash) sur surchauffe critique. [flash HUD + halo emissif avec transitions douces validés]
3. Introduire un mini boost consommant l'énergie pour fermer la boucle gameplay. [validé en jeu]
4. Paramètres feedback halo exposés dans Project Settings > Game > Zorb Tuning (Feedback)

### Préparation outillage tuning (future étape)
1. Créer un écran Admin runtime pour éditer les paramètres critiques (movement/energy/respawn).
2. Ajouter sauvegarde/chargement de presets (prototype, arcade, hardcore).

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
