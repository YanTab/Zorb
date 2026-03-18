 GAME FEATURES DOCUMENT — “ZORB EARTH RUN” (v1.2)

1. Structure générale du jeu
1.1. Genre
- Arcade racing
- Physique stylisée
- Génération procédurale basée sur données réelles
- Multijoueur compétitif et interactif
1.2. Plateformes
- iOS
- Android
- PC (Windows)
1.3. Durée d’une partie
- 1 à 3 minutes
- Sessions rapides, rejouabilité infinie

2. Gameplay Core
2.1. Contrôle du Zorb
Inputs
- Mobile :
- Tilt (inclinaison du téléphone)
- Stick virtuel (option)
- Bouton Boost
- Bouton Frein
- PC :
- WASD ou flèches
- Espace = Boost
- Shift = Frein
Comportement
- Le Zorb roule automatiquement sous l’effet de la gravité.
- Le joueur influence la direction, la gestion de l’énergie et la stabilité.

2.3. Système de profils (Preset System)

Objectif:
- Proposer des styles de conduite différents sans créer de hiérarchie unique.

Profils disponibles:
- Classic
- Agile
- Heavy
- Wild

Spécification fonctionnelle:
- Le profil actif modifie des multiplicateurs de comportement (vitesse, contrôle, grip, freinage, dynamique interne).
- Le profil est sélectionnable en configuration et utilisable en benchmark automatisé.
- Le profil ne doit pas casser les contrôles de base ni la lisibilité caméra.

Contraintes d'équilibrage:
- Chaque profil doit être top-1 sur au moins un scénario de référence.
- Aucun profil ne doit être dernier sur tous les scénarios.
- Les écarts inter-profils sur scénario standard doivent rester maîtrisés.
- Les scénarios de benchmark doivent couvrir air + ground (flow, freinage, virages, checkpoint).

Matrice de différenciation cible:
- Classic: stabilité et constance en trajectoire.
- Agile: relance et changements d'angle rapides.
- Heavy: contrôle du freinage et tenue sur sections engagées.
- Wild: bursts de performance avec variabilité plus élevée.

Validation:
- Mesure via télémétrie exportée et analyse offline.
- Comparaison par scénario et classement global pondéré.
- Runs invalides exclus (ex: respawn critique parasite pendant benchmark).

3. Système d’Énergie (mécanique centrale)
3.1. Accumulation d’énergie — “Roue libre”
Le Zorb accumule de l’énergie lorsqu’il :
- roule sans freiner
- roule sans booster
- maintient une trajectoire stable
- évite les collisions
L’énergie dépend :
- de la vitesse
- de la durée en roue libre
- de la pente
- de la stabilité
3.2. Utilisation de l’énergie
Freinage assisté
- Consomme de l’énergie
- Stabilise le Zorb
- Réduit la surchauffe
- Permet de négocier virages serrés et pentes extrêmes
Boost / Accélération
- Consomme de l’énergie
- Augmente la vitesse
- Augmente la surchauffe
- Risque de perte de contrôle
3.3. Capacité maximale
- Limite d’énergie variable selon le Zorb
- Peut être améliorée via progression (optionnel)

3.4. Extension prévue — Capacités spéciales par profil (post-tuning)

Statut:
- Prévu, non implémenté dans cette version.

Principe:
- Chaque profil aura une capacité dédiée (active ou passive) avec contrepartie.

Exemples de design (à valider):
- Classic: mitigation ponctuelle des pertes de contrôle.
- Agile: impulsion de relance courte.
- Heavy: réduction temporaire du gain thermique.
- Wild: burst renforcé avec coût thermique accru.

Règles techniques à respecter:
- cooldown explicite,
- durée d'effet bornée,
- impact mesurable via télémétrie,
- pas de capacité "gratuite" sans trade-off.

4. Système de Surchauffe (pilier du gameplay)
4.1. Jauge de surchauffe
La surchauffe augmente lorsque :
- le joueur reste trop longtemps à vitesse max
- le joueur booste trop longtemps
- le joueur accumule trop d’énergie sans la dépenser
- le Zorb roule sur des surfaces “chaudes”
- le joueur subit des collisions violentes
4.2. Niveaux de surchauffe
Niveau 1 — Avertissement
- Vibrations légères
- Traînée lumineuse rouge
- Son de friction
Niveau 2 — Danger
- Drift involontaire
- Perte de précision
- Consommation automatique d’énergie pour stabiliser
Niveau 3 — Explosion
- Explosion cartoon
- Perte totale d’énergie
- Respawn instantané au dernier checkpoint
- Pas de punition lourde
4.3. Réduction de la surchauffe
- Freinage
- Ralentissement
- Zones “fraîches” (neige, herbe)
- Zones de refroidissement (bonus)

5. Perte de Contrôle (pilier du gameplay)
5.1. Instabilité progressive
Plus la vitesse augmente :
- plus le Zorb oscille
- plus la direction devient sensible
- plus les collisions sont dangereuses
5.2. Drift involontaire
À haute vitesse ou en surchauffe :
- drift automatique
- correction nécessaire
- perte d’énergie
5.3. Rebond amplifié
Les collisions à haute vitesse :
- rebondissent plus fort
- dévient la trajectoire
- augmentent la surchauffe
5.4. Déséquilibre
Après un choc violent :
- tangage du Zorb
- perte de précision pendant 1 seconde
- consommation automatique d’énergie
5.5. Surfaces à faible contrôle
- glace
- sable
- boue
- neige fraîche
Ces surfaces amplifient la perte de contrôle.

6. Génération de Pistes
6.1. Entrée utilisateur
- Coordonnée GPS
- Code partagé
- Seed aléatoire
- Piste du jour
6.2. Pipeline
- Récupération de la heightmap optimisée
- Conversion en mesh low-poly
- Analyse de la pente
- Détection du point de départ (altitude max)
- Détection de l’arrivée (altitude min)
- Génération procédurale :
- obstacles
- tremplins
- zones spéciales
- checkpoints
- routes alternatives
6.3. Types d’obstacles
- Rochers
- Arbres
- Falaises
- Ponts naturels
- Tunnels
- Barrières
- Zones de glue
- Vent latéral
- Surfaces glissantes
6.4. Micro‑relief procédural
Ajout automatique de :
- bosses
- ondulations
- creux
Objectif : casser les lignes droites.

7. Modes de Jeu
7.1. Time Attack
- Objectif : meilleur temps
- Ghosts des meilleurs joueurs
- Classements par piste
7.2. Score / Tricks
- Sauts, flips, trajectoires risquées
- Multiplicateurs
- Bonus d’énergie
7.3. Survie
- Ne pas sortir de la piste
- Obstacles dynamiques
- Surchauffe plus rapide
7.4. Multijoueur
- 4 à 8 joueurs
- Interactions offensives
- Classements en temps réel
- Matchmaking rapide

8. Multijoueur — Interactions Offensives
8.1. Bump latéral
- Dévie la trajectoire d’un adversaire
- Amplifié à haute vitesse
8.2. Choc énergétique
- Consomme ton énergie
- Repousse un adversaire
- Augmente sa surchauffe
8.3. Glue automatique
Si un joueur surchauffe trop :
- il laisse une traînée collante
- ralentit les autres
- se pénalise lui-même
8.4. Vortex
Chaque joueur génère un vortex léger :
- attire les autres
- amplifie la perte de contrôle
8.5. Boost partagé
Si deux joueurs se frôlent à haute vitesse :
- boost mutuel
- risque de surchauffe

9. Progression & Méta
9.1. Déblocages
- Skins de Zorb
- Traces au sol
- Effets de boost
- Particules
- Emotes (PC uniquement)
9.2. Progression du joueur
- Niveau global
- Niveau par mode
- Succès / défis
9.3. Saisons
- Classements saisonniers
- Récompenses exclusives
- Pistes spéciales

10. Interface & UX
10.1. Écran principal
- Jouer
- Piste du jour
- Multijoueur
- Skins
- Classements
- Paramètres
10.2. HUD en course
- Vitesse
- Énergie
- Jauge de surchauffe
- Timer
- Mini-map (optionnelle)
- Position (en multi)
10.3. Feedback visuels
- Traînée lumineuse
- Particules de vitesse
- Vignettage léger en boost
- Éclats lors des collisions
- Effets de surchauffe

11. Audio
11.1. Effets sonores
- Roulement
- Glissade
- Boost
- Freinage
- Collisions
- Surchauffe
- Explosion cartoon
- Tricks réussis
11.2. Musique
- Dynamique
- Adaptative selon la vitesse et la surchauffe

12. Paramètres Techniques (résumé)
12.1. Terrain
- Mesh low-poly
- LODs automatiques
- Streaming si nécessaire
12.2. Physique
- Simplifiée pour mobile
- Tick optimisé
- Colliders low-poly
12.3. Multijoueur
- Asynchrone (ghosts)
- Synchrone (courses)
- Interactions légères pour éviter la latence

📗 FIN DU GAME FEATURES DOCUMENT — v1.2


