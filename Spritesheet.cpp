#include "Spritesheet.h"
#include <utility>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include "lodepng.h"

#define DEBUG_ENABLED false

#if DEBUG_ENABLED
#define DEBUG_PRINT(s) std::clog << "[DEBUG] " << (s) << std::endl;
#else
#define DEBUG_PRINT(s)
#endif

TextureElement::TextureElement(std::string texture_name, const glm::vec2 resolution, std::vector<unsigned char> pixels) :
	textureName(std::move(texture_name)), resolution(resolution), pixels(std::move(pixels))
{
}

Node::FitTypeEnum Node::fits(const glm::ivec2 rect_dims) const
{
	if (rect_dims.x == rectangle.z && rect_dims.y == rectangle.w)
	{
		return PERFECT_FIT;
	}
	if (rect_dims.x <= rectangle.z && rect_dims.y <= rectangle.w)
	{
		return EXTRA_SPACE;
	}
	return DOES_NOT_FIT;
}

Node* Node::insert(TextureElement& tex)
{
	// if not a leaf
	if (children[0] != nullptr && children[1] != nullptr)
	{
		// try inserting into first child
		const auto newNode = children[0]->insert(tex);
		if (newNode != nullptr)
		{
			return newNode;
		}

		// otherwise insert into second child
		return children[1]->insert(tex);
	}
	else // if leaf
	{
		// if there's already a texture here
		if (texture != nullptr) { return nullptr; }

		const auto fitType = fits(tex.resolution);

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
		const auto dw = rectangle.z - tex.resolution.x; // delta width
		const auto dh = rectangle.w - tex.resolution.y; // delta height

		/*
		 *    ASCII PICTURE OF WHAT'S GOING ON HERE
		 *
		 *    Parent node rect
		 *     ____________________
		 *    |                    |
		 *    |                    |
		 *    |                    |
		 *    |        :)          |
		 *    |                    |
		 *    |                    |
		 *    |                    |
		 *    |____________________|
		 *
		 *
		 *
		 *    At this point in the code, the texture that we're trying to put in this rectangle does not fit perfectly
		 *    and there is space left over.
		 *    Example texture that we're putting in the parent node's rect
		 *
		 *     ____________________
		 *    |        |           |
		 *    |  Tex   |           |
		 *    |  Rect  |           |
		 *    |        |           |
		 *    |________|           |
		 *    |                    |
		 *    |  Parent Node Rect  |
		 *    |____________________|
		 *
		 *
		 *    If the difference in width is greater than the difference in height, then split the parent node's rectangle
		 *    into two smaller rectangles. One of these rectangles will have the same width as the texture.
		 *    These two rectangles correspond to the parent node's two children.
		 *
		 *     ____________________
		 *    |        :           |
		 *    |        :           |
		 *    |        :           |
		 *    | child1 :   child2  |
		 *    |        :           |
		 *    |        :           |
		 *    |        :           |
		 *    |________:___________|
		 *       ^
		 *       Same width as texture
		 *
		 *
		 *
		 *    Texture is then inserted into child1
		 *
		 *    child1's rectangle
		 *     ________
		 *    |        |
		 *    |        |
		 *    |        |
		 *    |        |
		 *    |        |
		 *    |        |
		 *    |        |
		 *    |________|
		 *
		 *    Since the texture doesn't perfectly fit, split the rectangle again and make two child nodes
		 *    One of these node's rectangles will PERFECTLY fit the texture in it since now the difference
		 *    in height is greater than the difference in width (0, due to previous step).
		 *    The other node is space for a possible other texture
		 *
		 *     ________
		 *    |        |
		 *    |        |   <--- Texture now takes up this rectangle
		 *    |  Tex   |
		 *    |        |
		 *    |________|
		 *    |        |
		 *    |        |   <--- Free for another texture
		 *    |________|
		 *
		 */

		if (dw > dh)
		{
			// make rect for tex to fit into
			children[0]->rectangle = glm::ivec4(rectangle.x, rectangle.y, tex.resolution.x, rectangle.w);

			// make rect that a different tex could possibly go into
			children[1]->rectangle = glm::ivec4(rectangle.x + rectangle.z - dw, rectangle.y, dw, rectangle.w);
		}
		else
		{
			// same thing but with height instead of width
			children[0]->rectangle = glm::ivec4(rectangle.x, rectangle.y, rectangle.z, tex.resolution.y);
			children[1]->rectangle = glm::ivec4(rectangle.x, rectangle.y + rectangle.w - dh, rectangle.z, dh);
		}

		// put tex in child1
		return this->children[0]->insert(tex);
	}
}

Spritesheet::Spritesheet(const bool search_sub_directories, const SpritesheetTextureNameStorageEnum spritesheet_texture_name_storage, const bool use_file_extensions) :
	m_directory(""), m_searchSubdirs(search_sub_directories), m_spritesheetTextureNameStorage(spritesheet_texture_name_storage), m_useExtensions(use_file_extensions)
{
}

Spritesheet::Spritesheet(std::string directory, const bool search_sub_directories, const SpritesheetTextureNameStorageEnum spritesheet_texture_name_storage, const bool use_file_extensions) :
	m_directory(std::move(directory)), m_searchSubdirs(search_sub_directories), m_spritesheetTextureNameStorage(spritesheet_texture_name_storage), m_useExtensions(use_file_extensions)
{
}

bool Spritesheet::generate()
{
	// load images
	if (m_searchSubdirs)
	{
		for (const auto& p : boost::filesystem::recursive_directory_iterator(m_directory))
		{
			if (!addTexture(p))
			{
				return false;
			}
		}
	}
	else
	{
		for (const auto& p : boost::filesystem::directory_iterator(m_directory))
		{
			if (!addTexture(p))
			{
				return false;
			}
		}
	}

	if (m_textures.empty())
	{
		m_errors.emplace_back("No textures found");
	}
	
	if (!m_errors.empty())
	{
		return false;
	}

	const auto it = std::find_if(m_textures.begin(), m_textures.end(), [](const TextureElement& t)
	{
		return t.textureName == "default";
	});

	if (it == m_textures.end())
	{
		// Generate missing texture texture
		std::vector<unsigned char> missingTextureData = {
			255, 0, 255, 255, // magenta
			0, 0, 0, 255,     // black
			0, 0, 0, 255,     // black
			255, 0, 255, 255  // magenta
		};
		const unsigned w = 2;
		const auto h = w;

		m_textures.emplace_back("default", glm::vec2(w, h), missingTextureData);
	}

	// pack and create spritesheet
	return packTextures();
}

std::string Spritesheet::getErrors() const
{
	// If there are no errors, return "No errors"
	if (m_errors.empty()) { return "No errors"; }

	// Put all errors nicely into a string
	std::string errorList;
	for (unsigned i = 0; i < m_errors.size(); i++)
	{
		errorList += "[ERROR]: " + m_errors[i];
		if (i + 1 < m_errors.size()) { errorList += '\n'; }
	}
	return errorList;
}

glm::vec4 Spritesheet::getUv(const std::string& filename) const
{
	const auto it = m_elements.find(filename);

	if (it == m_elements.end())
	{
		// Replace this with your own error function, or don't
		std::cout << "[ERROR] Could not find \"" + filename + "\" in spritesheet" << std::endl;
		return getUv("default");
	}

	const auto uv = it->second;

	return glm::vec4(uv.x, 1.0f - uv.y - uv.w, uv.z, uv.w);
}

bool Spritesheet::addTexture(const boost::filesystem::directory_entry& p)
{
	if (!boost::filesystem::is_directory(p.path()) && boost::algorithm::ends_with(p.path().string(), ".png"))
	{
		DEBUG_PRINT("Loading \"" + p.path().string() + "\"");

		// Load and decode png
		std::vector<unsigned char> in;
		std::vector<unsigned char> out;
		unsigned w, h;

		const auto err = lodepng::decode(out, w, h, p.path().string());
		if (err != 0)
		{
			m_errors.push_back("Error occured during LodePNG decode: " + std::to_string(err));
			return false;
		}

		std::string textureName;

		const auto cutoff = m_useExtensions ? 0 : p.path().extension().string().size();

		const auto filename = p.path().filename().string().substr(0, p.path().filename().string().size() - cutoff);
		const auto filepath = p.path().string().substr(0, p.path().string().size() - cutoff);
		const auto directory = m_directory + "\\";

		switch (m_spritesheetTextureNameStorage)
		{
		case SpritesheetTextureNameStorageEnum::FULL_PATH:
			textureName = filepath;
			break;
		case SpritesheetTextureNameStorageEnum::PARTIAL_PATH:
			textureName = filepath.substr(directory.size(), filepath.size() - directory.size());
			break;
		case SpritesheetTextureNameStorageEnum::FILENAME:
			textureName = filename;
			break;
		}

		m_textures.emplace_back(textureName, glm::vec2(w, h), out);
	}

	return true;
}

bool Spritesheet::packTextures()
{
	// Sort from longest sides to shortest sides (image name is secondary)
	std::sort(m_textures.begin(), m_textures.end());

	m_root = std::make_unique<Node>();
	m_spritesheetResolution = m_textures[0].resolution; // set to image with largest side (which is first)
	m_root->rectangle = glm::ivec4(0, 0, m_spritesheetResolution);

	// Pack textures into sheet
	for (auto& t : m_textures)
	{
		DEBUG_PRINT("Packing \"" + t.textureName + "\"" + " <" + std::to_string(t.resolution.x) + ", " + std::to_string(t.resolution.y) + ">");

		const auto texNode = m_root->insert(t);
		if (texNode != nullptr)
		{
			t.position = glm::vec2(texNode->rectangle.x, texNode->rectangle.y);
			texNode->texture = &t;
		}
		else
		{
			/*
			 * Growing code adapted from https://codeincomplete.com/posts/bin-packing/
			 */
			const auto canGrowDown = t.resolution.x <= m_spritesheetResolution.x;
			const auto canGrowRight = t.resolution.y <= m_spritesheetResolution.y;

			const auto shouldGrowRight = canGrowRight && (m_spritesheetResolution.y >= m_spritesheetResolution.x + t.resolution.x);
			const auto shouldGrowDown = canGrowDown && (m_spritesheetResolution.x >= m_spritesheetResolution.y + t.resolution.y);

			auto grewDown = false;

			if (shouldGrowDown) // grow down
			{
				m_spritesheetResolution.y += t.resolution.y;
				grewDown = true;
			}
			else if (shouldGrowRight) // grow right
			{
				m_spritesheetResolution.x += t.resolution.x;
			}
			else if (canGrowDown)
			{
				m_spritesheetResolution.y += t.resolution.y;
				grewDown = true;
			}
			else if (canGrowRight)
			{
				m_spritesheetResolution.x += t.resolution.x;
			}
			else
			{
				m_errors.emplace_back("Couldn't grow spritesheet (this shouldn't happen)");
				return false;
			}

			auto newRoot = std::make_unique<Node>();
			newRoot->rectangle = glm::ivec4(0, 0, m_spritesheetResolution);

			newRoot->children[0] = std::move(m_root); // make m_root one of newRoot's children

			// make other child
			newRoot->children[1] = std::make_unique<Node>();

			// grow down
			if (grewDown)
			{
				newRoot->children[1]->rectangle = glm::ivec4(0, newRoot->children[0]->rectangle.w, newRoot->children[0]->rectangle.z, t.resolution.y);
			}
			else
			{
				newRoot->children[1]->rectangle = glm::ivec4(newRoot->children[0]->rectangle.z, 0, t.resolution.x, newRoot->children[0]->rectangle.w);
			}

			m_root = std::move(newRoot); // make newRoot m_root

			const auto node = m_root->insert(t); // now image can be inserted successfully

			// assign
			if (node != nullptr)
			{
				t.position = glm::vec2(node->rectangle.x, node->rectangle.y);
				node->texture = &t;
			}
			else
			{
				m_errors.emplace_back("Couldn't place image in spritesheet (this shouldn't happen)");
				return false;
			}

		}
	}

	// Turn it into an image
	const auto w = m_spritesheetResolution.x;
	const auto h = m_spritesheetResolution.y;

	std::vector<unsigned char> finalImagePixels;
	finalImagePixels.resize(w * h * 4); // 4 because rgba

	for (auto& t : m_textures)
	{
		DEBUG_PRINT("Adding \"" + t.textureName + "\" to spritesheet");

		// texture position to pixel index
		auto row = 0; // x * 4
		auto col = 0; // y

		for (unsigned i = 0; i < t.pixels.size(); i++)
		{
			const auto pixelIndex = (t.position.x * 4 + row) + (t.position.y + col) * (w * 4);

			finalImagePixels[pixelIndex] = t.pixels[i];

			row++;

			if (row == t.resolution.x * 4)
			{
				row = 0;
				col++;
			}
		}

		m_elements.emplace(t.textureName, glm::vec4(t.position, t.resolution) / glm::vec4(w, h, w, h));
	}

	DEBUG_PRINT("Creating spritesheet");

	// Feel free to change the output or just load it into a texture or something
	const auto err = lodepng::encode("final.png", finalImagePixels, w, h);
	if (err != 0)
	{
		m_errors.push_back("Error occured during LodePNG encode: " + std::to_string(err));
		return false;
	}

	return true;
}

bool operator<(const TextureElement& a, const TextureElement& b)
{
	const auto aSize = std::max(a.resolution.x, a.resolution.y);
	const auto bSize = std::max(b.resolution.x, b.resolution.y);
	if (aSize == bSize)
	{
		return a.textureName < b.textureName;
	}
	return aSize > bSize;
}
