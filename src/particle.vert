#version 330

layout(location=0) in vec2 g_position;

void main()
{
    gl_Position = vec4(g_position, 0.0, 1.0);
}
