#pragma once
#include <string>
#include <iostream>
#include <random>
namespace Graphics {

	struct Vector3 {
		union {
			float x;
			float r;
		};
		union {
			float y;
			float g;
		};
		union {
			float z;
			float b;
		};
		Vector3(uint8_t _r, uint8_t _g, uint8_t _b) {
			r = _r / 255.0f;
			g = _g / 255.0f;
			b = _b / 255.0f;
		}
		Vector3(int _r, int _g, int _b) {
			r = _r / 255.0f ;
			g = _g / 255.0f;
			b = _b / 255.0f;
		}
		Vector3(float _r, float _g, float _b) {
			r = _r;
			g = _g;
			b = _b;
		}
		Vector3() {
			r = 0;
			g = 0;
			b = 0;
		}
		static Vector3 RandomColorRGB() {
			return Vector3(rand() % 255, rand() % 255, rand() % 255);
		}

		static Vector3 RandomColorGray() {
			uint8_t col = rand() % 255;
			return Vector3(col, col, col);
		}
	};


	struct Image {
		unsigned int Width;
		unsigned int Height;
		Vector3* data;

		//loads png into float data and sets with and height
		void LoadImageData(std::string path);
		void Create(int width, int height);

		void WritePixel(int x, int y, Vector3 Color);


	};
}
