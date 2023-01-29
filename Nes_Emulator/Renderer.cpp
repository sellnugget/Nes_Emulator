#include "Renderer.h"

Renderer::Renderer()
{
    /* Initialize the library */
    if (!glfwInit())
        Error::Error(Error::Graphics_Init_Fail);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        Error::Error(Error::Graphics_Init_Fail);
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        Error::Error(Error::Graphics_Init_Fail);
    }
}

void Renderer::StartPPU()
{
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
}
