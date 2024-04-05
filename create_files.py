import os
src_path = './src'

files = [
    "main.c",
    "game_logic.c", "game_logic.h",
    "lcd_display.c", "lcd_display.h",
    "button_input.c", "button_input.h",
    "audio_control.c", "audio_control.h"
]

for file in files:
    file_path = os.path.join(src_path, file)
    with open(file_path, 'w') as f:
        if file.endswith('.c'):
            f.write('#include "{}"\n'.format(file.replace('.c', '.h')))
            f.write('#include "FreeRTOS/Source/include/FreeRTOS.h"\n')
        elif file.endswith('.h'):
            f.write('#ifndef {}_H\n#define {}_H\n\n#endif //{}_H\n'.format(
                file.upper().replace('.', '_'),
                file.upper().replace('.', '_'),
                file.upper().replace('.', '_')
            ))

print("Les fichiers ont été créés avec les includes nécessaires dans le dossier /src.")
