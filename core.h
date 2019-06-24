#pragma once
#include "common.h"
#include "error.h"
#include "util.h"
#include "userdata.h"

namespace zlua
{
// called by lua when access property, call member function
template <typename T>
int metatable_index_function(lua_State *ls)
{
    luaL_checkudata(ls, 1, type_info<T>::metatable_name());
    const char *key = luaL_checkstring(ls, 2);

    auto n = luaL_getmetafield(ls, 1, key);
    ZLUA_ARG_CHECK_THROW(ls, (n != LUA_TNIL), 1, "index nil");

    if (lua_isuserdata(ls, -1))
    {
        auto property_base = static_cast<userdata::PropertyBase *>(lua_touserdata(ls, -1));
        lua_pop(ls, 1);

        return property_base->access_handler(ls, property_base->property, key);
    }

    return 1;
}

// called by lua when write property
template <typename T>
int metatable_newindex_function(lua_State *ls)
{
    luaL_checkudata(ls, 1, type_info<T>::metatable_name());
    const char *key = luaL_checkstring(ls, 2);

    int n = luaL_getmetafield(ls, 1, key);
    ZLUA_ARG_CHECK_THROW(ls, (n != LUA_TNIL), 1, "newindex nil");

    if (lua_isuserdata(ls, -1))
    {
        auto property_base = static_cast<userdata::PropertyBase *>(lua_touserdata(ls, -1));
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
    using property_t = userdata::Property<P T::*>;
    auto *property = static_cast<property_t *>(raw_property);

    auto *obj = static_cast<userdata::Object<T> *>(lua_touserdata(ls, 1));
    stack_op<P>::push(ls, obj->ptr->*(property->ptr));
    return 1;
}

template <typename T, typename P>
int write_property_function(lua_State *ls, void *raw_property, const char *key)
{
    using property_t = userdata::Property<P T::*>;
    auto *property = static_cast<property_t *>(raw_property);

    auto *obj = static_cast<userdata::Object<T> *>(lua_touserdata(ls, 1));
    stack_op<P>::pop(ls, obj->ptr->*(property->ptr));
    return 0;
}

template <typename T, typename R, typename... Args>
int lua_function_forwarder(lua_State *ls)
{
    using method_t = userdata::Method<R (T::*)(Args...)>;

    userdata::Object<T> *obj = static_cast<userdata::Object<T> *>(lua_touserdata(ls, 1));
    method_t *func_wrapper = static_cast<method_t *>(lua_touserdata(ls, lua_upvalueindex(1)));

    using wrapped_tuple_t = typename wrap_tuple_reference<std::tuple<Args...>>::type;
    wrapped_tuple_t params;
    tuple_filler<sizeof...(Args)>::fill(ls, params);

    WrapperCall<T, R, decltype(params), Args...>::call(ls, func_wrapper->ptr, obj->ptr, params);
    return std::is_same<R, void>::value ? 0 : 1;
}

template <typename T, typename C>
int lua_object_creator(lua_State *ls)
{
    T *t = object_creator<T>::create(ls, (C *)nullptr);
    stack_op<T>::push(ls, t);
    return 1;
}

} // namespace zlua
