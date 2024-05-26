#include "LPC17xx.h"
#include "images_data.h"
#include <stdlib.h> // Pour rand() et srand()
#include <time.h>   // Pour time(), afin d'initialiser rand()
#include <stdio.h> // pour sprintf()
#include <string.h> // Pour memcpy()

#include "LPC17xx.h"

// Définitions des broches pour l'écran TFT
#define PIN_RST  (1 << 4)  // Reset - P0.4
#define PIN_CS   (1 << 5)  // Chip Select - P0.5
#define PIN_DC   (1 << 10) // Data/Command - P0.10
#define PIN_WR   (1 << 11) // Write - P0.11
#define PIN_RD   (1 << 13) // Read - P2.13

#define P0_MASK(bit) (1 << bit) // Macro pour créer un masque pour le port 0

// Définition des masques pour les broches de données D0-D7 sur le port 2
#define LCD_DATA_MASK (0xFF) // D0-D7 sur P2.0 à P2.7


#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#define RED_SCORE     RGB565(255, 0, 0)


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

// Gestion  de l'affichage de caractères
#include <stdint.h>

#define CHAR_WIDTH  5
#define CHAR_HEIGHT 8
#define NUM_CHARS 16

// ports des boutons en fonction de leurs directions (sur le port P0.x)

#define UP_BUTTON    6  // Bleu
#define DOWN_BUTTON  7  // Gris	
#define LEFT_BUTTON  9  // Rouge
#define RIGHT_BUTTON 8  // Blanc
#define MID_BUTTON 0 //  Noir

#define PCLK_UART0 60000000UL  // Définir selon la fréquence du système
#define BAUDRATE 9600          // Vitesse de transmission désirée  


// Séquence de directions 
#define MAX_SEQUENCE 15 // Longueur maximale de la séquence

// chiffres 0-9 et des caractères "SCORE: " en 5x8 pixels pour un affichage en noir et blanc
static uint8_t extended_font[][CHAR_HEIGHT] = {
    {0x1F, 0x11, 0x11, 0x11, 0x1F}, // 0
    {0x00, 0x00, 0x1F, 0x00, 0x00}, // 1
    {0x1D, 0x15, 0x15, 0x15, 0x17}, // 2
    {0x15, 0x15, 0x15, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x04, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x15, 0x15, 0x1D}, // 5
    {0x1F, 0x15, 0x15, 0x15, 0x1D}, // 6
    {0x01, 0x01, 0x01, 0x01, 0x1F}, // 7
    {0x1F, 0x15, 0x15, 0x15, 0x1F}, // 8
    {0x17, 0x15, 0x15, 0x15, 0x1F}, // 9
		
    {0x00, 0x14, 0x14, 0x14, 0x1F}, // S
    {0x00, 0x11, 0x11, 0x11, 0x1F}, // C
    {0x00, 0x15, 0x15, 0x15, 0x1F}, // O
    {0x00, 0x15, 0x15, 0x0A, 0x1F}, // R
    {0x00, 0x05, 0x05, 0x05, 0x1F}, // E
    {0x00, 0x00, 0x1F, 0x00, 0x00}  // :
};


void rotateExtendedFont180(int numTurns) {
    uint8_t temp[CHAR_HEIGHT];
		int n, i, j;
		
    int numEffectiveTurns = numTurns % 2;

    for (n = 0; n < numEffectiveTurns; n++) {
        for (i = 0; i < NUM_CHARS; i++) {
            // Copier le caractère actuel pour travailler dessus
            memcpy(temp, extended_font[i], CHAR_HEIGHT);
            
            // Inverser les lignes
            for (j = 0; j < CHAR_HEIGHT; j++) {
                extended_font[i][j] = temp[CHAR_HEIGHT - 1 - j];
            }
        }
    }
}

// Directions possibles des flèches à l'écran.
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} ArrowDirection;


ArrowDirection sequence[MAX_SEQUENCE];
int sequenceLength = 0; // Longueur actuelle de la séquence

// Lecture Bitmap
extern const unsigned char image_palette[256 * 3];

extern const uint16_t imageData[];
extern const int imageWidth;
extern const int imageHeight;

volatile int currentScore = 0;
volatile int bestScore = 0;

volatile uint32_t gen_alea;



// Fonction pour configurer l'UART
void initUART(void) {
    LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6); // Configurer les broches P0.2 et P0.3 comme TXD0 et RXD0 respectivement
    LPC_UART0->LCR = (1 << 7); // Activer le DLAB (Divisor Latch Access Bit) pour permettre la modification du registre DLL et DLM
    LPC_UART0->DLL = 162; // Pour une vitesse de transmission de 9600 bauds avec un PCLK de 25MHz
    LPC_UART0->DLM = 0;
    LPC_UART0->LCR = 3; // Désactiver le DLAB et configurer la taille des données sur 8 bits
    LPC_UART0->FCR = (1 << 0) | (1 << 1) | (1 << 2); // Activer et réinitialiser les FIFOs de l'UART
}

// Fonction pour transmettre une chaîne de caractères via l'UART
void UART_SendString(char *str) {
    while(*str != '\0') {
        while(!(LPC_UART0->LSR & (1 << 5))); // Attendre que le registre de transmission soit vide
        LPC_UART0->THR = *str;
        str++;
    }
}


void displaySequenceAndInput(ArrowDirection currentInput) {
    char *directions[] = {"UP", "DOWN", "LEFT", "RIGHT"};
		int i;
    char buffer[128];  // Augmenter la taille du buffer si nécessaire

    // Afficher la direction courante
    if (currentInput >= UP && currentInput <= LEFT) {
        sprintf(buffer, "Current Input: %s\n", directions[currentInput]);
        UART_SendString(buffer);
    }
}


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
    initButtonPort(7); // Initialiser le port du bouton GRIS (P0.7)
    initButtonPort(8); // Initialiser le port du bouton BLANC (P0.8)
    initButtonPort(9); // Initialiser le port du bouton ROUGE (P0.9)
		initButtonPort(0); // Initialiser le port du bouton NOIR (P0.0)
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

void sendData(uint16_t data) {
    LPC_GPIO0->FIOCLR = PIN_CS;
    LPC_GPIO0->FIOSET = PIN_DC;
    LPC_GPIO2->FIOPIN = (LPC_GPIO2->FIOPIN & ~LCD_DATA_MASK) | (data >> 8);  // High byte
    LPC_GPIO0->FIOCLR = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_WR;
    LPC_GPIO2->FIOPIN = (LPC_GPIO2->FIOPIN & ~LCD_DATA_MASK) | (data & 0xFF);  // Low byte
    LPC_GPIO0->FIOCLR = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_WR;
    LPC_GPIO0->FIOSET = PIN_CS;
}

void setPixel(int x, int y, uint16_t color) {
    setCol(x, x);
    setPage(y, y);
    sendCMD(0x2C);  // Write to RAM
    sendData(color);
}

void displayArrow(const uint16_t *imageData, int x, int y, int width, int height, ArrowDirection direction) {
    int i, j, pixelIndex;
    uint16_t color;

    setArea(x, y, width, height);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            switch (direction) {
                case DOWN: // Base orientation
                    pixelIndex = i * width + j;
                    break;
                case UP: // Rotate 180 degrees
                    pixelIndex = (height - 1 - i) * width + (width - 1 - j);
                    break;
                case LEFT: // Rotate 270 degrees
                    pixelIndex = (width - 1 - j) * height + i;
                    break;
                case RIGHT: // Rotate 90 degrees
                    pixelIndex = j * height + (height - 1 - i);
                    break;
            }

            if (imageData[pixelIndex / 16] & (1 << (pixelIndex % 16))) {
                color = WHITE;
            } else {
                color = BLACK;
            }

            writePixel(color);
        }
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
				displayArrow(imageData, 112, 40, width, height, currentDirection); 
        displayArrow(imageData, 0, 168, width, height, currentDirection); 
        displayArrow(imageData, 112, 168, width, height, currentDirection); 
        
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

void fillRectangle(int x, int y, int width, int height, uint16_t color) {
		int i;
		setCol(x, x + width - 1);
    setPage(y, y + height - 1);
    sendCMD(0x2c); // Commence à écrire dans la mémoire d'affichage

    LPC_GPIO0->FIOSET = PIN_DC; // Mode donnée
    LPC_GPIO0->FIOCLR = PIN_CS; // Sélectionne le TFT
    for (i = 0; i < width * height; i++) {
        sendData(color >> 8); // MSB of color
        sendData(color & 0xFF); // LSB of color
    }
		LPC_GPIO0->FIOSET = PIN_CS; // Désélectionne le TFT
}


// Fonctions d'affichage de caractères
void displayChar(char ch, int x, int y, uint16_t fgColor, uint16_t bgColor, int size) {
    int charIndex = -1;
    int row, col;
    const uint8_t *bitmap;

    // Déterminer l'index du caractère dans la police
    if (ch >= '0' && ch <= '9') {
        charIndex = ch - '0';
    } else if (ch >= 'A' && ch <= 'Z') {
        charIndex = 10 + (ch - 'A');  // Pour les lettres A-Z dans votre tableau
        if (ch == ':') {
            charIndex = 15; // ':' spécifique
        }
    } else {
        return; // Caractère non géré
    }

    bitmap = extended_font[charIndex];

    // Afficher chaque pixel du caractère, mais tourné de 180 degrés horizontalement (axe x)
    for (row = 0; row < CHAR_HEIGHT; row++) {
        for (col = 0; col < CHAR_WIDTH; col++) {
            // Inverser l'ordre des lignes pour retourner de haut en bas
            if (bitmap[row] & (1 << col)) {  // Inversion ici pour retourner autour de l'axe x
                fillRectangle(x + (CHAR_WIDTH - 1 - col) * size, y + row * size, size, size, fgColor);
            } else {
                fillRectangle(x + (CHAR_WIDTH - 1 - col) * size, y + row * size, size, size, bgColor);
            }
        }
    }
}




void drawString(const char* str, int x, int y, uint16_t fgColor, uint16_t bgColor, int size) {
    while (*str) {
        displayChar(*str, x, y, fgColor, bgColor, size);
        y += (CHAR_WIDTH * size);  // Move to the next character slot
        str++;
    }
}



void displayScore(int score, int x, int y, uint16_t fgColor, uint16_t bgColor, int size) {
    char scoreText[20];
    sprintf(scoreText, "%d", score);
    // Clear the area where the score will be displayed
    fillRectangle(x, y, (strlen(scoreText) * CHAR_WIDTH * size), (CHAR_HEIGHT * size), bgColor);
    // Display the score
    drawString(scoreText, x, y, fgColor, bgColor, size);
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

void drawProgressBar(int level) {
	/*
	* Bargraph qui affiche la progression du score actuel. 
	* Un carré reste à la position du meilleur score.
	*/
    int i;
    for (i = 0; i < level; i++) {
        fillRectangle(15, 45 + i * 6, 5, 5, BLUE);
    }
    fillRectangle(15, 45 + bestScore * 6, 5, 5, BLUE); 
}


void displaySequence(void) {
    int i;
    volatile int delay;
    for (i = 0; i < sequenceLength; i++) {
				drawProgressBar(i);
        displayArrow(up_image_data, 56, 96, 128, 128, sequence[i]);
        for (delay = 0; delay < 1000000; delay++); // Délai entre les flèches
    }
}

void addDirectionToSequence(ArrowDirection newDirection) {
    if (sequenceLength < MAX_SEQUENCE) {
        sequence[sequenceLength++] = newDirection;
    }
}

int readPlayerInput(void) {
    int currentInput = -1;
    volatile int delay;
    int attempt;
    for (attempt = 0; attempt < sequenceLength; attempt++) {
        int inputCorrect = 0;
        while (!inputCorrect) {
						 if (readButtonState(MID_BUTTON) == 1) { 
                displaySequence(); // Remontre la séquence entière
							 attempt = 0;
                continue; 
						} else if (readButtonState(UP_BUTTON) == 1) {
                currentInput = UP;
            } else if (readButtonState(DOWN_BUTTON) == 1) {
                currentInput = DOWN;
            } else if (readButtonState(LEFT_BUTTON) == 1) {
                currentInput = LEFT;
            } else if (readButtonState(RIGHT_BUTTON) == 1) {
                currentInput = RIGHT;
            } else {
                currentInput = -1; // Aucune entrée
							gen_alea++;
            }

            if (currentInput != -1) {
                for (delay = 0; delay < 90000*7; delay++); // Anti-rebond
                if ((currentInput == UP && readButtonState(UP_BUTTON) == 0) ||
                    (currentInput == DOWN && readButtonState(DOWN_BUTTON) == 0) ||
                    (currentInput == LEFT && readButtonState(LEFT_BUTTON) == 0) ||
                    (currentInput == RIGHT && readButtonState(RIGHT_BUTTON) == 0)) {
                    displaySequenceAndInput(currentInput);
                    if (currentInput == sequence[attempt]) {
                        inputCorrect = 1;
                    } else {
                        return 0; // Échec si l'entrée est incorrecte
                    }
                }
            }
        }
    }
    return 1; // Succès : toute la séquence a été correctement saisie
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
		initUART(); // initialisation de la liaison série
		UART_SendString("\nDEBUT DE LA PARTIE ARROW MIND...\n\n");
}

void restartGame(void) {
    int flashes = 2;  // Nombre de clignotements
    int i;
		volatile int delay;
		srand(gen_alea);
    for (i = 0; i < flashes; i++) {
        fillScreen(RED);  // Remplit l'écran en rouge
        for (delay = 0; delay < 500000; delay++);  
        fillScreen(GREEN);  // Remplit l'écran en vert
        for (delay = 0; delay < 500000; delay++);  
			  fillScreen(BLUE);  // Remplit l'écran en bleu
        for (delay = 0; delay < 500000; delay++);  
			  fillScreen(BLACK);  // Remplit l'écran en noir
        for (delay = 0; delay < 500000; delay++);  
			  fillScreen(YELLOW);  // Remplit l'écran en jaune
        for (delay = 0; delay < 500000; delay++);  
			  fillScreen(WHITE);  // Remplit l'écran en blanc.
        for (delay = 0; delay < 500000; delay++);
    }

    initGame();  // Réinitialise le jeu
    sequenceLength = 0;
    //addDirectionToSequence(LEFT);
}

void handleGameOver(void){
		if (currentScore > bestScore){
				bestScore = currentScore;
		}
		currentScore = 0;
		// Le joueur a échoué, fin du jeu
		displayMultipleArrows(up_image_data, 128,128, 3);
		restartGame();
		
}
void replayFullSequence(void) {
    int i;
    volatile int delay;

    for (i = 0; i < sequenceLength; i++) {
        displayArrow(up_image_data, 56, 96, 128, 128, sequence[i]);  // Afficher la flèche correspondant à l'index i
        for (delay = 0; delay < 10000000; delay++); // Temps de pause entre les flèches pour une bonne visualisation
    }
}

void playGame(void) {
    int level = 0;
		char buffer[128];
		ArrowDirection newDirection;

		TFTinit();
		restartGame();
	
    while (1) {
			// Gestion de l'aléatoire
			srand(gen_alea);
			rand();
			
			// Gestion de la première direction
			if (sequenceLength == 0) {
					addDirectionToSequence((ArrowDirection)(rand() % 4));
			}
			
			displaySequence(); // Affichez la séquence à l'écran
			displayScore(currentScore, 10, 10, WHITE, BLACK, 3);	
			
			if (readPlayerInput()) { // Vérifiez si l'entrée du joueur correspond à la séquence
						// Verif si la dernière direction de la séquence est correctement obtenue
						ArrowDirection lastDirection = sequenceLength > 0 ? sequence[sequenceLength - 1] : (ArrowDirection)-1;
						// Génère une nouvelle direction différente de la dernière direction de la séquence
						newDirection = generateNextDirection(lastDirection);
				
						sprintf(buffer, "READ PLAYER INPUT OK : %d\n", sequenceLength);
						UART_SendString(buffer);
				
						// Ajoute cette nouvelle direction à la séquence pour le prochain niveau
						addDirectionToSequence(newDirection);
						level++;
						currentScore = level; // mise a jour du score de la partie actuelle
						sprintf(buffer, "Good. Score actuel : %d\n", currentScore);
						UART_SendString(buffer);
						if(newDirection == 0) {
							sprintf(buffer, "\nATTENDU : %d : %s\n", newDirection, "UP");
						} else if(newDirection == 1) {
							sprintf(buffer, "\nATTENDU : %d : %s\n", newDirection, "DOWN");
						} else if(newDirection == 2) {
							sprintf(buffer, "\nATTENDU : %d : %s\n", newDirection, "LEFT");
						} else if(newDirection == 3) {
							sprintf(buffer, "\nATTENDU : %d : %s\n", newDirection, "RIGHT");
						}
						UART_SendString(buffer);
        } else { // Le joueur a échoué, redemarrage du jeu. Nouvelle séquence et score réinitialisé.
						if (currentScore > bestScore){
								bestScore = currentScore;
						}
						level = 0;
						currentScore = 0;
						displayMultipleArrows(up_image_data, 128,128, 3);
						restartGame();
        }
    }
}

int main(void) {
    playGame();
    return 0;
}
