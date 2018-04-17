# Spritesheet Generator

**What is this?**

This code searches for PNGs in a specified directory and packs them together into a single spritesheet. It then stores the data so you can easily retrieve the UV of a texture.

Example code
```cpp
// Make the spritesheet
Spritesheet mySheet(
	"bin\\textures",                              // Directory to search [Optional]
	 true,                                        // Search sub directories? [Optional]
	 SpritesheetTextureNameStorageEnum::FILENAME, // How texture names are stored [Optional]
	 false                                        // Use file extensions? [Optional]
	 );

// Generate the spritesheet and check for errors
bool success = mySheet.generate();
if (!success) { std::cout << mySheet.getErrors() << std::endl; }

// Get UV by texture name
glm::vec4 uv = mySheet.getUv("my-texture-name");
```

Example spritesheet

![Example](https://i.imgur.com/W26KOIu.png)

## Dependencies

**Required**
- [lodePNG](http://lodev.org/lodepng/) (for PNG decoding and encoding)

**Optional**
With some simple changes, you can get rid of these dependencies but I used them because this code is almost exactly the same as the code I use in my games.

- Boost (for filesystem and string checking, can easily be replaced with C++17 filesystem and basic code)
- GLM (for vectors, can easily be replaced by a custom type)