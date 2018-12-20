#include "../Spritesheet.h"

int main()
{
	/*
	 * Generate a spritesheet from textures with the
	 * extensions PNG, JPEG, and BMP in the
	 * "texture" directory
	 */
	auto sheet = Spritesheet("bin/textures", PNG | JPEG | BMP);
	sheet.exportSpritesheet("spritesheetData"); // Export the spritesheet to folder 'spritesheetData' for easy import later
	sheet.getUv("subfolder/a"); // Get texture 'bin/textures/subfolder/a.png'

	// Create an ungenerated spritesheet for import
	auto sheet2 = Spritesheet();
	/*
	 * Import spritesheet from folder 'spritesheetData'
	 * Note: you don't need the original images if you're importing,
	 * you only need image.png and data.dat (which are generated
	 * by exportSpritesheet())
	 */
	sheet2.importSpritesheet("spritesheetData");
	sheet.getUv("subfolder/a"); // Get texture
	return 0;
}
