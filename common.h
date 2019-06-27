#pragma once
#include "error.h"
#include <lua/lua.hpp>
#include <lua/lualib.h>
#include <iostream>
using std::boolalpha;
using std::cout;
using std::endl;

#define ctor(args...) void(args)

namespace zlua
{

template <typename T>
const static std::string &type_name()
{
    static std::string name;

    if (name.empty())
    {
        name = __PRETTY_FUNCTION__;
        size_t beg = name.find("[T =") + 5;
        size_t end = name.rfind("]");
        name = name.substr(beg, end - beg);
    }

    return name;
}
} // namespace zlua