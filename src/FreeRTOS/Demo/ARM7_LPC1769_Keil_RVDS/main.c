#include "LPC17xx.h"
#include "images_data.h"
#include <stdlib.h> // Pour rand() et srand()
#include <time.h>   // Pour time(), afin d'initialiser rand()

// Définitions des broches pour l'écran TFT
#define PIN_RST  (1 << 4)  // Reset - P0.4
#define PIN_CS   (1 << 5)  // Chip Select - P0.5
#define PIN_DC   (1 << 10) // Data/Command - P0.10
#define PIN_WR   (1 << 11) // Write - P0.11
#define PIN_RD   (1 << 13) // Read - P2.13

#define P0_MASK(bit) (1 << bit) // Macro pour créer un masque pour le port 0

// Définition des masques pour les broches de données D0-D7 sur le port 2
#define LCD_DATA_MASK (0xFF) // D0-D7 sur P2.0 à P2.7

// Définition des couleurs en composantes RGB565 -> Sur 16 bits
// Couleurs basiques
#define RED		0xf800
#define GREEN	0x07e0
#define BLUE	0x001f
#define BLACK	0x0000
#define YELLOW	0xffe0
#define WHITE	0xffff

// Autres couleurs
#define CYAN		0x07ff
#define BRIGHT_RED	0xf810
#define GRAY1		0x8410
#define GRAY2		0x4208

// Directions possibles des flèches à l'écran.
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} ArrowDirection;

// ports des boutons en fonction de leurs directions (sur le port P0.x)

#define UP_BUTTON    6  // Bleu
#define DOWN_BUTTON  7  // Noir
#define LEFT_BUTTON  9  // Rouge
#define RIGHT_BUTTON 8  // Blanc

// Séquence de directions 
#define MAX_SEQUENCE 10 // Longueur maximale de la séquence
ArrowDirection sequence[MAX_SEQUENCE];
int sequenceLength = 0; // Longueur actuelle de la séquence

// Lecture Bitmap
extern const unsigned char image_palette[256 * 3];

extern const uint16_t imageData[];
extern const int imageWidth;
extern const int imageHeight;


// Générer une séquence de directions

void generateSequence(int level) {
		int i;
    sequenceLength = level; // Augmente avec le niveau
    for (i = 0; i < sequenceLength; i++) {
        sequence[i] = (ArrowDirection)(rand() % 4); // Génère une direction aléatoire et la convertit en ArrowDirection
    }
}

// Initialiser le port du bouton comme entrée
void initButtonPort(uint8_t portBit) {
    LPC_GPIO0->FIODIR &= ~P0_MASK(portBit); // Configure le pin spécifié comme entrée
}

void initButtonPorts(void) {
    initButtonPort(6); // Initialiser le port du bouton BLEU (P0.6)
    initButtonPort(7); // Initialiser le port du bouton NOIR (P0.7)
    initButtonPort(8); // Initialiser le port du bouton BLANC (P0.8)
    initButtonPort(9); // Initialiser le port du bouton ROUGE (P0.9)
}

// Lire l'état du bouton sur P0.x (x=portBit)
int readButtonState(uint8_t portBit) {
	
		/*
		Rouge : P0.9
		Blanc : P0.8
		Noir : P0.7
		Bleu : P0.6
		*/
		 return (LPC_GPIO0->FIOPIN & P0_MASK(portBit)) ? 0 : 1;
}


void initGPIO(void) {
    LPC_GPIO0->FIODIR |= PIN_RST | PIN_CS | PIN_DC | PIN_WR;
    LPC_GPIO2->FIODIR |= PIN_RD | LCD_DATA_MASK;
}

void initTFTGPIO(void) {
    LPC_PINCON->PINSEL0 &= ~((3 << 8) | (3 << 10) | (3 << 20) | (3 << 22));
    LPC_GPIO0->FIODIR |= PIN_RST | PIN_CS | PIN_DC | PIN_WR;
    LPC_GPIO2->FIODIR |= PIN_RD | LCD_DATA_MASK;
    
    LPC_GPIO0->FIOSET = PIN_CS | PIN_WR | PIN_RST;
    LPC_GPIO2->FIOSET = PIN_RD;
}

void sendCMD(uint8_t cmd) {
    LPC_GPIO0->FIOCLR = PIN_CS;
    LPC_GPIO0->FIOCLR = PIN_DC;
    LPC_GPIO2->FIOPIN = (LPC_GPIO2->FIOPIN & ~LCD_DATA_MASK) | cmd;
    LPC_GPIO0->FIOCLR = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_CS;
}

void WRITE_DATA(uint8_t data) {
    LPC_GPIO0->FIOCLR = PIN_CS;
    LPC_GPIO0->FIOSET = PIN_DC;
    LPC_GPIO2->FIOPIN = (LPC_GPIO2->FIOPIN & ~LCD_DATA_MASK) | data;
    LPC_GPIO0->FIOCLR = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_CS;
}


void TFTinit(void) {
		int i; 
    initTFTGPIO(); // Initialise les GPIOs pour les commandes TFT

    // Reset du TFT
    LPC_GPIO0->FIOCLR = PIN_RST;
    for (i = 0; i < 100000; i++); // Petite attente // Attente après le reset
    LPC_GPIO0->FIOSET = PIN_RST;
    for (i = 0; i < 500000; i++); // Petite attente // Attente pour que le TFT soit prêt

    // Commence la séquence d'initialisation du TFT
    sendCMD(0x01); // Software reset
    for (i = 0; i < 100000; i++); // Petite attente

    // Commande d'initialisation du contrôleur du TFT
    sendCMD(0xCF);  
    WRITE_DATA(0x00); 
    WRITE_DATA(0x8B); 
    WRITE_DATA(0X30); 

    sendCMD(0xED);  
    WRITE_DATA(0x67); 
    WRITE_DATA(0x03); 
    WRITE_DATA(0X12); 
    WRITE_DATA(0X81);

    sendCMD(0xE8);  
    WRITE_DATA(0x85); 
    WRITE_DATA(0x10); 
    WRITE_DATA(0x7A); 

    sendCMD(0xCB);  
    WRITE_DATA(0x39); 
    WRITE_DATA(0x2C); 
    WRITE_DATA(0x00); 
    WRITE_DATA(0x34); 
    WRITE_DATA(0x02);

    sendCMD(0xF7);  
    WRITE_DATA(0x20); 

    sendCMD(0xEA);  
    WRITE_DATA(0x00); 
    WRITE_DATA(0x00); 

    sendCMD(0xC0);    // Power control 
    WRITE_DATA(0x1B); // VRH[5:0] 

    sendCMD(0xC1);    // Power control 
    WRITE_DATA(0x10); // SAP[2:0];BT[3:0] 

    sendCMD(0xC5);    // VCM control 
    WRITE_DATA(0x3F); 
    WRITE_DATA(0x3C); 

    sendCMD(0xC7);    // VCM control2 
    WRITE_DATA(0xB7); 

    sendCMD(0x36);    // Memory Access Control 
    WRITE_DATA(0x48); 

    sendCMD(0x3A); 
    WRITE_DATA(0x55); 

    sendCMD(0xB1); 
    WRITE_DATA(0x00);  
    WRITE_DATA(0x1B); 

    sendCMD(0xF2);    // 3Gamma Function Disable 
    WRITE_DATA(0x08); 

    sendCMD(0x26);    // Gamma curve selected 
    WRITE_DATA(0x01); 

    // Commandes supplémentaires pour régler le gamma de l'écran
    sendCMD(0xE0); // Set Gamma 
    WRITE_DATA(0x0F); 
    WRITE_DATA(0x31); 
    WRITE_DATA(0x2B); 
    WRITE_DATA(0x0C); 
    WRITE_DATA(0x0E); 
    WRITE_DATA(0x08); 
    WRITE_DATA(0x4E); 
    WRITE_DATA(0xF1); 
    WRITE_DATA(0x37); 
    WRITE_DATA(0x07); 
    WRITE_DATA(0x10); 
    WRITE_DATA(0x03); 
    WRITE_DATA(0x0E); 
    WRITE_DATA(0x09); 
    WRITE_DATA(0x00); 

    sendCMD(0xE1); // Set Gamma 
    WRITE_DATA(0x00); 
    WRITE_DATA(0x0E); 
    WRITE_DATA(0x14); 
    WRITE_DATA(0x03); 
    WRITE_DATA(0x11); 
    WRITE_DATA(0x07); 
    WRITE_DATA(0x31); 
    WRITE_DATA(0xC1); 
    WRITE_DATA(0x48); 
    WRITE_DATA(0x08); 
    WRITE_DATA(0x0F); 
    WRITE_DATA(0x0C); 
    WRITE_DATA(0x31); 
    WRITE_DATA(0x36); 
    // Suite de la configuration du gamma
    WRITE_DATA(0x0F); 
    WRITE_DATA(0x31); 
    WRITE_DATA(0x2B); 
    WRITE_DATA(0x0C); 
    WRITE_DATA(0x0E); 
    WRITE_DATA(0x08); 
    WRITE_DATA(0x4E); 
    WRITE_DATA(0xF1); 
    WRITE_DATA(0x37); 
    WRITE_DATA(0x07); 
    WRITE_DATA(0x10); 
    WRITE_DATA(0x03); 
    WRITE_DATA(0x0E); 
    WRITE_DATA(0x09); 
    WRITE_DATA(0x00); 

    // Sortie de veille
    sendCMD(0x11);  // Sortir du mode veille
    for (i = 0; i < 200000; i++); // Petite attente  // Attente après la commande "sleep out"

    // Activation de l'affichage
    sendCMD(0x29);  // Activation de l'affichage
		for (i = 0; i < 200000; i++); // attente affichage ready
    //vTaskDelay(pdMS_TO_TICKS(100));  // Attente pour que l'affichage soit prêt
}

/*void fillScreen(uint16_t color) {
		// Fonction pour remplir l'écran d'une couleur unie (rempli pas en entier; a revoir)

		int i; 
    // Sélectionnez la zone d'écran entière
    sendCMD(0x2A); // Commande pour colonne (X)
    WRITE_DATA(0x00);
    WRITE_DATA(0x00);
    WRITE_DATA((320 - 1) >> 8);
    WRITE_DATA((320 - 1) & 0xFF);

    sendCMD(0x2B); // Commande pour ligne (Y)
    WRITE_DATA(0x00);
    WRITE_DATA(0x00);
    WRITE_DATA((240 - 1) >> 8);
    WRITE_DATA((240 - 1) & 0xFF);

    sendCMD(0x2C); // Commencer l'écriture de données

    for (i = 0; i < 320 * 240; i++) {
        WRITE_DATA(color >> 8); // Partie haute de la couleur
        WRITE_DATA(color & 0xFF); // Partie basse de la couleur
    }
}*/


void setCol(uint16_t start, uint16_t end) {
		// Fonction pour sélectionner la plage de valeurs sur laquelle écrire
	
    sendCMD(0x2A); // Commande pour colonne (X)
    WRITE_DATA(start >> 8);
    WRITE_DATA(start & 0xFF);
    WRITE_DATA(end >> 8);
    WRITE_DATA(end & 0xFF);
}

void setPage(uint16_t start, uint16_t end) {
    sendCMD(0x2B); // Commande pour ligne (Y)
    WRITE_DATA(start >> 8);
    WRITE_DATA(start & 0xFF);
    WRITE_DATA(end >> 8);
    WRITE_DATA(end & 0xFF);
}

void writePixel(uint16_t color) {
    WRITE_DATA(color >> 8); // Partie haute de la couleur
    WRITE_DATA(color & 0xFF); // Partie basse de la couleur
}

void setArea(int x, int y, int width, int height) {
		// Configure la zone de l'écran où les données de l'image seront écrites
    sendCMD(0x2A); // Commande pour colonne (X)
    WRITE_DATA(x >> 8);
    WRITE_DATA(x & 0xFF);
    WRITE_DATA((x + width - 1) >> 8);
    WRITE_DATA((x + width - 1) & 0xFF);

    sendCMD(0x2B); // Commande pour ligne (Y)
    WRITE_DATA(y >> 8);
    WRITE_DATA(y & 0xFF);
    WRITE_DATA((y + height - 1) >> 8);
    WRITE_DATA((y + height - 1) & 0xFF);

    sendCMD(0x2C); // Commencer à écrire des données
}

/*
Fonction qui affiche simplement un BMP extrait du code python bmp to c array
void displayImage(const uint16_t *imageData, int x, int y, int width, int height) {
		int i;
		int j;
	
    // Configure la zone de l'écran où les données de l'image seront écrites
		setArea(x, y, width, height);

    // Écrit les données de pixels
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            writePixel(imageData[i * width + j]);
        }
    }
}*/


void displayArrow(const uint16_t *imageData, int x, int y, int width, int height, ArrowDirection direction) {
    int i, j;
    
    setArea(x, y, width, height);
	
		// INVERSION DES DIRECTIONS CAR ECRAN A L'ENVERS
    switch (direction) {
        case UP: direction = DOWN; break;
        case DOWN: direction = UP; break;
        case LEFT: direction = RIGHT; break;
        case RIGHT: direction = LEFT; break;
    }    
		
		switch (direction) {
        case UP:
            // Orientation originale, pas de rotation
            for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                    writePixel(imageData[i * width + j]);
                }
            }
            break;

        case DOWN:
            // Rotation de 180 degrés
            for (i = height - 1; i >= 0; i--) {
                for (j = width - 1; j >= 0; j--) {
                    writePixel(imageData[i * width + j]);
                }
            }
            break;

        case LEFT:
            // Rotation de 90 degrés vers la gauche
            for (j = width - 1; j >= 0; j--) {
                for (i = 0; i < height; i++) {
                    writePixel(imageData[j + (height - i - 1) * width]);
                }
            }
            break;

        case RIGHT:
            // Rotation de 90 degrés vers la droite
            for (j = 0; j < width; j++) {
                for (i = height - 1; i >= 0; i--) {
                    writePixel(imageData[j + i * width]);
                }
            }
            break;
    }
}

void displayMultipleArrows(const uint16_t *imageData, int width, int height, int times) {
		/* 
		Fonction pour afficher quatre flèches tournantes dans la même direction 
		Affiche times fois la rotation (peut être utile par exemple durant l'initialisation du jeu -> 10 times = 1 seconde environ)
		*/
	
    int delay;
    ArrowDirection currentDirection = UP; 
		int counter = 0;
	
    while (counter < times) {
				// Léger décalage d'affichage car les affichages sont appelés un par un pour chaque flèche.
        displayArrow(imageData, 0, 40, width, height, currentDirection);
        displayArrow(imageData, 120, 40, width, height, currentDirection); 
        displayArrow(imageData, 0, 160, width, height, currentDirection); 
        displayArrow(imageData, 120, 160, width, height, currentDirection); 
        
        for (delay = 0; delay < 1000000; delay++);

        switch (currentDirection) {
            case UP:
                currentDirection = RIGHT;
                break;
            case RIGHT:
                currentDirection = DOWN;
                break;
            case DOWN:
                currentDirection = LEFT;
                break;
            case LEFT:
                currentDirection = UP;
                break;
        }
				counter++;
    }
}


void fillScreen(uint32_t color) {
	  int i;
    setCol(0, 239);
    setPage(0, 319);
    sendCMD(0x2c); // Commence à écrire dans la mémoire d'affichage

    LPC_GPIO0->FIOSET = PIN_DC; // Mode donnée
    LPC_GPIO0->FIOCLR = PIN_CS; // Sélectionne le TFT

    for (i = 0; i < 76800; i++) { // Pour chaque pixel (320x240)
        WRITE_DATA(color >> 8); // Partie haute de la couleur
        WRITE_DATA(color & 0xFF); // Partie basse de la couleur
    }

    LPC_GPIO0->FIOSET = PIN_CS; // Désélectionne le TFT
}

void displaySequence(void) {
		int i;
		volatile int delay;
    for (i = 0; i < sequenceLength; i++) {
        displayArrow(up_image_data, 120, 40, 120, 120, sequence[i]);
        for (delay = 0; delay < 1000000; delay++); // Délai entre les flèches
    }
}

void addDirectionToSequence(ArrowDirection newDirection) {
    if (sequenceLength < MAX_SEQUENCE) {
        sequence[sequenceLength++] = newDirection;
    }
}

int readPlayerInput(void) {
    int currentInput;
    int inputIndex = 0; // Index pour suivre les entrées de l'utilisateur
		int i;
    volatile int delay;
    int inputSequence[MAX_SEQUENCE]; // Tableau pour stocker les entrées de l'utilisateur

    // Réinitialise le tableau des entrées
    for(i = 0; i < MAX_SEQUENCE; i++) {
        inputSequence[i] = -1;
    }


    while (inputIndex < sequenceLength) {
        currentInput = -1;

        // Lecture de l'entrée de l'utilisateur
        if (readButtonState(UP_BUTTON) == 1) {
            currentInput = UP;
        } else if (readButtonState(DOWN_BUTTON) == 1) {
            currentInput = DOWN;
        } else if (readButtonState(LEFT_BUTTON) == 1) {
            currentInput = LEFT;
        } else if (readButtonState(RIGHT_BUTTON) == 1) {
            currentInput = RIGHT;
        }

        // Si une entrée a été détectée
        if (currentInput != -1) {
            // Attendre un court instant pour le debouncing
            for (delay = 0; delay < 50000; delay++);

            // Confirmation de l'entrée
            if ((currentInput == UP && readButtonState(UP_BUTTON) == 0) ||
                (currentInput == DOWN && readButtonState(DOWN_BUTTON) == 0) ||
                (currentInput == LEFT && readButtonState(LEFT_BUTTON) == 0) ||
                (currentInput == RIGHT && readButtonState(RIGHT_BUTTON) == 0)) {

                // Stocke l'entrée de l'utilisateur dans le tableau
                inputSequence[inputIndex++] = currentInput;
                
                // Vérifie si la séquence d'entrée correspond à la séquence attendue jusqu'à présent
                for (i = 0; i < inputIndex; i++) {
                    if (inputSequence[i] != sequence[i]) {
                        return 0; // Échec si l'entrée ne correspond pas à la séquence attendue
                    }
                }

                // Affichage visuel de la direction entrée
                displayArrow(up_image_data, 0, 40, 120, 120, currentInput);
            }
        }
    }

    return 1; // Succès si toute la séquence est correctement saisie
}





ArrowDirection generateNextDirection(ArrowDirection lastDirection) {
    ArrowDirection nextDirection;
    do {
        nextDirection = (ArrowDirection)(rand() % 4); // Génère une nouvelle direction aléatoirement
    } while (nextDirection == lastDirection); // Répète si la nouvelle direction est identique à la dernière
    return nextDirection;
}

void initGame(void) {
    SystemInit(); // Initialisation du système
    TFTinit(); // Initialisation de l'écran TFT
		initButtonPorts(); // Initialiser les ports des boutons
		
		// Définir manuellement la première direction
    //addDirectionToSequence(UP); 
    //srand(time(NULL)); // Initialisation du générateur de nombres aléatoires
}

void restartGame(void) {
    initGame(); 
    sequenceLength = 0;
    //addDirectionToSequence(UP);
}

void playGame(void) {
    int level = 1;
    initGame();
		displayMultipleArrows(up_image_data, 120,120, 5);
		TFTinit();


    while (1) {
        displaySequence(); // Affichez la séquence à l'écran

        if (readPlayerInput()) { // Vérifiez si l'entrée du joueur correspond à la séquence
						// Assurez-vous que la dernière direction de la séquence est correctement obtenue
						ArrowDirection lastDirection = sequenceLength > 0 ? sequence[sequenceLength - 1] : (ArrowDirection)-1;
						
						// Génère une nouvelle direction différente de la dernière direction de la séquence
						ArrowDirection newDirection = generateNextDirection(lastDirection);
						
						// Ajoute cette nouvelle direction à la séquence pour le prochain niveau
						addDirectionToSequence(newDirection);
						level++; // Augmentez le niveau
        } else {
            // Le joueur a échoué, fin du jeu
						displayMultipleArrows(up_image_data, 120,120, 3);
						restartGame();
						playGame();
            break;
        }
    }
}

/*
FOnction de jeu avec quatre niveaux directions prédéfinies : haut bas gauche droite
void playGame(void) {
    int level = 1; // Commence au niveau 1
    initGame();
    displayMultipleArrows(up_image_data, 120, 120, 5);
    TFTinit();

    // Réinitialisez la longueur de la séquence à chaque début de jeu
    sequenceLength = 0;

    while (level <= 4) {
        // Ajouter une nouvelle direction à la séquence pour chaque niveau
        switch (level) {
            case 1:
                addDirectionToSequence(UP);
                break;
            case 2:
                addDirectionToSequence(DOWN);
                break;
            case 3:
                addDirectionToSequence(LEFT);
                break;
            case 4:
                addDirectionToSequence(RIGHT);
                break;
        }

        displaySequence(); // Affichez la séquence à l'écran
        if (readPlayerInput()) { // Vérifiez si l'entrée du joueur correspond à la séquence
            level++; // Augmentez le niveau
        } else {
            // Le joueur a échoué, fin du jeu
            //displayMultipleArrows(up_image_data, 120, 120, 20);
            break;
        }
    }
}
*/

int main(void) {
    playGame();
    return 0;
}
