#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>

#include <stdexcept>

class glfw_context {
public:
    glfw_context()
    {
        if(!glfwInit())
            throw std::runtime_error("cannot initialize GLFW");
    }
    ~glfw_context()
    {
        glfwTerminate();
    }
};

int main()
{
    class glfw_context glfw;
    
    auto window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if(!window)
        return 1;
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(GLEW_OK != err)
        return 1;

    GLuint bufferName = -1;
    glGenBuffers(1, &bufferName);
    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}