# Plan de Développement pour ArrowMind

## Structure des Dossiers
```
src/
├── FreeRTOS/
│ ├── Demo/
│ └── Source/
├── main.c
├── game_logic.c
├── game_logic.h
├── lcd_display.c
├── lcd_display.h
├── button_input.c
├── button_input.h
├── audio_control.c
└── audio_control.h
```

## Description des Fichiers

### `main.c`
- **Fonction** : Point d'entrée principal du jeu.
- **Contenu** : 
  - Initialisation du système.
  - Création et gestion des tâches FreeRTOS.
  - Boucle principale du programme.

### `game_logic.c` et `game_logic.h`
- **Fonction** : Gère la logique principale du jeu.
- **Contenu** : 
  - Gestion des niveaux.
  - Séquence des actions (directions et couleurs).
  - Logique pour augmenter la difficulté.

### `lcd_display.c` et `lcd_display.h`
- **Fonction** : Contrôle l'affichage sur l'écran LCD.
- **Contenu** : 
  - Affichage du score et des niveaux.
  - Affichage des instructions en mode visuel.
  - Gestion de l'interface utilisateur.

### `button_input.c` et `button_input.h`
- **Fonction** : Gestion des entrées des boutons.
- **Contenu** : 
  - Détection des appuis sur les boutons.
  - Traitement des entrées pour la logique du jeu.

### `audio_control.c` et `audio_control.h`
- **Fonction** : Gère la partie audio du jeu.
- **Contenu** : 
  - Lecture des instructions audio en mode audio.
  - Contrôle des effets sonores.

## Intégration avec FreeRTOS
- Création de tâches distinctes pour la logique du jeu, l'affichage LCD, les entrées des boutons, et le contrôle audio.
- Utilisation des sémaphores et des files d'attente pour la synchronisation des tâches.
- Gestion des priorités des tâches pour assurer un fonctionnement fluide.
