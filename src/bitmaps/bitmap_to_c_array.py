from PIL import Image
import numpy as np

def rgb888_to_rgb565(rgb):
    r, g, b = rgb
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def bmp_to_c_array(bmp_filename, output_filename):
    # Charger l'image
    img = Image.open(bmp_filename)
    # Redimensionner l'image à la taille cible de 128x128 pixels
    img = img.resize((128, 128), Image.ANTIALIAS)
    img = img.convert('RGB')
    img_data = np.array(img)
    height, width, _ = img_data.shape

    # Écrire les données dans un fichier de sortie en format C array
    with open(output_filename, 'w') as f:
        f.write(f"const unsigned short up_image_data[{width * height}] = " + "{\n")
        count = 0
        for y in range(height):
            for x in range(width):
                rgb = img_data[y, x]
                rgb565 = rgb888_to_rgb565(rgb)
                if count % 8 == 0:
                    f.write("\n    ")
                f.write(f"0x{rgb565:04X}, ")
                count += 1
        f.write("\n};\n")
        f.write(f"const unsigned int up_image_width = {width};\n")
        f.write(f"const unsigned int up_image_height = {height};\n")

# Appeler la fonction pour convertir un fichier BMP en un tableau C
bmp_to_c_array("up_bitmap.bmp", "up_image_data.h")
print("DONE")
