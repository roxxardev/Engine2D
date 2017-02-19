#pragma once
#include "GLTexture.h"

#include <string>

namespace Engine {

	class ImageLoader
	{
	public:
		static GLTexture loadPNG(std::string filePath);
	};
}