#pragma once
#include "common.h"
#include "stack.h"
#include <utility>

namespace zlua
{
////////////////////////////////////////////////////////////////////////////////
// sequence
// to generate a sequence of variadic size_t <1,2,3...>
// used to unpack tuples: std::get<S>(tuple)...
////////////////////////////////////////////////////////////////////////////////
template <size_t...>
struct sequence
{
};

template <size_t N, size_t... S>
struct generate_sequence : generate_sequence<N - 1, N - 1, S...>
{
};

template <size_t... S>
struct generate_sequence<0, S...>
{
    using type = sequence<S...>;
};

template <typename... Args>
using sequence_t = typename generate_sequence<sizeof...(Args)>::type;

////////////////////////////////////////////////////////////////////////////////
// tuple_call
// to universally call a function with tuple as supplier of function parameters
////////////////////////////////////////////////////////////////////////////////
template <typename>
struct tuple_caller;

template <size_t... S>
struct tuple_caller<sequence<S...>>
{
    template <typename R, typename... Args, typename T>
    static R call(R (*f)(Args...), T &t)
    {
        return f(std::get<S>(t)...);
    }

    template <typename C, typename R, typename... Args, typename T>
    static R call(R (C::*f)(Args...), C *c, T &t)
    {
        return (c->*f)(std::get<S>(t)...);
    }
};

template <typename Ret, typename... Args, typename Tuple>
Ret tuple_call(Ret (*f)(Args...), Tuple &t)
{
    return tuple_caller<sequence_t<Args...>>::call(f, t);
};

template <typename Obj, typename Ret, typename... Args, typename Tuple>
Ret tuple_call(Ret (Obj::*f)(Args...), Obj *obj, Tuple &t)
{
    return tuple_caller<sequence_t<Args...>>::call(f, obj, t);
};

template <typename C, typename R, typename T, typename... Args>
struct WrapperCall
{
    static int call(lua_State *ls, R (C::*f)(Args...), C *c, T &t)
    {
        R ret = tuple_call(f, c, t);
        stack_op<R>::push(ls, std::forward<R>(ret));
        return 1;
    }
};

template <typename C, typename T, typename... Args>
struct WrapperCall<C, void, T, Args...>
{
    static int call(lua_State *ls, void (C::*f)(Args...), C *c, T &t)
    {
        tuple_call(f, c, t);
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
// tuple_construct
// to universally new an object with tuple as supplier of constructor parameters
////////////////////////////////////////////////////////////////////////////////
namespace impl
{
template <typename>
struct tuple_constructor;

template <size_t... S>
struct tuple_constructor<sequence<S...>>
{
    template <typename T, typename... Args>
    static T *construct(std::tuple<Args...> &params)
    {
        return new T(std::get<S>(params)...);
    }
};
} // namespace impl

template <typename T, typename... Args>
T *tuple_construct(std::tuple<Args...> &params)
{
    return impl::tuple_constructor<sequence_t<Args...>>::template construct<T>(params);
}

} // namespace zlua
