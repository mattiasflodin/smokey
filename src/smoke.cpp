#include <vector>
#include <type_traits>  // alignment_of
#include <fstream>      // ifstream
#include <algorithm>    // move, swap
#include <stdexcept>

#include <windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>

void gl_check_error()
{
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
        throw std::runtime_error("opengl error");
}

void gl_check_error(GLuint result)
{
    if(0 == result)
        throw std::runtime_error("opengl error");
}

void gl_check_error(void const* result)
{
    if(nullptr == result)
        throw std::runtime_error("opengl error");
}

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

class gl_buffer {
public:
    gl_buffer(GLenum target, GLsizei size, GLenum usage = GL_STATIC_DRAW)
    {
        glGenBuffers(1, &_name);
        gl_check_error();
        glBindBuffer(target, _name);
        gl_check_error();
        glBufferData(target, size, nullptr, usage);
        gl_check_error();
    }

    ~gl_buffer()
    {
        if(_name)
            glDeleteBuffers(1, &_name);
    }

    gl_buffer(gl_buffer const&) = delete;
    gl_buffer& operator=(gl_buffer const&) = delete;

    gl_buffer(gl_buffer&& other) :
        _name(other._name)
    {
        other._name = 0;
    }
    gl_buffer& operator=(gl_buffer&& other)
    {
        std::swap(_name, other._name);
        return *this;
    }

    GLuint get() const
    {
        return _name;
    }

private:
    GLuint _name;
};

template <class Vertex>
class gl_vertex_buffer_map;

template <class Vertex>
class gl_vertex_buffer {
public:
    gl_vertex_buffer(GLsizei size, GLenum usage = GL_STATIC_DRAW) :
        _buffer(GL_ARRAY_BUFFER, size*stride, usage)
    {
    }

    gl_vertex_buffer(gl_vertex_buffer const&) = delete;
    gl_vertex_buffer& operator=(gl_vertex_buffer const&) = delete;

    gl_vertex_buffer(gl_vertex_buffer&& other) :
        _buffer(std::move(other._buffer))
    {
    }
    gl_vertex_buffer& operator=(gl_vertex_buffer&& other)
    {
        _buffer = std::move(other._buffer);
        return *this;
    }

    GLuint get() const
    {
        return _buffer.get();
    }

    gl_vertex_buffer_map<Vertex> map();

private:
    static auto const alignment = std::alignment_of<Vertex>::value;
    static auto const stride = ((sizeof(Vertex) + alignment - 1) / alignment)*alignment;

    gl_buffer _buffer;
};

template <class Vertex>
class gl_vertex_buffer_map {
public:
    gl_vertex_buffer_map(gl_vertex_buffer<Vertex>& buffer, GLenum access = GL_WRITE_ONLY) :
        _p(map_buffer(buffer, access)),
        _buffer(buffer)
    {
    }

    ~gl_vertex_buffer_map()
    {
        if(_p)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _buffer.get());
            gl_check_error();
            glUnmapBuffer(GL_ARRAY_BUFFER);
            gl_check_error();
        }
    }

    gl_vertex_buffer_map(gl_vertex_buffer_map const&) = delete;
    gl_vertex_buffer_map& operator=(gl_vertex_buffer_map const&) = delete;

    gl_vertex_buffer_map(gl_vertex_buffer_map&& other) :
        _p(other._p)
    {
        other._p = nullptr;
    }

    gl_vertex_buffer_map& operator=(gl_vertex_buffer_map&& other)
    {
        std::swap(_p, other._p);
        return *this;
    }

    Vertex* data()
    {
        return _p;
    }

    Vertex const* data() const
    {
        return _p;
    }

    Vertex& operator[](std::ptrdiff_t i)
    {
        return _p[i];
    }

    Vertex const& operator[](std::ptrdiff_t i) const
    {
        return _p[i];
    }

private:
    static Vertex* map_buffer(gl_vertex_buffer<Vertex>& buffer, GLenum access)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer.get());
        gl_check_error();
        auto p = static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, access));
        gl_check_error(p);
        return p;
    }
    Vertex* _p;
    gl_vertex_buffer<Vertex>& _buffer;
};

template <class Vertex>
gl_vertex_buffer_map<Vertex> gl_vertex_buffer<Vertex>::map()
{
    return gl_vertex_buffer_map<Vertex>(*this);
}
namespace detail
{
    class gl_shader_object {
    public:
        gl_shader_object(GLenum type) :
            _shader(glCreateShader(type))
        {
            gl_check_error(_shader);
        }
        ~gl_shader_object()
        {
            if(_shader)
                glDeleteShader(_shader);
        }

        gl_shader_object(gl_shader_object const&) = delete;
        gl_shader_object& operator=(gl_shader_object const&) = delete;

        gl_shader_object(gl_shader_object&& other) :
            _shader(other._shader)
        {
            other._shader = 0;
        }

        gl_shader_object& operator=(gl_shader_object&& other)
        {
            std::swap(_shader, other._shader);
            return *this;
        }

        GLuint get() const
        {
            return _shader;
        }

    private:
        GLuint _shader;
    };
}

class gl_shader {
public:
    gl_shader(GLenum type, GLchar const* source, GLint length) :
        _shader(type)
    {
        glShaderSource(_shader.get(), 1, &source, &length);
        gl_check_error();
        glCompileShader(_shader.get());
        gl_check_error();
        GLint compiled;
        glGetShaderiv(_shader.get(), GL_COMPILE_STATUS, &compiled);
        gl_check_error();
        if(!compiled) {
            GLint length;
            glGetShaderiv(_shader.get(), GL_INFO_LOG_LENGTH, &length);
            gl_check_error();
            std::vector<GLchar> infolog(length);
            glGetShaderInfoLog(_shader.get(), length, nullptr, infolog.data());
            OutputDebugStringA(infolog.data());
            throw std::runtime_error("shader compilation failed");
        }
    }

    gl_shader(gl_shader const&) = delete;
    gl_shader& operator=(gl_shader const&) = delete;

    gl_shader(gl_shader&& other) :
        _shader(std::move(other._shader))
    {
    }

    gl_shader& operator=(gl_shader&& other)
    {
        _shader = std::move(other._shader);
    }

    GLuint get() const
    {
        return _shader.get();
    }

private:
    detail::gl_shader_object _shader;
};

class gl_program {
public:
    gl_program() :
        _program(glCreateProgram())
    {
        gl_check_error(_program);
    }
    ~gl_program()
    {
        glDeleteProgram(_program);
    }

    gl_program& attach(gl_shader const& shader)
    {
        glAttachShader(_program, shader.get());
        gl_check_error();
        return *this;
    }

    void link()
    {
        glLinkProgram(_program);
        gl_check_error();
        GLint linked;
        glGetProgramiv(_program, GL_LINK_STATUS, &linked);
        gl_check_error();
        if(!linked)
            throw std::runtime_error("link failed");
    }

    void use()
    {
        glUseProgram(_program);
    }

private:
    GLuint _program;
};

gl_shader load_shader(GLenum type, char const* path)
{
    std::ifstream ifs(path);
    ifs.seekg(0, std::ios_base::end);
    std::size_t size = static_cast<std::size_t>(ifs.tellg());
    ifs.seekg(0, std::ios_base::beg);
    std::vector<char> v(size);
    ifs.read(v.data(), size);
    gl_shader shader(type, v.data(), size);
    return shader;
}

struct Vertex
{
    float x;
    float y;
};

int main()
{
    glfw_context glfw;
    
    auto window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    if(!window)
        return 1;
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(GLEW_OK != err)
        return 1;

    gl_vertex_buffer<Vertex> vertex_buffer(4);
    {
        auto&& vertices = vertex_buffer.map();
        vertices[0].x = -1;
        vertices[0].y = -1;
        vertices[1].x = 1;
        vertices[1].y = -1;
        vertices[2].x = 1;
        vertices[2].y = 1;
        vertices[3].x = -1;
        vertices[3].y = 1;
    }

    gl_program program;
    program
        .attach(load_shader(GL_VERTEX_SHADER, "src\\vertex.glsl"))
        .attach(load_shader(GL_FRAGMENT_SHADER, "src\\fragment.glsl"))
        .link();

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}