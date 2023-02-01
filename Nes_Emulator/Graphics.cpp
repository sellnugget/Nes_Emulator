#include "Graphics.h"
#include <FreeImage.h>
namespace Graphics {
	void Image::LoadImageData(std::string path)
	{
		FIBITMAP* sourceBitmap1 = FreeImage_Load(FIF_PNG, path.c_str(), PNG_DEFAULT);
		Width = FreeImage_GetWidth(sourceBitmap1);
		Height = FreeImage_GetHeight(sourceBitmap1);

		FreeImage_ConvertToFloat(sourceBitmap1);
		data = new float[Width * Height * 3];

		for (int i = 0; i < Height * Width; i++) {
			int x = i % Width;
			int y = i / Width;
			RGBQUAD pixel;
			FreeImage_GetPixelColor(sourceBitmap1, x, y, &pixel);
			data[i * 3] = pixel.rgbRed / 255.0f;
			data[i * 3 + 1] = pixel.rgbGreen / 255.0f;
			data[i * 3 + 2] = pixel.rgbBlue / 255.0f;
		}
		

		
	}

}
