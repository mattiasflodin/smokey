template <typename T>
class valarray {
public:
    valarray(std::size_t size) :
        _p(new T[size])
    {
        std::async()
    }

};