#pragma once
#include "common.h"
#include "meta.h"
// #include "engine.h"
#include <vector>

namespace zlua
{

namespace stl
{

template <typename T, typename E>
void register_vector(E &engine)
{
    std::string vec_name = std::string("vector.") + type_info<T>::name();
    cout << __PRETTY_FUNCTION__ << " " << vec_name << endl;
    engine.template reg<std::vector<T>, ctor()>(vec_name.c_str());
}

} // namespace stl
} // namespace zlua
