#include <cmath>
#include <limits>
#include <mmintrin.h>

class vec2
{
public:
    vec2() : vec2(0.0f)
    {
    }
    vec2(float v) : vec2(v, v)
    {
    }
    vec2(float x, float y) :
        x(x),
        y(y)
    {
    }

    vec2& operator+=(vec2 const& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    vec2& operator*=(vec2 const& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    vec2& operator*=(float other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    float x;
    float y;
};

inline vec2 operator+(vec2 const& lhs, vec2 const& rhs)
{
    return vec2(lhs) += rhs;
}

inline vec2 operator*(vec2 const& lhs, vec2 const& rhs)
{
    return vec2(lhs) *= rhs;
}

inline vec2 operator*(vec2 const& lhs, float rhs)
{
    return vec2(lhs)*=rhs;
}

inline vec2 operator*(float lhs, vec2 const& rhs)
{
    return vec2(rhs) *= lhs;
}

inline vec2 operator-(vec2 const& v)
{
    return vec2(-v.x, -v.y);
}

inline float dot(vec2 const& lhs, vec2 const& rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y;
}

inline vec2 normalize(vec2 const& v)
{
    float length = std::sqrt(dot(v, v));
    if(length < std::numeric_limits<float>::epsilon())
        return vec2(0, 0);
    return v*(1.0f / length);
}