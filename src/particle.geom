#version 150
#extension GL_ARB_explicit_uniform_location: enable

layout(points) in;
layout(triangle_strip, max_vertices=34) out;
layout(location=0) uniform float g_aspect;
out float g_Distance;

const float PARTICLE_RADIUS = 0.05;

void emit(vec2 point_on_circle)
{
    gl_Position = vec4(gl_in[0].gl_Position.xy +
        vec2(PARTICLE_RADIUS, PARTICLE_RADIUS*g_aspect)*point_on_circle,
        gl_in[0].gl_Position.zw);
    g_Distance = 0.0;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position;
    g_Distance = 1.0;
    EmitVertex();
}

void main()
{
    const vec2 p0 = vec2(1, 0);
    const vec2 p1 = vec2(0.923879532511286756, -0.382683432365089772);
    const vec2 p2 = vec2(0.707106781186547524, -0.707106781186547524);
    const vec2 p3 = vec2(0.382683432365089772, -0.923879532511286756);
    const vec2 p4 = vec2(0, -1);
    const vec2 p5 = vec2(-0.382683432365089772, -0.923879532511286756);
    const vec2 p6 = vec2(-0.707106781186547524, -0.707106781186547524);
    const vec2 p7 = vec2(-0.923879532511286756, -0.382683432365089772);
    const vec2 p8 = vec2(-1, 0);
    const vec2 p9 = vec2(-0.923879532511286756, 0.382683432365089772);
    const vec2 p10 = vec2(-0.707106781186547524, 0.707106781186547524);
    const vec2 p11 = vec2(-0.382683432365089772, 0.923879532511286756);
    const vec2 p12 = vec2(0, 1);
    const vec2 p13 = vec2(0.382683432365089772, 0.923879532511286756);
    const vec2 p14 = vec2(0.707106781186547524, 0.707106781186547524);
    const vec2 p15 = vec2(0.923879532511286756, 0.382683432365089772);

    emit(p0);
    emit(p1);
    emit(p2);
    emit(p3);
    emit(p4);
    emit(p5);
    emit(p6);
    emit(p7);
    emit(p8);
    emit(p9);
    emit(p10);
    emit(p11);
    emit(p12);
    emit(p13);
    emit(p14);
    emit(p15);
    emit(p0);
}
