#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>      // ifstream
#include <type_traits>  // alignment_of
#include <algorithm>    // move, swap

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>

namespace gl
{

class error : public std::runtime_error
{
public:
    error(GLenum code) :
        runtime_error("OpenGL call failed"),
        _code(code)
    {
    }

    GLenum code() const
    {
        return _code;
    }

private:
    GLenum _code;
};

class compilation_error : public std::runtime_error
{
public:
    compilation_error(std::string const& log) :
        runtime_error("shader compilation error"),
        _log(log)
    {
    }

    std::string const& log() const
    {
        return _log;
    }

private:
    std::string _log;
};

void check_error()
{
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
        throw error(err);
}

void check_error(GLuint result)
{
    if(0 == result)
        throw error(glGetError());
}

void check_error(void const* result)
{
    if(nullptr == result)
        throw error(glGetError());
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

class buffer {
public:
    buffer(GLenum target, GLsizei size, GLenum usage = GL_STATIC_DRAW)
    {
        glGenBuffers(1, &_name);
        check_error();
        glBindBuffer(target, _name);
        check_error();
        glBufferData(target, size, nullptr, usage);
        check_error();
    }

    ~buffer()
    {
        if(_name)
            glDeleteBuffers(1, &_name);
    }

    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;

    buffer(buffer&& other) :
        _name(other._name)
    {
        other._name = 0;
    }
    buffer& operator=(buffer&& other)
    {
        std::swap(_name, other._name);
        return *this;
    }

    GLuint get() const
    {
        return _name;
    }

    void bind(GLenum target)
    {
        glBindBuffer(target, _name);
        check_error();
    }

private:
    GLuint _name;
};

template <class Vertex>
class vertex_buffer_map;

template <class Vertex>
class vertex_buffer {
public:
    static auto const alignment = std::alignment_of<Vertex>::value;
    static auto const stride = ((sizeof(Vertex) + alignment - 1) / alignment)*alignment;

    vertex_buffer(GLsizei size, GLenum usage = GL_STATIC_DRAW) :
        _buffer(GL_ARRAY_BUFFER, size*stride, usage)
    {
    }

    vertex_buffer(vertex_buffer const&) = delete;
    vertex_buffer& operator=(vertex_buffer const&) = delete;

    vertex_buffer(vertex_buffer&& other) :
        _buffer(std::move(other._buffer))
    {
    }
    vertex_buffer& operator=(vertex_buffer&& other)
    {
        _buffer = std::move(other._buffer);
        return *this;
    }

    GLuint get() const
    {
        return _buffer.get();
    }

    void bind()
    {
        _buffer.bind(GL_ARRAY_BUFFER);
    }

    vertex_buffer_map<Vertex> map();

private:
    buffer _buffer;
};

template <class Vertex>
class vertex_buffer_map {
public:
    vertex_buffer_map(vertex_buffer<Vertex>& buffer, GLenum access = GL_WRITE_ONLY) :
        _p(map_buffer(buffer, access)),
        _buffer(buffer)
    {
    }

    ~vertex_buffer_map()
    {
        if(_p)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _buffer.get());
            check_error();
            glUnmapBuffer(GL_ARRAY_BUFFER);
            check_error();
        }
    }

    vertex_buffer_map(vertex_buffer_map const&) = delete;
    vertex_buffer_map& operator=(vertex_buffer_map const&) = delete;

    vertex_buffer_map(vertex_buffer_map&& other) :
        _p(other._p)
    {
        other._p = nullptr;
    }

    vertex_buffer_map& operator=(vertex_buffer_map&& other)
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
    static Vertex* map_buffer(vertex_buffer<Vertex>& buffer, GLenum access)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer.get());
        check_error();
        auto p = static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, access));
        check_error(p);
        return p;
    }
    Vertex* _p;
    vertex_buffer<Vertex>& _buffer;
};

template <class Vertex>
vertex_buffer_map<Vertex> vertex_buffer<Vertex>::map()
{
    return vertex_buffer_map<Vertex>(*this);
}

class shader_object {
public:
    shader_object(GLenum type) :
        _shader(glCreateShader(type))
    {
        check_error(_shader);
    }
    ~shader_object()
    {
        if(_shader)
            glDeleteShader(_shader);
    }

    shader_object(shader_object const&) = delete;
    shader_object& operator=(shader_object const&) = delete;

    shader_object(shader_object&& other) :
        _shader(other._shader)
    {
        other._shader = 0;
    }

    shader_object& operator=(shader_object&& other)
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

class shader {
public:
    shader(GLenum type, GLchar const* source, GLint length) :
        _object(type)
    {
        glShaderSource(_object.get(), 1, &source, &length);
        check_error();
        glCompileShader(_object.get());
        check_error();
        GLint compiled;
        glGetShaderiv(_object.get(), GL_COMPILE_STATUS, &compiled);
        check_error();
        if(!compiled) {
            GLint length;
            glGetShaderiv(_object.get(), GL_INFO_LOG_LENGTH, &length);
            check_error();
            std::vector<GLchar> infolog(length);
            glGetShaderInfoLog(_object.get(), length, nullptr, infolog.data());
            throw compilation_error(std::string(begin(infolog), end(infolog)));
        }
    }

    shader(shader const&) = delete;
    shader& operator=(shader const&) = delete;

    shader(shader&& other) :
        _object(std::move(other._object))
    {
    }

    shader& operator=(shader&& other)
    {
        _object = std::move(other._object);
    }

    GLuint get() const
    {
        return _object.get();
    }

private:
    shader_object _object;
};

class program {
public:
    program() :
        _program(glCreateProgram())
    {
        check_error(_program);
    }
    ~program()
    {
        glDeleteProgram(_program);
    }

    program& attach(shader const& shader)
    {
        glAttachShader(_program, shader.get());
        check_error();
        return *this;
    }

    void link()
    {
        glLinkProgram(_program);
        check_error();
        GLint linked;
        glGetProgramiv(_program, GL_LINK_STATUS, &linked);
        check_error();
        if(!linked) {
            GLint length;
            glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &length);
            check_error();
            std::vector<GLchar> log(length);
            glGetProgramInfoLog(_program, length, nullptr, log.data());
            check_error();
            throw compilation_error(std::string(begin(log), end(log)));
        }
    }

    GLint uniform_location(GLchar const* name)
    {
        auto location = glGetUniformLocation(_program, name);
        check_error();
        return location;
    }

    void use()
    {
        glUseProgram(_program);
    }

private:
    GLuint _program;
};

shader load_shader(GLenum type, char const* path)
{
    std::ifstream ifs(path);
    ifs.seekg(0, std::ios_base::end);
    std::size_t size = static_cast<std::size_t>(ifs.tellg());
    ifs.seekg(0, std::ios_base::beg);
    std::vector<char> v(size);
    ifs.read(v.data(), size);
    shader shader(type, v.data(), size);
    return shader;
}

}   // namespace gl
