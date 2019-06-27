#pragma once
#include "common.h"
#include "traits.h"
#include "meta.h"
#include "userdata.h"
#include <utility>

namespace zlua
{

template <typename T, typename Enabled = void>
struct stack_op;

////////////////////////////////////////////////////////////////////////////////
// integral, rejects char, bool
template <typename T>
struct stack_op<T, typename std::enable_if<
                       !is_reference_wrapper<T>::value &&
                       is_integral_type<base_type_t<T>>::value>::type>
{
    using Base = base_type_t<T>;

    template <typename U>
    static typename std::enable_if<!std::is_pointer<U>::value>::type
    push(lua_State *ls, const U &u)
    {
        lua_pushinteger(ls, static_cast<Base>(u));
    }

    template <typename U>
    static typename std::enable_if<!std::is_pointer<U>::value>::type
    peek(lua_State *ls, U &u, int pos = -1)
    {
        ZLUA_ARG_CHECK_THROW(ls, lua_isinteger(ls, pos), pos, "not an integer value");
        u = static_cast<Base>(luaL_checkinteger(ls, pos));
    }

    template <typename U>
    static typename std::enable_if<!std::is_pointer<U>::value>::type
    pop(lua_State *ls, U &u, int pos = -1)
    {
        peek(ls, u, pos);
        lua_remove(ls, pos);
    }
};

////////////////////////////////////////////////////////////////////////////////
// floating point
template <typename T>
struct stack_op<T, typename std::enable_if<
                       !is_reference_wrapper<T>::value &&
                       std::is_floating_point<base_type_t<T>>::value>::type>
{
    using Base = base_type_t<T>;

    template <typename U>
    static typename std::enable_if<!std::is_pointer<U>::value>::type
    push(lua_State *ls, const U &u)
    {
        lua_pushnumber(ls, static_cast<Base>(u));
    }

    template <typename U>
    static typename std::enable_if<!std::is_pointer<U>::value>::type
    peek(lua_State *ls, U &u, int pos = -1)
    {
        ZLUA_ARG_CHECK_THROW(ls, lua_isinteger(ls, pos), pos, "not a floating point value");
        u = static_cast<Base>(luaL_checknumber(ls, pos));
    }

    template <typename U>
    static typename std::enable_if<!std::is_pointer<U>::value>::type
    pop(lua_State *ls, U &u, int pos = -1)
    {
        peek(ls, u, pos);
        lua_remove(ls, pos);
    }
};

////////////////////////////////////////////////////////////////////////////////
// bool
template <typename T>
struct stack_op<T, typename std::enable_if<
                       !is_reference_wrapper<T>::value &&
                       std::is_same<bool, base_type_t<T>>::value>::type>
{
    static void push(lua_State *ls, bool b)
    {
        lua_pushboolean(ls, b ? 1 : 0);
    }

    static void peek(lua_State *ls, bool &b, int pos = -1)
    {
        ZLUA_ARG_CHECK_THROW(ls, lua_isboolean(ls, pos), pos, "not a boolean value");
        b = lua_toboolean(ls, pos) != 0;
    }

    static void pop(lua_State *ls, bool &b, int pos = -1)
    {
        peek(ls, b, pos);
        lua_remove(ls, pos);
    }
};

////////////////////////////////////////////////////////////////////////////////
// string, char
template <typename T>
struct stack_op<T, typename std::enable_if<
                       !is_reference_wrapper<T>::value &&
                       is_string_type<base_type_t<T>>::value>::type>
{
    using Base = base_type_t<T>;

    static void push(lua_State *ls, const std::string &s)
    {
        _push(ls, s.c_str(), s.length());
    }

    static void push(lua_State *ls, const char *s)
    {
        _push(ls, s);
    }

    static void push(lua_State *ls, const char c)
    {
        _push(ls, &c, 1);
    }

    static void peek(lua_State *ls, std::string &s, int pos = -1)
    {
        ZLUA_ARG_CHECK_THROW(ls, lua_isstring(ls, pos), pos, "not a string value");
        s = luaL_checkstring(ls, pos);
    }

    static void peek(lua_State *ls, const char *&s, int pos = -1)
    {
        if (lua_isnil(ls, pos) != 0)
        {
            s = nullptr;
            return;
        }

        ZLUA_ARG_CHECK_THROW(ls, lua_isstring(ls, pos), pos, "not a string value");
        s = luaL_checkstring(ls, pos);
    }

    static void peek(lua_State *ls, char &c, int pos = -1)
    {
        ZLUA_ARG_CHECK_THROW(ls, lua_isstring(ls, pos), pos, "not a string value");
        size_t len = 0;
        const char *s = luaL_checklstring(ls, pos, &len);
        c = len > 0 ? s[0] : '\0';
    }

    static void pop(lua_State *ls, const char *&s, int pos = -1)
    {
        peek(ls, s, pos);
        lua_remove(ls, pos);
    }

    static void pop(lua_State *ls, std::string &s, int pos = -1)
    {
        peek(ls, s, pos);
        lua_remove(ls, pos);
    }

private:
    static void _push(lua_State *ls, const char *s, size_t l = 0)
    {
        if (s == nullptr)
        {
            lua_pushnil(ls);
        }
        else
        {
            l > 0 ? lua_pushlstring(ls, s, l) : lua_pushstring(ls, s);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// struct/class
// rejects std::string and stl containers(vector, map, set, etc...)
template <typename T>
struct stack_op<T, typename std::enable_if<
                       std::is_class<base_type_t<T>>::value &&
                       !std::is_same<base_type_t<T>, std::string>::value &&
                       //!is_stl_container<base_type_t<T>>::value &&
                       !is_tuple_type<base_type_t<T>>::value &&
                       !is_reference_wrapper<T>::value>::type>
{
    using Base = base_type_t<T>;
    using userdata_object_t = userdata::object_t<Base>;
    using const_userdata_object_t = userdata::object_t<const Base>;

    // rvalue
    static void push(lua_State *ls, Base &&b, int pos = -1)
    {
        auto *object_wrapper = static_cast<userdata_object_t *>(lua_newuserdata(ls, sizeof(userdata_object_t)));
        new (object_wrapper) userdata_object_t;
        object_wrapper->ptr = new Base(std::move(b));
        object_wrapper->need_release = true;

        prepare_metatable(ls);
    }

    // lvalue
    static void push_new(lua_State *ls, Base *b, int pos = -1)
    {
        auto *object_wrapper = static_cast<userdata_object_t *>(lua_newuserdata(ls, sizeof(userdata_object_t)));
        new (object_wrapper) userdata_object_t;
        object_wrapper->ptr = b;
        object_wrapper->need_release = true;

        prepare_metatable(ls);
    }

    static void push(lua_State *ls, Base *b, int pos = -1)
    {
        if (b == nullptr)
        {
            lua_pushnil(ls);
            return;
        }

        auto *object_wrapper = static_cast<userdata_object_t *>(lua_newuserdata(ls, sizeof(userdata_object_t)));
        new (object_wrapper) userdata_object_t;
        object_wrapper->ptr = b;

        prepare_metatable(ls);
    }

    static void push(lua_State *ls, const Base *b, int pos = -1)
    {
        if (b == nullptr)
        {
            lua_pushnil(ls);
            return;
        }

        auto *object_wrapper = static_cast<const_userdata_object_t *>(lua_newuserdata(ls, sizeof(const_userdata_object_t)));
        new (object_wrapper) const_userdata_object_t;
        object_wrapper->ptr = b;

        prepare_metatable(ls);
    }

    static void push(lua_State *ls, Base &b, int pos = -1)
    {
        push(ls, &b, pos);
    }

    static void push(lua_State *ls, const Base &b, int pos = -1)
    {
        push(ls, &b, pos);
    }

    // peek
    static void peek(lua_State *ls, Base &b, int pos = -1)
    {
        // ZLUA_ARG_CHECK_THROW(ls, luaL_checkudata(ls, pos, type_info<Base>::name()), pos, "incorrect userdata type");
        auto *object_wrapper = static_cast<userdata_object_t *>(lua_touserdata(ls, pos));
        b = *object_wrapper->ptr;
    }

    static void peek(lua_State *ls, Base *&b, int pos = -1)
    {
        // ZLUA_ARG_CHECK_THROW(ls, luaL_checkudata(ls, pos, type_info<Base>::name()), pos, "incorrect userdata type");
        if (lua_isnil(ls, pos) != 0)
        {
            b = nullptr;
            return;
        }

        auto *object_wrapper = static_cast<userdata_object_t *>(lua_touserdata(ls, pos));
        ZLUA_ARG_CHECK_THROW(ls, !object_wrapper->is_const, pos, "cannot cast const " + type_name<Base>() + " to non-const reference");

        b = object_wrapper->ptr;
    }

    static void peek(lua_State *ls, const Base *&b, int pos = -1)
    {
        if (lua_isnil(ls, pos) != 0)
        {
            b = nullptr;
            return;
        }

        // ZLUA_ARG_CHECK_THROW(ls, luaL_checkudata(ls, pos, type_info<Base>::name()), pos, "incorrect userdata type");
        auto *object_wrapper = static_cast<userdata_object_t *>(lua_touserdata(ls, pos));
        b = object_wrapper->ptr;
    }

    // pop
    template <typename U>
    static void pop(lua_State *ls, U &u, int pos = -1)
    {
        peek(ls, u, pos);
        lua_remove(ls, pos);
    }

private:
    static void prepare_metatable(lua_State *ls)
    {
        // ZLUA_CHECK_THROW(ls, type_info<Base>::is_registered(), std::string("prepare_metatable for type <") + type_name<Base>() + "> failed, not registered");
        const char *metatable_name = type_info<Base>::metatable_name();
        luaL_setmetatable(ls, metatable_name);
    }
};

////////////////////////////////////////////////////////////////////////////////
// reference_wrapper
// <const char*>, <object_type>
template <typename T>
// struct stack_op<T, typename std::enable_if<is_reference_wrapper<T>::value>::type>
struct stack_op<reference_wrapper<T>>
{
    using wrapper_t = reference_wrapper<T>;
    using ref_t = typename wrapper_t::referenced_type;

    static void push(lua_State *ls, wrapper_t &t)
    {
        stack_op<ref_t>::push(ls, t.get_ref());
    }

    static void peek(lua_State *ls, wrapper_t &t, int pos = -1)
    {
        stack_op<ref_t>::peek(ls, t.get_ptr(), pos);
    }

    static void pop(lua_State *ls, wrapper_t &t, int pos = -1)
    {
        stack_op<ref_t>::pop(ls, t.get_ptr(), pos);
    }
};

// template <typename T>
// struct stack_op<reference_wrapper<T> &>
// {
//     using wrapper_t = reference_wrapper<T>;
//     using ref_t = typename wrapper_t::referenced_type;

//     static void push(lua_State *ls, wrapper_t &t)
//     {
//         stack_op<ref_t>::push(ls, t.get_ref());
//     }

//     static void peek(lua_State *ls, wrapper_t &t, int pos = -1)
//     {
//         stack_op<ref_t>::peek(ls, t.get_ptr(), pos);
//     }

//     static void pop(lua_State *ls, wrapper_t &t, int pos = -1)
//     {
//         stack_op<ref_t>::pop(ls, t.get_ptr(), pos);
//     }
// };

////////////////////////////////////////////////////////////////////////////////
// tuple
// read from lua stack a series of values
// or read from a table on lua stack
namespace impl
{
template <size_t N>
struct tuple_op
{
    template <typename... Args>
    static void push(lua_State *ls, std::tuple<Args...> &t)
    {
        tuple_op<N - 1>::push(ls, t);
        stack_op<decltype(std::get<N - 1>(t))>::push(ls, std::get<N - 1>(t));
    }

    template <typename... Args>
    static void pop(lua_State *ls, std::tuple<Args...> &t, int table_pos, bool reversed_order = false)
    {
        using elem_t = typename std::remove_reference<decltype(std::get<N - 1>(t))>::type;

        if (!reversed_order)
        {
            if (lua_istable(ls, table_pos) != 0)
            {
                lua_pushinteger(ls, N);
                lua_rawget(ls, table_pos);
            }

            stack_op<elem_t>::pop(ls, std::get<N - 1>(t));
        }

        tuple_op<N - 1>::pop(ls, t, table_pos, reversed_order);

        if (reversed_order)
        {
            if (lua_istable(ls, table_pos) != 0)
            {
                lua_pushinteger(ls, N);
                lua_rawget(ls, table_pos);
            }

            stack_op<elem_t>::pop(ls, std::get<N - 1>(t));
        }
    }
};

template <>
struct tuple_op<0>
{
    template <typename... Args>
    static void push(lua_State *ls, std::tuple<Args...> &t) {}

    template <typename... Args>
    static void pop(lua_State *ls, std::tuple<Args...> &t, int table_pos, bool reversed_order = false) {}
};
} // namespace impl

template <typename... Args>
struct stack_op<std::tuple<Args...>>
{
    static void push(lua_State *ls, std::tuple<Args...> &tuple)
    {
        impl::tuple_op<sizeof...(Args)>::push(ls, tuple);
    }

    // not implemented for this type
    static void peek(lua_State *ls, std::tuple<Args...> &tuple) {}

    static void pop(lua_State *ls, std::tuple<Args...> &tuple, int pos = -1)
    {
        impl::tuple_op<sizeof...(Args)>::pop(ls, tuple, pos, false);
        if (lua_istable(ls, pos) != 0)
        {
            lua_remove(ls, pos);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// TODO stl containers specialization
// template <typename T>
// struct stack_op<std::vector<T>>
// {
//     static void push(lua_State *ls, std::vector<T> &tuple)
//     {
//         // impl::tuple_op<sizeof...(Args)>::push(ls, tuple);
//     }

//     static void push_new(lua_State *ls, std::vector<T> *tuple)
//     {
//         // impl::tuple_op<sizeof...(Args)>::push(ls, tuple);
//     }

//     // not implemented for this type
//     static void peek(lua_State *ls, std::vector<T> &tuple) {}

//     static void pop(lua_State *ls, std::vector<T> &tuple, int pos = -1)
//     {
//     }
// };

} // namespace zlua