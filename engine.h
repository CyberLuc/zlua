#pragma once
#include "common.h"
#include "register.h"
#include <string>
// #include <utility>

namespace zlua
{

class Engine
{
public:
    Engine(lua_State *ls = nullptr)
        : ls_(ls)
    {
        if (this->ls_ == nullptr)
        {
            this->ls_ = luaL_newstate();
            luaL_openlibs(this->ls_);
            reg_basic_types();
            dtor_release_ = true;
        }
    }

    ~Engine()
    {
        if (this->ls_ && this->dtor_release_)
        {
            lua_close(this->ls_);
        }
    }

    lua_State *get_lua_state()
    {
        return this->ls_;
    }

    bool load_file(const std::string &file_name, std::string &err)
    {
        if (luaL_dofile(this->ls_, file_name.c_str()) != 0)
        {
            err = luaL_checkstring(this->ls_, -1);
            return false;
        }

        return true;
    }

    bool load_file(const std::string &file_name)
    {
        std::string err;
        if (!this->load_file(file_name, err))
        {
            throw exception(err);
            return false;
        }

        return true;
    }

    template <typename T, typename C, typename... Bases>
    Registrar<T, C, Bases...> reg(const char *name)
    {
        return std::move(Registrar<T, C, Bases...>(this->ls_, name));
    }

    template <typename E>
    EnumRegistrar<E> reg(const char *name)
    {
        return std::move(EnumRegistrar<E>(this->ls_, name));
    }

private:
    void reg_basic_types()
    {
        type_info<int>::set_name("int");
        type_info<double>::set_name("double");
        type_info<float>::set_name("float");
        type_info<uint64_t>::set_name("uint64_t");
        type_info<int64_t>::set_name("int64_t");
        type_info<bool>::set_name("bool");
        type_info<char>::set_name("char");
        type_info<std::string>::set_name("string");

        this->reg<std::vector<int>, ctor()>("int")
            .def("push_back", (void (std::vector<int>::*)(const int &)) & std::vector<int>::push_back)
            .def("pop_back", &std::vector<int>::pop_back)
            .def("at", (const int &(std::vector<int>::*)(size_t) const) & std::vector<int>::at)
            .def("atc", (int &(std::vector<int>::*)(size_t)) & std::vector<int>::at)
            .def("clear", &std::vector<int>::clear)
            .def("size", &std::vector<int>::size)
            //
            ;
    }

    lua_State *ls_;
    bool dtor_release_;
};

} // namespace zlua