#include "Spritesheet.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <fstream>
#include <iostream>

#ifndef LOG_VERBOSE
#define LOG_VERBOSE std::cout << "[VERBOSE] "
#endif
#ifndef LOG_INFO
#define LOG_INFO std::cout << "[INFO] "
#endif
#ifndef LOG_WARN
#define LOG_WARN std::cerr << "[WARN] "
#endif

Node::FitTypeEnum Node::fits(const glm::ivec2 dimensions) const
{
	if (dimensions.x == rectangle.z && dimensions.y == rectangle.w)
	{
		return PERFECT_FIT;
	}
	if (dimensions.x <= rectangle.z && dimensions.y <= rectangle.w)
	{
		return EXTRA_SPACE;
	}
	return DOES_NOT_FIT;
}

Node* Node::insert(TextureData& data)
{
	// if not a leaf
	if (children[0] != nullptr && children[1] != nullptr)
	{
		// try inserting into first child
		const auto newNode = children[0]->insert(data);
		if (newNode != nullptr)
		{
			return newNode;
		}

		// otherwise insert into second child
		return children[1]->insert(data);
	}
	else // if leaf
	{
		// if there's already a texture here
		if (textureData != nullptr)
		{
			return nullptr;
		}

		const auto fitType = fits(data.dimensions);

		// if too small
		if (fitType == DOES_NOT_FIT)
		{
			// grow node

			return nullptr;
		}

		// if fits perfectly
		if (fitType == PERFECT_FIT)
		{
			return this;
		}

		// otherwise need to split due to extra room
		this->children[0] = std::make_unique<Node>();
		this->children[1] = std::make_unique<Node>();

		// decide which way to split
		const auto dw = rectangle.z - data.dimensions.x; // delta width
		const auto dh = rectangle.w - data.dimensions.y; // delta height

		if (dw > dh)
		{
			// make rect for tex to fit into
			children[0]->rectangle = glm::ivec4(rectangle.x, rectangle.y, data.dimensions.x, rectangle.w);

			// make rect that a different tex could possibly go into
			children[1]->rectangle = glm::ivec4(rectangle.x + rectangle.z - dw, rectangle.y, dw, rectangle.w);
		}
		else
		{
			// same thing but with height instead of width
			children[0]->rectangle = glm::ivec4(rectangle.x, rectangle.y, rectangle.z, data.dimensions.y);
			children[1]->rectangle = glm::ivec4(rectangle.x, rectangle.y + rectangle.w - dh, rectangle.z, dh);
		}

		// put tex in child1
		return this->children[0]->insert(data);
	}
}

Spritesheet::Spritesheet()
	: m_imageTypeFlags(0)
{}

Spritesheet::Spritesheet(const std::string& directory, const unsigned image_type_flags)
	: m_directory(directory), m_imageTypeFlags(image_type_flags)
{
	generate();
	m_initialized = true;
}

glm::vec4 Spritesheet::getUv(const std::string& texture_name)
{
	if (!m_initialized)
	{
		LOG_WARN << "Spritesheet has not been initialized, could not find texture '" << texture_name << "'" << std::endl;
		return glm::vec4(0);
	}
	const auto it = m_elements.find(texture_name);
	if (it == m_elements.end())
	{
		LOG_WARN << "'" << texture_name << "' was not found in spritesheet" << std::endl;
		return glm::vec4(0);
	}
	return it->second;
}

void Spritesheet::exportSpritesheet(const std::string& directory)
{
	LOG_VERBOSE << "Exporting spritesheet" << std::endl;

	std::experimental::filesystem::create_directory(directory);
	std::ofstream out(directory + "/data.dat");
	for (auto& e : m_elements)
	{
		out << e.first << ";";
		for (auto i = 0; i < 4; i++)
		{
			out << e.second[i] << ";";
		}
		out << "\n";
	}
	out.close();

	// Export png
	if (stbi_write_png((directory + std::string("/image.png")).c_str(),
		m_spritesheetDimensions.x, m_spritesheetDimensions.y, 4,
		m_pixels.data(), m_spritesheetDimensions.x * 4))
	{
		LOG_VERBOSE << "Successfully exported spritesheet" << std::endl;
	}
	else
	{
		LOG_INFO << "Failed to export spritesheet from '" << directory << "/'" << std::endl;
	}
}

void Spritesheet::importSpritesheet(const std::string& directory)
{
	LOG_VERBOSE << "Importing spritesheet from '" << directory << "/'" << std::endl;
	{
		std::ifstream image(directory + "/image.png", std::ios::in | std::ios::binary);
		if (!image.good())
		{
			LOG_WARN << "Unable to load image.png" << std::endl;
			return;
		}
		m_pixels = std::vector<uint8_t>(std::istream_iterator<char>(image), std::istream_iterator<char>());
	}

	std::string line;
	std::ifstream data(directory + "/data.dat");
	if (!data.good())
	{
		LOG_WARN << "Unable to load data.bat" << std::endl;
		return;
	}

	while (std::getline(data, line))
	{
		std::string textureName;
		glm::vec4 uv;

		auto element = 0;
		std::string current;
		for (auto i = 0u; i < line.size() && element < 5; i++)
		{
			if (line[i] == ';')
			{
				if (element == 0)
				{
					textureName = current;
				}
				else
				{
					uv[element - 1] = std::stof(current);
				}
				element++;
				current = "";
			}
			else
			{
				current += line[i];
			}
		}

		m_elements.emplace(textureName, uv);
	}

	LOG_VERBOSE << "Successfully imported spritesheet" << std::endl;

	m_initialized = true;
}

void Spritesheet::generate()
{
	for (const auto& p : std::experimental::filesystem::recursive_directory_iterator(m_directory))
	{
		addTexture(p);
	}
	packTextures();
	cleanup();
}

void Spritesheet::addTexture(const std::experimental::filesystem::directory_entry& p)
{
	auto pathStr = p.path().string();
	std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
	auto extension = p.path().extension().string();
	const auto strippedFileName = pathStr.substr(m_directory.size() + 1, pathStr.size() - m_directory.size() - 1 - extension.size());
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	// Check if directory entry is a valid image
	if (!is_directory(p.path())
		&& ((m_imageTypeFlags & PNG && extension == ".png")
			|| (m_imageTypeFlags & JPEG && (extension == ".jpg" || extension == ".jpeg"))
			|| (m_imageTypeFlags & BMP && extension == ".bmp")))
	{
		LOG_VERBOSE << "Processing '" << strippedFileName << "'" << std::endl;

		/*
		 * Get image data, image data isn't kept in memory because
		 * if you have a lot of large textures, you'll end up using
		 * a lot of memory fast
		 */
		int width, height, channels;
		const auto data = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);
		if (data == nullptr)
		{
			LOG_WARN << "Failed to load image " << strippedFileName << std::endl;
			return;
		}
		stbi_image_free(data);

		// Check if a texture with the same name is already processed
		const auto textureData = TextureData(pathStr, strippedFileName, glm::ivec2(width, height), channels);
		const auto it = std::find(m_unprocessedTextures.begin(), m_unprocessedTextures.end(), textureData);
		if (it != m_unprocessedTextures.end())
		{
			// Note: You could avoid this by not stripping the extension
			LOG_WARN << "'" << strippedFileName << "' already exists. Skipping " << extension << " version" << std::endl;
			return;
		}

		m_unprocessedTextures.push_back(textureData);
	}
}

void Spritesheet::packTextures()
{
	if (m_unprocessedTextures.empty()) return;

	// Sort from longest sides to shortest sides
	std::sort(m_unprocessedTextures.begin(), m_unprocessedTextures.end());

	m_root = std::make_unique<Node>();
	m_spritesheetDimensions = m_unprocessedTextures[0].dimensions;
	m_root->rectangle = glm::ivec4(0, 0, m_spritesheetDimensions);

	for (auto& t : m_unprocessedTextures)
	{
		LOG_VERBOSE << "Packing '" << t.textureName << "' (" << t.dimensions.x << ", " << t.dimensions.y << ")" << std::endl;

		const auto texNode = m_root->insert(t);
		if (texNode != nullptr)
		{
			t.position = glm::ivec2(texNode->rectangle.x, texNode->rectangle.y);
			texNode->textureData = &t;
		}
		else
		{
			/*
			 * Growing code adapted from https://codeincomplete.com/posts/bin-packing/
			 */
			const auto canGrowDown = t.dimensions.x <= m_spritesheetDimensions.x;
			const auto canGrowRight = t.dimensions.y <= m_spritesheetDimensions.y;

			const auto shouldGrowRight = canGrowRight && (m_spritesheetDimensions.y >= m_spritesheetDimensions.x + t.dimensions.x);
			const auto shouldGrowDown = canGrowDown && (m_spritesheetDimensions.x >= m_spritesheetDimensions.y + t.dimensions.y);

			auto grewDown = false;

			// Figure out which way to grow
			if (shouldGrowDown)
			{
				m_spritesheetDimensions.y += t.dimensions.y;
				grewDown = true;
			}
			else if (shouldGrowRight)
			{
				m_spritesheetDimensions.x += t.dimensions.x;
			}
			else if (canGrowDown)
			{
				m_spritesheetDimensions.y += t.dimensions.y;
				grewDown = true;
			}
			else if (canGrowRight)
			{
				m_spritesheetDimensions.x += t.dimensions.x;
			}
			else
			{
				throw std::runtime_error("Could not grow spritesheet (this shouldn't happen)");
			}

			auto newRoot = std::make_unique<Node>();
			newRoot->rectangle = glm::ivec4(0, 0, m_spritesheetDimensions);

			newRoot->children[0] = std::move(m_root); // make m_root one of newRoot's children

			// make other child
			newRoot->children[1] = std::make_unique<Node>();

			// Modify rectanges so that they're correct
			if (grewDown)
			{
				newRoot->children[1]->rectangle = glm::ivec4(0, newRoot->children[0]->rectangle.w, newRoot->children[0]->rectangle.z, t.dimensions.y);
			}
			else
			{
				newRoot->children[1]->rectangle = glm::ivec4(newRoot->children[0]->rectangle.z, 0, t.dimensions.x, newRoot->children[0]->rectangle.w);
			}

			m_root = std::move(newRoot); // make newRoot m_root

			// now image can be inserted into node successfully
			const auto node = m_root->insert(t);
			if (node != nullptr)
			{
				t.position = glm::ivec2(node->rectangle.x, node->rectangle.y);
				node->textureData = &t;
			}
			else
			{
				LOG_WARN << "Could not place image in spritesheet (this shouldn't happen)";
				return;
			}
		}
	}

	const auto w = m_spritesheetDimensions.x;
	const auto h = m_spritesheetDimensions.y;
	m_pixels.resize(w * h * 4); // Each pixel is 4 bytes (rgba)

	for (auto& t : m_unprocessedTextures)
	{
		LOG_VERBOSE << "Adding '" << t.textureName << "' to spritesheet" << std::endl;

		auto pixelCol = 0;
		auto pixelRow = 0;

		const auto data = stbi_load(t.path.c_str(), &t.dimensions.x, &t.dimensions.y, &t.channels, 4);

		if (data != nullptr)
		{
			for (auto i = 0; i < t.dimensions.x * t.dimensions.y * 4; i++)
			{
				const auto pixelIndex = (t.position.x * 4 + pixelCol) + (t.position.y + pixelRow) * (w * 4);

				m_pixels[pixelIndex] = data[i];

				pixelCol++;

				if (pixelCol == t.dimensions.x * 4)
				{
					pixelCol = 0;
					pixelRow++;
				}
			}
		}

		stbi_image_free(data);
		m_elements.emplace(t.textureName, glm::vec4(t.position, t.dimensions) / glm::vec4(w, h, w, h));
	}
}

void Spritesheet::cleanup()
{
	std::vector<TextureData> temp;
	std::swap(temp, m_unprocessedTextures);

	m_root = nullptr;
}
