# Arrow Mind

## Description
"ArrowMind" est un jeu de mémoire interactif pour enfants, développé dans le cadre du projet universitaire long semestre 8 pour la formation EI2I 4 Polytech Sorbonne. Ce jeu combine des défis visuels et auditifs pour améliorer la mémoire et les capacités de reconnaissance des directions et des couleurs.

## Caractéristiques
- **Modes de Jeu** : 
  - **Visuel** : Affiche les directions ou les couleurs sur l'écran LCD.
    - Mode Flèches
    - Mode Couleurs
    - Mode Mixte (Flèches et Couleurs)
  - **Audio** : Donne des instructions vocales pour les directions et/ou les couleurs. -- *Non implémenté*
    - Mode Flèches
    - Mode Couleurs
    - Mode Mixte
- **Progression** : Chaque niveau ajoute une action aléatoire, augmentant progressivement la difficulté.
- **Affichage** : Écran LCD pour la sélection du mode de jeu et l'affichage du score.
- **Interaction** : Utilisation de boutons directionnels colorés pour la saisie des réponses.
- **Alimentation** : Jouet autonome en énergie, rechargeable, avec une autonomie d'au moins 24 heures en fonctionnement et une semaine en veille.

## Matériel
- **Carte Principale** : LPC1769
- **Système d'Exploitation** : FreeRTOS
- **Périphériques** : Boutons poussoirs directionnels colorés, écran LCD, haut-parleur pour les instructions audio
- **Structure** : PCB avec cinq boutons poussoirs, boîtier imprimé en 3D, alimenté par une batterie de 5V

## Schéma de Câblage

### LPC1769

| LPC1769 Pin | Fonction              |
|-------------|-----------------------|
| P0.4        | Reset (RST)           |
| P0.5        | Chip Select (CS)      |
| P0.10       | Data/Command (DC)     |
| P0.11       | Write (WR)            |
| P2.13       | Read (RD)             |
| P2.0        | D0                    |
| P2.1        | D1                    |
| P2.2        | D2                    |
| P2.3        | D3                    |
| P2.4        | D4                    |
| P2.5        | D5                    |
| P2.6        | D6                    |
| P2.7        | D7                    |
| P0.6        | Bouton BLEU           |
| P0.7        | Bouton GRIS           |
| P0.8        | Bouton BLANC          |
| P0.9        | Bouton ROUGE          |
| P1.0        | Bouton NOIR           |

### Écran TFT

| LPC1769 Pin | Écran TFT Pin         |
|-------------|-----------------------|
| P0.4        | Reset (RST)           |
| P0.5        | Chip Select (CS)      |
| P0.10       | Data/Command (DC)     |
| P0.11       | Write (WR)            |
| P2.13       | Read (RD)             |
| P2.0        | D0                    |
| P2.1        | D1                    |
| P2.2        | D2                    |
| P2.3        | D3                    |
| P2.4        | D4                    |
| P2.5        | D5                    |
| P2.6        | D6                    |
| P2.7        | D7                    |

### Boutons

| LPC1769 Pin | Bouton                |
|-------------|-----------------------|
| P0.6        | Bouton BLEU           |
| P0.7        | Bouton GRIS           |
| P0.8        | Bouton BLANC          |
| P0.9        | Bouton ROUGE          |
| P1.0        | Bouton NOIR           |




## Installation
1. **Prérequis** : Configurez votre environnement de développement pour la LPC1769 et FreeRTOS.
2. **Clonage** : Clonez ce dépôt.
3. **Compilation** : Compilez le projet. (Nous utilisons KEIL5 avec le compilateur version 5)
4. **Déploiement** : Téléversez le code compilé sur la carte LPC1769.

## Utilisation
Au démarrage, sélectionnez le mode de jeu sur l'écran LCD. Suivez les instructions affichées pour jouer. Si vous échouez, le meilleur score s'actualisera et la partie recommence.

## Contribution
Ce projet est une collaboration des étudiants de Polytech Sorbonne : @wolfyc, @A-LELONG, @gregoiremahon et Chahine Boukhenaissi.

## Licence
Ce projet est distribué sous une licence ouverte.

## Auteurs
- Mohamed ZAIDI
- Armand LELONG
- Gregoire Mahon
- Chahine Boukhenaissi
