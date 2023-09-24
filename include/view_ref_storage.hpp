#ifndef LRF_VIEW_REF_STORAGE_HPP
#define LRF_VIEW_REF_STORAGE_HPP


namespace lrf
{
    template<bool View, typename T>
    struct ViewRefStorage;

    template<typename T>
    struct ViewRefStorage<false,T>
    {
        const T& value;

        ViewRefStorage(const T& value) : value(value) {};
    };

    template<typename T>
    struct ViewRefStorage<true,T>
    {
        T& value;

        ViewRefStorage(T& value) : value(value) {};
    };
}

#endif