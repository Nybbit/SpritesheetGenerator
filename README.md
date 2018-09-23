# Spritesheet Generator

**What is this?**

This code searches for PNGs in a specified directory and packs them together into a single spritesheet. It also stores the data so you can easily retrieve the UV for a specific texture.

This is meant to be modified for your own usage. (For example, generating an OpenGL texture for the spritesheet)

Example code
```cpp
// Make the spritesheet
Spritesheet mySheet = Spritesheet(
	"bin/textures",                               // Directory to search [Optional]
	 true,                                        // Search sub directories? [Optional]
	 SpritesheetTextureNameStorageEnum::FILENAME, // How texture names are stored [Optional]
	 false                                        // Use file extensions? [Optional]
	 );

// Generate the spritesheet and check for errors
try
{
	mySheet.generate();
	mySheet.exportSpritesheet("spritesheet.png");
}
catch (const std::runtime_error& err)
{
	std::cout << "[ERROR] " << err.what() << std::endl;
}

// Get UV by texture name (x, y, width, height)
glm::vec4 uv = mySheet.getUv("my-texture-name");
```

Example spritesheet

![Example](https://i.imgur.com/W26KOIu.png)

## Dependencies

**Required**
- [lodePNG](http://lodev.org/lodepng/) (for PNG decoding and encoding)

**Optional**
- GLM (for `vec2`, `ivec2`, `vec4`, `ivec4`, etc. Easily replacable)

## Building

> Requires a compiler that supports C++17 (for filesystem). Alternatively, use boost's filesystem