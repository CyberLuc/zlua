#pragma once
#include "common.h"

namespace zlua
{

namespace userdata
{

template <typename T>
struct Object
{
    T *ptr;
    size_t offset = 0;
    const bool is_const = std::is_const<T>::value;
    bool need_release = false;
};

template <typename F>
struct Method
{
    Method() {}
    Method(F f_) : ptr(f_) {}

    F ptr;
};

template <typename P>
struct Property
{
    Property() {}
    Property(P p) : ptr(p) {}

    P ptr;
};

struct PropertyBase
{
    PropertyBase() : property(nullptr) {}

    int (*access_handler)(lua_State *, void *, const char *key);
    int (*write_handler)(lua_State *, void *, const char *key);
    void *property;
};

template <typename P>
struct PropertyWrapper : PropertyBase
{
    PropertyWrapper() {}

    Property<P> property_holder;
};

} // namespace userdata

} // namespace zlua