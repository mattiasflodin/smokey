#version 130

in float g_Distance;

void main()
{
    gl_FragColor = vec4(g_Distance, g_Distance, g_Distance, 1.0);
}
