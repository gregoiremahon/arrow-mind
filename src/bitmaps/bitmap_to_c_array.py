from PIL import Image
import numpy as np

def rgb888_to_rgb565(rgb):
    r, g, b = rgb
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def subsample_image_data(img_data, factor):
    # Sous-échantillonner l'image par un facteur donné
    return img_data[::factor, ::factor]

def bmp_to_c_array(bmp_filename, output_filename, max_width=239, max_height=319, subsample_factor=2):
    img = Image.open(bmp_filename)
    img.thumbnail((max_width, max_height), Image.ANTIALIAS)
    img = img.convert('RGB')
    img_data = np.array(img)
    height, width, _ = img_data.shape

    # Appliquer le sous-échantillonnage
    img_data = subsample_image_data(img_data, subsample_factor)
    subsampled_height, subsampled_width, _ = img_data.shape

    with open(output_filename, 'w') as f:
        f.write(f"const unsigned short up_image_data[{subsampled_width * subsampled_height}] = " + "{\n")
        count = 0
        for y in range(subsampled_height):
            for x in range(subsampled_width):
                rgb = img_data[y, x]
                rgb565 = rgb888_to_rgb565(rgb)
                if count % 8 == 0:
                    f.write("\n    ")
                f.write(f"0x{rgb565:04X}, ")
                count += 1

        f.write("\n};\n")
        f.write(f"const unsigned int up_image_width = {subsampled_width};\n")
        f.write(f"const unsigned int up_image_height = {subsampled_height};\n")

bmp_to_c_array("up_bitmap.bmp", "up_image_data.h")
