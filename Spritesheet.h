#pragma once
#ifndef NYBBIT_SPRITESHEET_H
#define NYBBIT_SPRITESHEET_H

#include <string>
#include <vector>
#include <map>

#include <boost/filesystem.hpp>
#include <glm/glm.hpp>

struct TextureElement
{
	/**
	 * \brief Holds path and resolution of a texture for packing
	 * \param texture_name Name of texture
	 * \param resolution Resolution of image
	 * \param pixels Raw pixel data of texture
	 */
	explicit TextureElement(std::string texture_name, glm::vec2 resolution, std::vector<unsigned char> pixels);

	friend bool operator<(const TextureElement& a, const TextureElement& b);

	std::string textureName;
	glm::vec2 resolution;
	std::vector<unsigned char> pixels;

	// Set manually
	glm::vec2 position;
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
	 * \return If spritesheet was generated successfully
	 */
	bool generate();

	/**
	 * \brief Get a formatted string with a list of errors
	 * \return Formatted string with a list of errors
	 */
	std::string getErrors() const;

	/**
	 * \brief Get the UV of a texture by filename
	 * \param filename Name of texture to get
	 * \return uv
	 */
	glm::vec4 getUv(const std::string& filename) const;
private:
	/**
	 * \brief Adds a png to the spritesheet
	 * \param p Directory entry of png to add to spritesheet
	 */
	bool addTexture(const boost::filesystem::directory_entry& p);

	/**
	 * \brief Packs textures together and creates spritesheet
	 */
	bool packTextures();

	std::string m_directory;
	bool m_searchSubdirs;
	SpritesheetTextureNameStorageEnum m_spritesheetTextureNameStorage;
	bool m_useExtensions;
	std::vector<std::string> m_errors;

	std::vector<TextureElement> m_textures;

	std::map<std::string, glm::vec4> m_elements;

	glm::ivec2 m_spritesheetResolution;

	std::unique_ptr<Node> m_root;
};
#endif
