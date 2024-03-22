import numpy as np

def draw_arrow(direction, width, height):
    """Dessine une flèche dans une image numpy array (RGB565)."""
    image = np.zeros((height, width), dtype=np.uint16)  # Fond transparent (ou blanc)
    arrow_color = 0xFFFF  # Blanc

    # Centre et taille de la flèche
    cx, cy = width // 2, height // 2
    arrow_size = min(width, height) // 3

    if direction == 'up':
        points = [(cx, cy - arrow_size), (cx - arrow_size, cy + arrow_size), (cx + arrow_size, cy + arrow_size)]
    elif direction == 'down':
        points = [(cx, cy + arrow_size), (cx - arrow_size, cy - arrow_size), (cx + arrow_size, cy - arrow_size)]
    elif direction == 'left':
        points = [(cx - arrow_size, cy), (cx + arrow_size, cy - arrow_size), (cx + arrow_size, cy + arrow_size)]
    elif direction == 'right':
        points = [(cx + arrow_size, cy), (cx - arrow_size, cy - arrow_size), (cx - arrow_size, cy + arrow_size)]
    else:
        raise ValueError("Direction non reconnue.")

    # Dessiner la flèche (simplifié pour l'exemple)
    for x, y in points:
        image[y][x] = arrow_color

    return image

def save_to_c_array(image, direction, output_filename):
    """Enregistre l'image numpy array en tant que tableau C."""
    height, width = image.shape
    with open(output_filename, 'a') as f:
        f.write(f"// Flèche {direction}\n")
        f.write(f"const unsigned short arrow_{direction}_data[] = {{\n    ")
        for y in range(height):
            for x in range(width):
                f.write(f"0x{image[y, x]:04X}, ")
            f.write("\n    ")
        f.write("};\n\n")

# Exemple d'utilisation
directions = ['up', 'down', 'left', 'right']
output_filename = "arrows_data.h"

for direction in directions:
    image = draw_arrow(direction, 32, 32)  # 32x32 est un exemple
    save_to_c_array(image, direction, output_filename)
