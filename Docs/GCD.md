 GAME CONCEPT DOCUMENT — “ZORB EARTH RUN” (v1.2)
🎯 1. Vision du jeu
Zorb Earth Run est un jeu arcade où le joueur dévale des pentes générées à partir de la topographie réelle de la Terre.
Chaque piste est unique : elle est créée à partir d’une coordonnée GPS, d’un code partagé ou d’un seed aléatoire.
Le jeu mise sur :
- la vitesse,
- la fluidité,
- la simplicité,
- le fun immédiat,
- une direction artistique stylisée,
- et un gameplay arcade avec interactions multijoueurs.

🧩 2. Pitch
“Choisis un point sur Terre. Dévale sa pente. Affronte les autres. Domine les classements.”

🌍 3. Concept central
- Le joueur génère une piste en entrant :
- une coordonnée GPS
- un code partagé
- un seed aléatoire
- Le jeu télécharge une heightmap optimisée correspondant à cette zone.
- Unreal génère un terrain jouable low-poly stylisé.
- Le joueur contrôle une boule (Zorb) qui dévale la pente en gérant sa vitesse, son énergie et ses interactions avec les autres joueurs.

🕹️ 4. Boucles de jeu
🔁 Boucle courte (Core Loop)
- Entrer une coordonnée / choisir une piste
- Génération de la piste
- Descente (vitesse, énergie, tricks, survie, interactions)
- Score / temps / classement
- Recommencer ou partager la piste
🔁 Boucle moyenne (Session Loop)
- Défis quotidiens
- Pistes recommandées
- Amélioration des compétences du joueur
- Déblocage de skins
🔁 Boucle longue (Meta Loop)
- Collection de skins
- Classements mondiaux
- Saisons / événements
- Pistes “mythiques” (Everest, Andes, Alpes, Islande…)

🧠 5. Identité du jeu
🎨 Direction artistique
- Low-poly stylisé
- Couleurs vives
- Effets légers (poussière, neige, particules)
- Interface minimaliste
🎵 Ambiance sonore
- Musique dynamique
- Sons de roulement, glissade, vent
- Feedback audio pour les tricks, boosts, collisions
🎮 Feeling de gameplay
- Arcade
- Rapide
- Accessible
- Satisfaisant dès les premières secondes
- Interactions fun et imprévisibles en multijoueur

🧱 6. Features principales
🗺️ Génération de pistes
- Import de heightmaps réelles
- Analyse automatique de la pente
- Détection du point de départ et d’arrivée
- Placement procédural :
- obstacles
- boosts
- tremplins
- checkpoints
- zones spéciales (glue, vents latéraux, surfaces glissantes)

🟡 Physique de la boule (nouvelle version)
La physique est arcade et repose sur un système d’énergie dynamique :
🔋 Accumulation d’énergie — “Roue libre”
- En laissant la boule rouler sans freiner, elle accumule de l’énergie cinétique stockée.
- Cette énergie peut être utilisée de deux façons :
🛑 1. Freinage assisté
- Permet de négocier :
- virages serrés
- pentes très fortes
- zones dangereuses
- Le freinage consomme l’énergie accumulée.
⚡ 2. Boost / Accélération
- Utilisable sur :
- zones plates
- relances après un saut
- dépassements en multijoueur
- Le boost consomme l’énergie accumulée.
🎯 Objectif du système
Créer un gameplay tactique, où le joueur doit :
- choisir quand laisser rouler
- quand freiner
- quand booster
- comment optimiser sa trajectoire pour maximiser l’énergie disponible

🟣 6.b Concept des profils de Zorb (identité gameplay)

Le jeu repose sur 4 profils jouables, conçus pour être situatifs.
Objectif produit: éviter un "meilleur profil absolu" et proposer des avantages/inconvénients lisibles selon le terrain.

Profils:
- Classic: profil polyvalent, stable, lisible en enchaînements techniques.
- Agile: profil réactif, fort en relance et changements de direction.
- Heavy: profil robuste, performant en contrôle/freinage et gestion des pentes.
- Wild: profil à risque/récompense, fort en pics de performance mais moins prévisible.

Principes d'équilibrage:
- Chaque profil doit dominer au moins un type de contexte (terrain/scénario).
- Aucun profil ne doit dominer toutes les catégories à la fois.
- Chaque avantage majeur implique un coût réel (stabilité, chauffe, relance, contrôle).
- L'écart global inter-profils doit rester limité pour préserver le choix joueur.

Types de terrains/cas cibles:
- Vallée fluide (flow/sprint)
- Switchbacks (virages alternés)
- Air control (phases aériennes)
- Gestion freinage/relance
- Terrain mixte "checkpoint"

Vision long terme:
- Les profils recevront une capacité spéciale propre (active ou passive), avec cooldown et trade-off explicite.
- Cette couche sera ajoutée après stabilisation du tuning de base.

👥 7. Multijoueur (nouvelle version)
Le multijoueur est asynchrone ou synchrone, selon le mode, et inclut des interactions offensives fun.
🎮 Interactions possibles
- Déviation de trajectoire (petit bump latéral)
- Perte d’énergie (collision ou attaque)
- Glue ralentissante (zone posée par un joueur)
- Choc énergétique (consomme ton énergie pour repousser un adversaire)
- Vortex temporaire (aspire légèrement les concurrents proches)
- Boost partagé (si deux joueurs se frôlent à haute vitesse)
🧨 Philosophie
- Pas de violence
- Pas de destruction
- Des interactions fun, cartoon, imprévisibles
- Toujours lisibles et faciles à comprendre

🏁 8. Modes de jeu
⏱️ Time Attack
- Descendre le plus vite possible
- Ghosts des meilleurs joueurs
⭐ Score / Tricks
- Sauts, flips, trajectoires risquées
- Multiplicateurs
💀 Survie
- Ne pas sortir de la piste
- Obstacles dynamiques
👥 Multijoueur
- Courses à 4–8 joueurs
- Interactions offensives
- Classements par piste
🎯 Défis quotidiens
- Une piste unique par jour
- Classement mondial 24h

📱💻 9. Plateformes
- iOS
- Android
- PC (Windows)

💰 10. Monétisation (optionnelle)
- Skins de Zorb
- Pass saisonnier
- Packs de zones premium
- Publicités récompensées (respawn, boosts)

🎯 11. Objectifs du projet
- Créer un jeu fun, unique et rejouable à l’infini
- Exploiter la topographie réelle comme source de gameplay
- Offrir une expérience fluide et accessible
- Construire une base technique solide pour évoluer vers un jeu plus ambitieux
