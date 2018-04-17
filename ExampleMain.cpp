#include "Spritesheet.h"
#include <iostream>

int main()
{
	Spritesheet sheet("bin\\textures", true, SpritesheetTextureNameStorageEnum::FILENAME, false);

	std::cout << "Generating spritesheet" << std::endl;

	if (!sheet.generate())
	{
		std::cout << sheet.getErrors() << std::endl;
	}
	else
	{
		std::cout << "Spritesheet generated" << std::endl;
	}

	const auto uv = sheet.getUv("my-texture");

	std::cout << uv.x << ", " << uv.y << ", " << uv.z << ", " << uv.w << std::endl;

	return 0;
}
