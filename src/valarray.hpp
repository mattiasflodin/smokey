#include <future>   // async

namespace valarray_impl
{
    struct add
    {

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
    };
}

template <class T>
class valarray {
public:
    valarray(std::size_t size) :
        _p(new T[size])
    {
    }
    ~valarray()
    {
        delete[] _p;
    }

    template <class Op>
    valarray(Op const& op)
    {
        op.size()
    }

private:
    T* _p;
};

template <class T>
valarray_impl::binop<T, valarray_impl::add> operator+(valarray<T> const& lhs, valarray<T> const& rhs)
{
    return valarray_impl::binop<T, valarray_impl::add>(lhs, rhs);
}
