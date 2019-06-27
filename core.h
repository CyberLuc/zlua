#pragma once
#include "common.h"
#include "error.h"
#include "util.h"
#include "userdata.h"

namespace zlua
{
template <typename T>
int metatable_index_function(lua_State *ls)
{
    auto *ud = static_cast<userdata::object_t<T> *>(lua_touserdata(ls, 1));
    const char *key = luaL_checkstring(ls, 2);

    auto n = luaL_getmetafield(ls, 1, key);

    if (n == LUA_TNIL && type_info<T>::is_inherited())
    {
        auto &inheritance_info_vec = type_info<T>::get_inheritance_info();
        for (auto &info : inheritance_info_vec)
        {
            luaL_getmetatable(ls, ("zlua." + info.name).c_str());
            lua_pushstring(ls, key);
            n = lua_rawget(ls, -2);
            lua_remove(ls, -2);

            if (n != LUA_TNIL)
            {
                ud->offset = info.offset;
                break;
            }

            lua_pop(ls, 1);
        }
    }

    ZLUA_ARG_CHECK_THROW(ls, (n != LUA_TNIL), 1, (__PRETTY_FUNCTION__ + std::string("index nil ") + key).c_str());

    if (lua_isuserdata(ls, -1))
    {
        auto property_base = static_cast<userdata::property_base_t *>(lua_touserdata(ls, -1));
        lua_pop(ls, 1);

        return property_base->access_handler(ls, property_base->property, key);
    }

    return 1;
}

template <typename T>
int metatable_newindex_function(lua_State *ls)
{
    luaL_checkudata(ls, 1, type_info<T>::metatable_name());
    const char *key = luaL_checkstring(ls, 2);

    int n = luaL_getmetafield(ls, 1, key);
    ZLUA_ARG_CHECK_THROW(ls, (n != LUA_TNIL), 1, "newindex nil");

    if (lua_isuserdata(ls, -1))
    {
        auto property_base = static_cast<userdata::property_base_t *>(lua_touserdata(ls, -1));
        lua_pop(ls, 1);
        return property_base->write_handler(ls, property_base->property, key);
    }
    else
    {
        // pop
    }

    return 0;
}

template <typename T, typename P>
int access_property_function(lua_State *ls, void *raw_property, const char *key)
{
    using property_t = userdata::property_t<P T::*>;
    auto *property = static_cast<property_t *>(raw_property);

    auto *obj = static_cast<userdata::object_t<T> *>(lua_touserdata(ls, 1));
    stack_op<P>::push(ls, obj->ptr->*(property->ptr));
    return 1;
}

template <typename T, typename P>
int write_property_function(lua_State *ls, void *raw_property, const char *key)
{
    using property_t = userdata::property_t<P T::*>;
    auto *property = static_cast<property_t *>(raw_property);

    auto *obj = static_cast<userdata::object_t<T> *>(lua_touserdata(ls, 1));
    stack_op<P>::pop(ls, obj->ptr->*(property->ptr));
    return 0;
}

template <typename T, typename R, typename... Args>
int lua_function_forwarder(lua_State *ls)
{
    using method_t = userdata::method_t<R (T::*)(Args...)>;

    userdata::object_t<T> *obj_wrapper = static_cast<userdata::object_t<T> *>(lua_touserdata(ls, 1));
    T *t = reinterpret_cast<T *>(((char *)obj_wrapper->ptr + obj_wrapper->offset));
    obj_wrapper->offset = 0;

    method_t *func_wrapper = static_cast<method_t *>(lua_touserdata(ls, lua_upvalueindex(1)));
    assert(!obj_wrapper->is_const || func_wrapper->is_const && "const object can't call non-const member function");

    using wrapped_tuple_t = pack_tuple_t<Args...>;
    wrapped_tuple_t params;
    stack_op<wrapped_tuple_t>::pop(ls, params);

    wrapped_tuple_invoke<T, R, decltype(params), Args...>::call(ls, func_wrapper->ptr, t, params);
    return element_size<R>::value;
}

template <typename T, typename... Args>
int lua_object_creator(lua_State *ls)
{
    using wrapped_tuple_t = pack_tuple_t<Args...>;
    wrapped_tuple_t params;
    stack_op<wrapped_tuple_t>::pop(ls, params);

    T *t = tuple_construct<T>(params);
    stack_op<T>::push_new(ls, t);

    return 1;
}

template <typename T, typename... Args>
int (*fetch_creator(void (*)(Args...)))(lua_State *)
{
    return &lua_object_creator<T, Args...>;
}

template <typename T>
int lua_object_deleter(lua_State *ls)
{
    userdata::object_t<T> *obj = static_cast<userdata::object_t<T> *>(lua_touserdata(ls, 1));
    if (obj->need_release)
    {
        obj->need_release = false;
        delete obj->ptr;
        obj->ptr = nullptr;
    }

    return 0;
}

template <typename T, typename Enabled = void>
struct lua_object_cloner_wrapper
{
    static int clone(lua_State *ls)
    {
        // TODO check udata
        userdata::object_t<T> *obj = static_cast<userdata::object_t<T> *>(lua_touserdata(ls, 1));
        lua_pop(ls, 1);

        T *t = new T(*obj->ptr);
        stack_op<T>::push_new(ls, t);

        return 1;
    }
};

template <typename T>
struct lua_object_cloner_wrapper<T, typename std::enable_if<!std::is_copy_constructible<T>::value>::type>
{
    static int clone(lua_State *ls)
    {
        lua_pushnil(ls);
        return 1;
    }
};

} // namespace zlua
