#include "../Spritesheet.h"
#include <iostream>

int main()
{
	Spritesheet sheet = Spritesheet("bin/textures", true, SpritesheetTextureNameStorageEnum::FILENAME, false);

	std::cout << "Generating spritesheet" << std::endl;

	try
	{
		sheet.generate();
		sheet.exportSpritesheet("out.png");
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "[ERROR] " << err.what() << std::endl;
		return 1;
	}

	std::cout << "Spritesheet generated" << std::endl;

	const auto uv = sheet.getUv("c");

	std::cout << uv.x << ", " << uv.y << ", " << uv.z << ", " << uv.w << std::endl;

	return 0;
}
