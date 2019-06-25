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

    template <typename T, typename C, typename... Args>
    Registrar<T, C> reg(const char *name, Args... args)
    {
        return std::move(Registrar<T, C>(this->ls_, name, args...));
    }

    template <typename E>
    EnumRegistrar<E> reg(const char *name)
    {
        return std::move(EnumRegistrar<E>(this->ls_, name));
    }

private:
    lua_State *ls_;
    bool dtor_release_;
};

} // namespace zlua