#include "vec2.hpp"
#include "timer.hpp"

#include <vector>
#include <type_traits>  // alignment_of
#include <fstream>      // ifstream
#include <algorithm>    // move, swap
#include <stdexcept>
#include <random>
#include <iostream>

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

    void bind(GLenum target)
    {
        glBindBuffer(target, _name);
        gl_check_error();
    }

private:
    GLuint _name;
};

template <class vertex>
class gl_vertex_buffer_map;

template <class vertex>
class gl_vertex_buffer {
public:
    static auto const alignment = std::alignment_of<vertex>::value;
    static auto const stride = ((sizeof(vertex) + alignment - 1) / alignment)*alignment;

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

    void bind()
    {
        _buffer.bind(GL_ARRAY_BUFFER);
    }

    gl_vertex_buffer_map<vertex> map();

private:
    gl_buffer _buffer;
};

template <class vertex>
class gl_vertex_buffer_map {
public:
    gl_vertex_buffer_map(gl_vertex_buffer<vertex>& buffer, GLenum access = GL_WRITE_ONLY) :
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

    vertex* data()
    {
        return _p;
    }

    vertex const* data() const
    {
        return _p;
    }

    vertex& operator[](std::ptrdiff_t i)
    {
        return _p[i];
    }

    vertex const& operator[](std::ptrdiff_t i) const
    {
        return _p[i];
    }

private:
    static vertex* map_buffer(gl_vertex_buffer<vertex>& buffer, GLenum access)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer.get());
        gl_check_error();
        auto p = static_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, access));
        gl_check_error(p);
        return p;
    }
    vertex* _p;
    gl_vertex_buffer<vertex>& _buffer;
};

template <class vertex>
gl_vertex_buffer_map<vertex> gl_vertex_buffer<vertex>::map()
{
    return gl_vertex_buffer_map<vertex>(*this);
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
            std::cerr << infolog.data();
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

    GLint uniform_location(GLchar const* name)
    {
        auto location = glGetUniformLocation(_program, name);
        gl_check_error();
        return location;
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

struct vertex
{
    vec2 position;
};

float g_aspect = 1.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_aspect = static_cast<float>(width) / static_cast<float>(height);
    glViewport(0, 0, width, height);
}

std::size_t const N_PARTICLES = 10000;

void simulate(std::vector<vec2>& positions, std::vector<vec2>& velocities, float dt)
{
    float const GRAVITY = 0.05f;
    for(std::size_t i = 0; i != N_PARTICLES; ++i)
    {
        vec2 pos = positions[i];
        vec2 acceleration = -pos*GRAVITY;
        vec2 velocity = velocities[i];
        velocity += dt*acceleration;
        pos += dt*velocity;
        positions[i] = pos;
        velocities[i] = velocity;
    }
}

void commit_particles(gl_vertex_buffer<vertex>& vertex_buffer, std::vector<vec2> const& positions)
{
    auto&& vertices = vertex_buffer.map();
    static_assert(sizeof(vertex) == sizeof(positions[0]), "vertex size does not match position size");
    std::memcpy(vertices.data(), positions.data(), sizeof(vertex)*N_PARTICLES);
}



int main()
{
    glfw_context glfw;
    
    auto window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    if(!window)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    framebuffer_size_callback(window, 640, 480);

    GLenum err = glewInit();
    if(GLEW_OK != err)
        return 1;

    std::mt19937 rng_engine;
    std::uniform_real_distribution<float> rng(-1.0f, 1.0f);
    std::vector<vec2> positions(N_PARTICLES);
    std::vector<vec2> velocities(N_PARTICLES);
    for(std::size_t i = 0; i != N_PARTICLES; ++i)
    {
        positions[i] = 0.75f*vec2(rng(rng_engine), rng(rng_engine));
        velocities[i] = 0.1f*rng(rng_engine)*normalize(vec2(rng(rng_engine), rng(rng_engine)));
    }

    gl_vertex_buffer<vertex> vertex_buffer(N_PARTICLES);
    gl_program program;
    program
        .attach(load_shader(GL_VERTEX_SHADER, "src\\particle.vert"))
        .attach(load_shader(GL_GEOMETRY_SHADER, "src\\particle.geom"))
        .attach(load_shader(GL_FRAGMENT_SHADER, "src\\particle.frag"))
        .link();

    glEnableVertexAttribArray(0);
    gl_check_error();

    program.use();
    glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    auto aspect_location = program.uniform_location("g_aspect");

    class timer timer;
    unsigned frame_time = 0;
    while(!glfwWindowShouldClose(window)) {
        simulate(positions, velocities, static_cast<float>(1.0)/16);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gl_check_error();
        glUniform1f(aspect_location, g_aspect);
        gl_check_error();

        vertex_buffer.bind();
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertex_buffer.stride, nullptr);
        gl_check_error();
        commit_particles(vertex_buffer, positions);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glDrawArrays(GL_POINTS, 0, N_PARTICLES);
        gl_check_error();
        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_time += 16;
        timer.sleep_until(frame_time);
    }
    return 0;
}
