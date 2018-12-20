# Spritesheet Generator

**What is this?**

This code generates a spritesheet from textures in a specified directory. It also has simple serialization so you can import and already generated spritesheet. Feel free to use this in *any* of your projects and modify the code, attribution would be nice but it's not required :simple_smile:

## Example

**Code**
```cpp
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
```

**Spritesheet output**

![Example](https://i.imgur.com/W26KOIu.png)

## Dependencies

**Required**
- [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) (for image decoding)
- [stb_image_write.h](https://github.com/nothings/stb/blob/master/stb_image_write.h) (for image encoding)
- [glm](https://glm.g-truc.net) (for `ivec2` and `vec4` - easily replacable)

## Building

> Requires a compiler that supports C++17 (for filesystem). Alternatively, use boost's filesystem
