#include <future>   // async

namespace valarray_impl
{
    template <class T>
    struct plus
    {
        T apply(T const& lhs, T const& rhs)
        {
            return lhs + rhs;
        }
    };
    template <class T, class Op>
    struct binop
    {
        T lhs;
        T rhs;

        std::size_t size() const
        {
            return lhs.size();
        }
        T operator[](std::size_t i) const
        {
            return Op::apply(lhs[i], rhs[i]);
        }
    };
}

template <class T>
class valarray {
public:
    valarray(std::size_t size) :
        _p(new T[size]),
        _size(size)
    {
    }
    ~valarray()
    {
        delete[] _p;
    }

    template <class Op>
    valarray(Op const& op)
    {
        auto size = op.size();
        _allocate(size);
        _p = new T[size];
        _size = size;
        _assign(op);
    }

private:
    template <class Op>
    void _assign(Op const& op)
    {
        auto new_size = op.size();
        if(_size != new_size)
        {
            _allocate(new_size);
        }
    }

    T* _p;
    std::size_t _size;
};

template <class T>
valarray_impl::binop<T, valarray_impl::add> operator+(valarray<T> const& lhs, valarray<T> const& rhs)
{
    return valarray_impl::binop<T, valarray_impl::plus>(lhs, rhs);
}
