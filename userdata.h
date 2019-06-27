#pragma once
#include "common.h"
#include "traits.h"

namespace zlua
{

namespace userdata
{

template <typename T>
struct object_t
{
    T *ptr;
    size_t offset = 0;
    const bool is_const = std::is_const<T>::value;
    bool need_release = false;
};

template <typename F>
struct method_t
{
    method_t() {}
    method_t(F f_) : ptr(f_) {}

    F ptr;
    const bool is_const = is_const_member_function_pointer<F>::value;
};

template <typename P>
struct property_t
{
    property_t() {}
    property_t(P p) : ptr(p) {}

    P ptr;
};

struct property_base_t
{
    property_base_t() : property(nullptr) {}

    int (*access_handler)(lua_State *, void *, const char *key);
    int (*write_handler)(lua_State *, void *, const char *key);
    void *property;
};

template <typename P>
struct property_wrapper_t : property_base_t
{
    property_wrapper_t() {}

    property_t<P> property_holder;
};

} // namespace userdata

} // namespace zlua