#pragma once
#ifndef NYBBIT_SPRITESHEET_H
#define NYBBIT_SPRITESHEET_H

#include <string>
#include <vector>
#include <map>
#include <experimental/filesystem>

#include <glm/glm.hpp>

struct TextureElement
{
	/**
	 * \brief Holds path and resolution of a texture for packing
	 * \param texture_name Name of texture
	 * \param resolution Resolution of image
	 * \param pixels Raw pixel data of texture
	 */
	explicit TextureElement(std::string texture_name, glm::ivec2 resolution, std::vector<unsigned char> pixels);

	friend bool operator<(const TextureElement& a, const TextureElement& b);

	std::string textureName;
	glm::ivec2 resolution;
	std::vector<unsigned char> pixels;

	// Set manually
	glm::ivec2 position;
};

/*
 * Packing code adapted from http://blackpawn.com/texts/lightmaps/default.html
 */
struct Node
{
	std::unique_ptr<Node> children[2] { nullptr, nullptr };
	glm::ivec4 rectangle;
	TextureElement* texture = nullptr;

	enum FitTypeEnum
	{
		DOES_NOT_FIT, PERFECT_FIT, EXTRA_SPACE
	};

	FitTypeEnum fits(glm::ivec2 rect_dims) const;

	Node* insert(TextureElement& tex);
};

/**
 * \brief How texture names are stored and to be referred to
 */
enum class SpritesheetTextureNameStorageEnum
{
	/*
	 * The full path relative to the exe
	 *
	 * Example:
	 * "bin\textures\particles\explosion\splode_frame_0001.png"
	 */
	FULL_PATH,

	/*
	 * The full path not including the spritesheet directory
	 *
	 * Example (assuming the directory is "bin\textures"):
	 * "particles\explosion\splode_frame_0001.png"
	 */
	PARTIAL_PATH,

	/*
	 * Just the file name of the image
	 *
	 * Example:
	 * "splode_frame_0001.png"
	 */
	FILENAME
};

class Spritesheet
{
public:
	/**
	 * \brief Constructors spritesheet set to current directory
	 */
	explicit Spritesheet(bool search_sub_directories = false,
		SpritesheetTextureNameStorageEnum spritesheet_texture_name_storage =
		SpritesheetTextureNameStorageEnum::FULL_PATH, bool use_file_extensions = true);

	/**
	 * \brief Constructs spritesheet with specified directory
	 * \param directory Directory to search for PNGs
	 * \param search_sub_directories Whether to search subdirectories for PNGs
	 * \param spritesheet_texture_name_storage How texture naems are stored
	 * \param use_file_extensions Whether extensions will be a part of texture names
	 */
	explicit Spritesheet(std::string directory, bool search_sub_directories,
		SpritesheetTextureNameStorageEnum spritesheet_texture_name_storage =
		SpritesheetTextureNameStorageEnum::FULL_PATH, bool use_file_extensions = true);

	/**
	 * \brief Generates a spritesheet from all PNGs in directory
	 */
	void generate();

	/**
	 * \brief Get the UV of a texture by filename in the format (x, 1.0f - y, width, height) normalized [0-1]
	 * \param filename Name of texture to get
	 * \return uv
	 */
	glm::vec4 getUv(const std::string& filename) const;

	/**
	 * \brief Exports the spritesheet's image data to a file in the PNG format
	 */
	void exportSpritesheet(const std::string& filename) const;
private:
	/**
	 * \brief Adds a png to the spritesheet
	 * \param p Directory entry of png to add to spritesheet
	 */
	void addTexture(const std::experimental::filesystem::directory_entry& p);

	/**
	 * \brief Packs textures together and creates spritesheet
	 */
	void packTextures();

	std::string m_directory;

	bool m_searchSubdirs;

	SpritesheetTextureNameStorageEnum m_spritesheetTextureNameStorage;

	bool m_useExtensions;

	std::vector<TextureElement> m_textures;

	std::map<std::string, glm::vec4> m_elements;

	std::vector<unsigned char> m_pixels;

	glm::ivec2 m_spritesheetResolution;

	std::unique_ptr<Node> m_root;
};
#endif
