#pragma once
#include <string>
#include <iostream>

namespace Graphics {
	struct Image {
		unsigned int Width;
		unsigned int Height;
		float* data;
		//loads png into float data and sets with and height
		void LoadImageData(std::string path);
	};
}
