from PIL import Image

def convert_png_to_h(input_image_path, output_h_path, target_size=(480, 480)):
    try:
        # Ouvrir l'image PNG
        image = Image.open(input_image_path)
    except FileNotFoundError:
        print("Fichier d'image introuvable.")
        return
    except Exception as e:
        print(f"Erreur lors de l'ouverture de l'image : {e}")
        return

    # Redimensionner l'image à la taille cible
    image = image.resize(target_size)

    # Vérifier que l'image est au format RGB
    if image.mode != 'RGB':
        image = image.convert('RGB')

    # Obtenir la taille de l'image redimensionnée
    width, height = image.size

    # Ouvrir le fichier de sortie .h
    with open(output_h_path, 'w') as output_file:
        output_file.write('#ifndef IMAGE_H\n')
        output_file.write('#define IMAGE_H\n\n')
        output_file.write(f'const uint16_t image_width = {width};\n')
        output_file.write(f'const uint16_t image_height = {height};\n')
        output_file.write('const uint16_t image_data[] = {\n')

        # Parcourir tous les pixels de l'image
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                output_file.write(f'0x{rgb565:04X}, ')

        output_file.write('\n};\n\n')
        output_file.write('#endif // IMAGE_H\n')

    print(f'Conversion terminée. Fichier {output_h_path} généré.')

# Utilisation
nom = '250px-Pikachu-DEPS'
image_path = nom + '.png'
output_h_path = nom + '.h'
convert_png_to_h(image_path, output_h_path)
