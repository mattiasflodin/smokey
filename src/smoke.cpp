#include "vec2.hpp"
#include "timer.hpp"
#include "gl.hpp"

#include <stdexcept>
#include <random>
#include <iostream>

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

void commit_particles(gl::vertex_buffer<vertex>& vertex_buffer, std::vector<vec2> const& positions)
{
    auto&& vertices = vertex_buffer.map();
    static_assert(sizeof(vertex) == sizeof(positions[0]), "vertex size does not match position size");
    std::memcpy(vertices.data(), positions.data(), sizeof(vertex)*N_PARTICLES);
}



int main()
{
    gl::glfw_context glfw;
    
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

    gl::vertex_buffer<vertex> vertex_buffer(N_PARTICLES);
    gl::program program;
    program
        .attach(gl::load_shader(GL_VERTEX_SHADER, "src\\particle.vert"))
        .attach(gl::load_shader(GL_GEOMETRY_SHADER, "src\\particle.geom"))
        .attach(gl::load_shader(GL_FRAGMENT_SHADER, "src\\particle.frag"))
        .link();

    glEnableVertexAttribArray(0);
    gl::check_error();

    program.use();
    glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    auto aspect_location = program.uniform_location("g_aspect");

    class timer timer;
    unsigned frame_time = 0;
    while(!glfwWindowShouldClose(window)) {
        simulate(positions, velocities, static_cast<float>(1.0)/16);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gl::check_error();
        glUniform1f(aspect_location, g_aspect);
        gl::check_error();

        vertex_buffer.bind();
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertex_buffer.stride, nullptr);
        gl::check_error();
        commit_particles(vertex_buffer, positions);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glDrawArrays(GL_POINTS, 0, N_PARTICLES);
        gl::check_error();
        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_time += 16;
        timer.sleep_until(frame_time);
    }
    return 0;
}
