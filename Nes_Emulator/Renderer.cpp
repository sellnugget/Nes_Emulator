#include "Renderer.h"


namespace Graphics {
    void Renderer::Resize(int x, int y)
    {
        WindowX = x;
        WindowY = y;
        glfwSetWindowSize(_window, WindowX, WindowY);
    }
    Renderer::Renderer()
    {
        /* Initialize the library */
        if (!glfwInit())
            Error::Error(Error::Graphics_Init_Fail);


        /* Create a windowed mode window and its OpenGL context */
        _window = glfwCreateWindow(960, 720, "Nes Emulator", NULL, NULL);
        if (!_window)
        {
            glfwTerminate();
            Error::Error(Error::Graphics_Init_Fail);
        }
        /* Make the window's context current */
        glfwMakeContextCurrent(_window);

        if (glewInit() != GLEW_OK) {
            Error::Error(Error::Graphics_Init_Fail);
        }
    }

    void Renderer::UpdateGL(int update)
    {   
        /* Render here */
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WindowX, 0, WindowY, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor3f(1, 1, 1);

        for (int i = 0; i < _renderobjects.size(); i++) {
            GLuint tex = _images[_renderobjects[i].texture];
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _imagedat[tex]->Width, _imagedat[tex]->Height, 0, GL_RGB, GL_FLOAT, _imagedat[tex]->data);
            glBegin(GL_QUADS);

            glTexCoord2f(0, 0);
            glVertex2f(_renderobjects[i].x, _renderobjects[i].y);
            glTexCoord2f(0, 1);
            glVertex2f(_renderobjects[i].x, _renderobjects[i].y + (_imagedat[tex]->Height * _renderobjects[i].height));
            glTexCoord2f(1, 1);
            glVertex2f(_renderobjects[i].x + (_imagedat[tex]->Width * _renderobjects[i].width), _renderobjects[i].y + (_imagedat[tex]->Height * _renderobjects[i].height));
            glTexCoord2f(1, 0);
            glVertex2f(_renderobjects[i].x + (_imagedat[tex]->Width * _renderobjects[i].width), _renderobjects[i].y);
            glEnd();
        }


        /* Swap front and back buffers */
        glfwSwapBuffers(_window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    void Renderer::DrawObject(std::string texture, float x, float y, float width, float height)
    {
        _renderobjects.push_back({ texture, height, width, x, y});
    }

    void Renderer::AddImage(std::string name, Image* image)
    {
        _images[name] = _current;
        _current++;
        _imagedat.push_back(image);
        delete[] _texid;
        _texid = new GLuint[_current];
        for (int i = 0; i < _current; i++) {
            _texid[i] = i;
        }
        GenBuffers(image->Width, image->Height);
    }

    void Renderer::GenBuffers(int width, int height)
    {
        glEnable(GL_TEXTURE_2D);
        glGenTextures(_current, _texid);
        for (int i = 0; i < _current; i++) {
            glBindTexture(GL_TEXTURE_2D, i);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _imagedat[i]->Width, _imagedat[i]->Height, 0, GL_RGB, GL_FLOAT, _imagedat[i]->data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }

}
