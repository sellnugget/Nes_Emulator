#include "Graphics.h"
#include <FreeImage.h>
namespace Graphics {
	void Image::LoadImageData(std::string path)
	{
		FIBITMAP* sourceBitmap1 = FreeImage_Load(FIF_PNG, path.c_str(), PNG_DEFAULT);
		Width = FreeImage_GetWidth(sourceBitmap1);
		Height = FreeImage_GetHeight(sourceBitmap1);

		FreeImage_ConvertToFloat(sourceBitmap1);
		data = new Vector3[Width * Height];

		for (int i = 0; i < Height * Width; i++) {
			int x = i % Width;
			int y = i / Width;
			RGBQUAD pixel;
			FreeImage_GetPixelColor(sourceBitmap1, x, y, &pixel);
			data[i].r = pixel.rgbRed / 255.0f;
			data[i].g = pixel.rgbGreen / 255.0f;
			data[i].b = pixel.rgbBlue / 255.0f;
		}
		

		
	}

	void Image::Create(int width, int height)
	{
		Width = width;
		Height = height;
		data = new Vector3[width * height];
	}

	void Image::WritePixel(int x, int y, Vector3 Color)
	{
		if (y > Height || x > Width) {
			return;
		}
		data[Width * y + x] = Color;
	}

}
