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

// Lecture Bitmap
extern const unsigned char image_palette[256 * 3];

extern const uint16_t imageData[];
extern const int imageWidth;
extern const int imageHeight;

// Directions possibles des flèches à l'écran.
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} ArrowDirection;

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

void displayMultipleArrows(const uint16_t *imageData, int width, int height) {
    int delay;
    ArrowDirection currentDirection = UP; 

    while (1) {
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

int main(void) {
		uint64_t i;
    SystemInit(); // Initialisation du système
    TFTinit(); // Initialisation de l'écran TFT
		
	/* COLOR SHIFT TEST
    fillScreen(RED); // Remplissage de l'écran en rouge
		for (i = 0; i < 5000000; i++);
		fillScreen(CYAN);
		for (i = 0; i < 5000000; i++);
		fillScreen(BLUE);
		for (i = 0; i < 5000000; i++);
		fillScreen(GRAY2);
		for (i = 0; i < 5000000; i++);
		fillScreen(YELLOW);
		for (i = 0; i < 5000000; i++);
		fillScreen(BLACK);
		for (i = 0; i < 5000000; i++);
		fillScreen(YELLOW);
		fillScreen(RED);
	*/
		/*// Affichage de l'image BMP en mode UP LEFT DOWN RIGHT LEFT
		for (i = 0; i < 5000000; i++);
		//displayImage(up_image_data, 0, 0, up_image_width, up_image_width);
		displayArrow(up_image_data, 0, 0, up_image_width, up_image_height, UP); // pas de rotation
		for (i = 0; i < 5000000; i++);
		for (i = 0; i < 5000000; i++);
		displayArrow(up_image_data, 0, 120, up_image_width, up_image_height, LEFT); // s'affiche a gauche de la premiere image
		for (i = 0; i < 5000000; i++);
		for (i = 0; i < 5000000; i++);
		displayArrow(up_image_data, 120, 0, up_image_width, up_image_height, LEFT); // s'affiche a gauche de la premiere image
		for (i = 0; i < 5000000; i++);
		for (i = 0; i < 5000000; i++);
		displayArrow(up_image_data, 120, 0, up_image_width, up_image_height, LEFT); // s'affiche a gauche de la premiere image
		for (i = 0; i < 5000000; i++);
		for (i = 0; i < 5000000; i++);
		displayArrow(up_image_data, 0, 0, up_image_width, up_image_height, DOWN); 
		for (i = 0; i < 5000000; i++);
		for (i = 0; i < 5000000; i++);
		displayArrow(up_image_data, 0, 0, up_image_width, up_image_height, RIGHT); 
		for (i = 0; i < 5000000; i++);
		for (i = 0; i < 5000000; i++);
		displayArrow(up_image_data, 0, 0, up_image_width, up_image_height, LEFT); */
		displayMultipleArrows(up_image_data, 120,120);

    while (1) {
        // Boucle infinie
    }
}
