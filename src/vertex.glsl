#version 100

attribute vec2 g_position;

void main(void)
{
    gl_Position = vec4(g_position.x, g_position.y, 0.0, 0.0);
}