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
        _x(x),
        _y(y)
    {
    }

    vec2& operator+=(vec2 const& other)
    {
        _x += other._x;
        _y += other._y;
        return *this;
    }

    vec2& operator*=(vec2 const& other)
    {
        _x *= other._x;
        _y *= other._y;
        return *this;
    }

private:
    float _x;
    float _y;
};

vec2 operator+(vec2 const& lhs, vec2 const& rhs)
{
    return vec2(lhs) += rhs;
}

vec2 operator*(vec2 const& lhs, vec2 const& rhs)
{
    return vec2(lhs) *= rhs;
}
