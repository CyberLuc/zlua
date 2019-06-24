#pragma once
#include "common.h"
#include "stack.h"
#include <utility>

namespace zlua
{

////////////////////////////////////////////////////////////////////////////////

template <bool B, bool... Bs>
struct and_t
{
    const static bool value = B && and_t<Bs...>::value;
};

template <bool B>
struct and_t<B>
{
    const static bool value = B;
};

template <bool B, bool... Bs>
struct or_t
{
    const static bool value = B || and_t<Bs...>::value;
};

template <bool B>
struct or_t<B>
{
    const static bool value = B;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// to generate a sequence of variadic ints <1,2,3...>
template <int...>
struct sequence
{
};

template <int N, int... S>
struct generate_sequence : generate_sequence<N - 1, N - 1, S...>
{
};

template <int... S>
struct generate_sequence<0, S...>
{
    using type = sequence<S...>;
};

template <typename... Args>
using sequence_t = typename generate_sequence<sizeof...(Args)>::type;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// TODO tuple_caller change to callable object invoke
template <typename>
struct tuple_caller;

template <size_t... S>
struct tuple_caller<sequence<S...>>
{
    template <typename R, typename... Args, typename T>
    static R call(R (*f)(Args...), T &&t)
    {
        return f(std::get<S>(t)...);
    }

    template <typename C, typename R, typename... Args, typename T>
    static R call(R (C::*f)(Args...), C *c, T &&t)
    {
        return (c->*f)(std::get<S>(t)...);
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename Ret, typename... Args, typename Tuple>
Ret tuple_call(Ret (*f)(Args...), Tuple &&t)
{
    return tuple_caller<sequence_t<Args...>>::call(f, t);
};

template <typename Obj, typename Ret, typename... Args, typename Tuple>
Ret tuple_call(Ret (Obj::*f)(Args...), Obj *obj, Tuple &&t)
{
    return tuple_caller<sequence_t<Args...>>::call(f, obj, t);
};

template <typename C, typename R, typename T, typename... Args>
struct WrapperCall
{
    static int call(lua_State *ls, R (C::*f)(Args...), C *c, const T &t)
    {
        R ret = tuple_call(f, c, t);
        stack_op<R>::push(ls, std::forward<R>(ret));
        return 1;
    }
};

template <typename C, typename T, typename... Args>
struct WrapperCall<C, void, T, Args...>
{
    static int call(lua_State *ls, void (C::*f)(Args...), C *c, const T &t)
    {
        tuple_call(f, c, t);
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// invoke

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <size_t N>
struct tuple_filler
{
    template <typename... Args>
    static void fill(lua_State *ls, std::tuple<Args...> &t)
    {
        using T = decltype(std::get<N - 1>(t));
        stack_op<base_type_t<T>>::pop(ls, std::get<N - 1>(t));
        tuple_filler<N - 1>::fill(ls, t);
    }
};

template <>
struct tuple_filler<0>
{
    template <typename... Args>
    static void fill(lua_State *ls, std::tuple<Args...> &t) {}
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename>
struct tuple_construct
{
    template <typename T, typename... Args>
    static T *construct(std::tuple<Args...> &&params)
    {
        return nullptr;
    }
};

template <size_t... S>
struct tuple_construct<sequence<S...>>
{
    template <typename T, typename... Args>
    static T *construct(std::tuple<Args...> &params)
    {
        return new T(std::get<S>(params)...);
    }
};

template <typename T>
struct object_creator
{
    template <typename R, typename... Args>
    static T *create(lua_State *ls, R (*)(Args...))
    {
        using wrapped_tuple_t = typename wrap_tuple_reference<std::tuple<Args...>>::type;
        wrapped_tuple_t params;
        tuple_filler<sizeof...(Args)>::fill(ls, params);

        return tuple_construct<sequence_t<Args...>>::template construct<T>(params);
    }

    template <typename R>
    static T *create(lua_State *ls, R (*)())
    {
        return new T;
    }
};

} // namespace zlua