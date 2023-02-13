#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ErrorHandle.h"
#include "Graphics.h"
#include <vector>
#include <map>
namespace Graphics {


	
	class Renderer
	{

		struct RenderObject {
			std::string texture;
			//scaler
			float width;
			float height;
			float x;
			float y;
		};

	public:
		void Resize(int x, int y);
		Renderer();
		void UpdateGL(int update);
		void DrawObject(std::string texture, float x, float y, float width = 1, float height = 1);
		void AddImage(std::string name,Image* image);
		GLFWwindow* _window;
	private:
		int WindowX = 256;
		int WindowY = 240;
		void GenBuffers(int width, int height);
		std::map<std::string, GLuint> _images;
		std::vector<Image*> _imagedat;
		int _current;
		std::vector<RenderObject> _renderobjects;
		GLuint* _texid;
	};


}

