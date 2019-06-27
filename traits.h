#pragma once
#include <utility>
#include <type_traits>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace zlua
{
////////////////////////////////////////////////////////////////////////////////
// is_stl_container
// these containers are specially treated
// thus sometimes need to be distinguished from other object types
////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct is_stl_container
{
    template <typename K, typename V>
    static int detail(std::map<K, V> *);

    template <typename K, typename V>
    static int detail(std::unordered_map<K, V> *);

    template <typename K, typename V>
    static int detail(std::set<K, V> *);

    template <typename K, typename V>
    static int detail(std::unordered_set<K, V> *);

    template <typename K>
    static int detail(std::vector<K> *);

    template <typename K>
    static int detail(std::list<K> *);

    template <typename... Args>
    static int detail(std::tuple<Args...> *);

    template <typename U>
    static char detail(U *);

    const static bool value = sizeof(decltype(detail((T *)(nullptr)))) == sizeof(int);
};

template <typename T>
struct is_integral_type
{
    const static bool value =
        (std::is_integral<T>::value ||
         std::is_enum<T>::value) &&
        !std::is_same<char, T>::value &&
        !std::is_same<bool, T>::value;
};

template <typename T>
struct is_string_type
{
    const static bool value = std::is_same<T, char>::value ||
                              std::is_same<T, std::string>::value;
};

template <typename T>
struct is_object_type
{
    const static bool value =
        std::is_class<T>::value &&
        !is_stl_container<T>::value;
};

template <typename T>
struct is_tuple_type
{
    template <typename... Args>
    static int detail(std::tuple<Args...> *);

    template <typename U>
    static char detail(U *);

    const static bool value = sizeof(decltype(detail((T *)(nullptr)))) == sizeof(int);
};

template <typename T>
struct reference_wrapper;

template <typename T>
struct remove_reference_wrapper;

template <typename T>
using base_type_t =
    typename std::remove_const<
        typename std::remove_pointer<
            typename std::remove_reference<
                typename remove_reference_wrapper<
                    typename std::remove_reference<
                        T>::
                        type>::
                    type>::
                type>::
            type>::
        type;

template <typename T>
struct is_reference_wrapper
{
    const static bool value = false;
};

template <typename T>
struct is_reference_wrapper<reference_wrapper<T>>
{
    const static bool value = true;
};

template <typename T>
struct remove_reference_wrapper
{
    using type = T;
};

template <typename T>
struct remove_reference_wrapper<reference_wrapper<T>>
{
    using type = T &;
};

////////////////////////////////////////////////////////////////////////////////
// reference_wrapper
// to allow registered function to use reference parameters
// which are initialized and read and passed to function call seperately
////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct reference_wrapper
{
    using type = T &;
    using referenced_type = T;

    reference_wrapper() : ptr(nullptr) {}
    reference_wrapper(T &t) : ptr(&t) {}
    reference_wrapper(const reference_wrapper<T> &w) : ptr(w.ptr) {}

    T &get_ref() { return *this->ptr; }
    const T &get_ref() const { return *this->ptr; }

    T *&get_ptr() { return this->ptr; }

    reference_wrapper<T> &operator=(const T &t)
    {
        this->ptr = &t;
        return *this;
    }
    reference_wrapper<T> &operator=(const reference_wrapper<T> &t)
    {
        this->ptr = t.ptr;
        return *this;
    }

    reference_wrapper<T> &assign(T &t)
    {
        this->ptr = &t;
        return *this;
    }

    bool has_ref() const { return this->ptr != nullptr; }

    operator T &()
    {
        return *this->ptr;
    }

    operator const T &() const // ???
    {
        return *this->ptr;
    }

private:
    T *ptr;
};

template <typename T>
struct element_size
{
    const static size_t value = 1;
};

template <typename... Args>
struct element_size<std::tuple<Args...>>
{
    const static size_t value = sizeof...(Args);
};

template <>
struct element_size<void>
{
    const static size_t value = 0;
};

////////////////////////////////////////////////////////////////////////////////

// template <typename T, typename R>
// using const_member_variable_pointer_t = const R(T::*);

// template <typename T, typename R>
// using member_variable_pointer_t = R(T::*);

// template <typename T, typename Enabled = void>
// struct is_const_member_variable_pointer
// {
//     const static bool value = false;
// };

// template <typename T>
// struct is_const_member_variable_pointer<T, typename std::enable_if<std::is_member_object_pointer<T>::value>::type>
// {
//     template <typename U, typename R>
//     static char detail(member_variable_pointer_t<U, R>);
//     template <typename U, typename R>
//     static int detail(const_member_variable_pointer_t<U, R>);

//     const static bool value = sizeof(decltype(detail(T()))) == sizeof(int);
// };

// template <typename T, typename Enabled = void>
// struct remove_member_variable_pointer_const
// {
//     using type = T;
// };

// template <typename T>
// struct remove_member_variable_pointer_const<T, typename std::enable_if<
//                                                    is_const_member_variable_pointer<T>::value>::type>
// {
//     template <typename U, typename R>
//     static member_variable_pointer_t<U, R> detail(const_member_variable_pointer_t<U, R>);

//     using type = decltype(detail(T()));
// };

// template <typename T>
// using remove_member_variable_pointer_const_t = typename remove_member_variable_pointer_const<T>::type;

// template <typename T, typename Enabled = void>
// struct add_member_variable_pointer_const
// {
//     using type = T;
// };

// template <typename T>
// struct add_member_variable_pointer_const<T, typename std::enable_if<
//                                                 !is_const_member_variable_pointer<T>::value>::type>
// {
//     template <typename U, typename R>
//     static const_member_variable_pointer_t<U, R> detail(member_variable_pointer_t<U, R>);

//     using type = decltype(detail(T()));
// };

// template <typename T>
// using add_member_variable_pointer_const_t = typename add_member_variable_pointer_const<T>::type;

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename R, typename... Args>
using const_member_function_pointer_t = R (T::*)(Args...) const;

template <typename T, typename R, typename... Args>
using member_function_pointer_t = R (T::*)(Args...);

template <typename T, typename Enabled = void>
struct is_const_member_function_pointer;

template <typename T>
struct is_const_member_function_pointer<T, typename std::enable_if<
                                               std::is_member_function_pointer<T>::value>::type>
{
    template <typename U, typename R, typename... Args>
    static char detail(member_function_pointer_t<U, R, Args...>);
    template <typename U, typename R, typename... Args>
    static int detail(const_member_function_pointer_t<U, R, Args...>);

    const static bool value = sizeof(decltype(detail(T()))) == sizeof(int);
};

// template <typename T, typename Enabled = void>
// struct remove_member_function_pointer_const
// {
//     using type = T;
// };

// template <typename T>
// struct remove_member_function_pointer_const<T, typename std::enable_if<
//                                                    is_const_member_function_pointer<T>::value>::type>
// {
//     template <typename U, typename R, typename... Args>
//     static member_function_pointer_t<U, R, Args...> detail(const_member_function_pointer_t<U, R, Args...>);

//     using type = decltype(detail(T()));
// };

// template <typename T>
// using remove_member_function_pointer_const_t = typename remove_member_function_pointer_const<T>::type;

// template <typename T, typename Enabled = void>
// struct add_member_function_pointer_const
// {
//     using type = T;
// };

// template <typename T>
// struct add_member_function_pointer_const<T, typename std::enable_if<
//                                                 !is_const_member_function_pointer<T>::value>::type>
// {
//     template <typename U, typename R, typename... Args>
//     static const_member_function_pointer_t<U, R, Args...> detail(member_function_pointer_t<U, R, Args...>);

//     using type = decltype(detail(T()));
// };

// template <typename T>
// using add_member_function_pointer_const_t = typename add_member_function_pointer_const<T>::type;

////////////////////////////////////////////////////////////////////////////////
// pack_tuple, pack_tuple_t
// packs Args... to std::tuple<Args...> , but:
//   replace <[const] std::string [&]> with <const char*>
//   replace <[const] T &> with <reference_wrapper<[const] T>>
// attensions:
//   <char *> is replaced with <const char*>
//   <[const] std::string *> is replaced with <const char*>
//   but these types should be rejected by check_params_validity when register function
//   and thus never be used
////////////////////////////////////////////////////////////////////////////////
namespace impl
{
template <typename T, typename Enabled = void>
struct pack_element
{
    using type = T;
};

template <>
struct pack_element<void>
{
    using type = void *; // to suppress empty arguments error
};

template <typename T>
struct pack_element<T, typename std::enable_if<is_string_type<base_type_t<T>>::value>::type>
{
    using type = const char *;
};

template <typename T>
struct pack_element<T, typename std::enable_if<std::is_reference<T>::value &&
                                               !is_string_type<base_type_t<T>>::value>::type>
{
    using type = reference_wrapper<typename std::remove_reference<T>::type>;
};
} // namespace impl

template <typename... Args>
struct pack_tuple
{
    using type = std::tuple<typename impl::pack_element<Args>::type...>;
};

template <typename... Args>
using pack_tuple_t = typename pack_tuple<Args...>::type;

////////////////////////////////////////////////////////////////////////////////
// check_params_validity
// check registered function parameters validity
// rejects [const] reference/pointer to non-class type, except for const char*
////////////////////////////////////////////////////////////////////////////////
namespace impl
{
template <typename T, typename Enabled = void>
struct check_param
{
    const static bool value = true;
};

template <typename T>
struct check_param<T, typename std::enable_if<std::is_reference<T>::value>::type>
{
    // only accepts [const] reference to class
    const static bool value = std::is_class<base_type_t<T>>::value;
};

template <typename T>
struct check_param<T, typename std::enable_if<std::is_pointer<T>::value>::type>
{
    // only accepts [const] pointer to class and const char*
    // rejects char* and [const] std::string*
    const static bool value = std::is_same<T, const char *>::value ||
                              (std::is_class<base_type_t<T>>::value && !is_string_type<base_type_t<T>>::value);
};
} // namespace impl

template <typename... Args>
struct check_params_validity;

template <typename A, typename... Args>
struct check_params_validity<A, Args...>
{
    const static bool value = impl::check_param<A>::value && check_params_validity<Args...>::value;
};

template <typename A>
struct check_params_validity<A>
{
    const static bool value = impl::check_param<A>::value;
};

template <>
struct check_params_validity<>
{
    const static bool value = true;
};

////////////////////////////////////////////////////////////////////////////////
// check_return_validity
// check registered function return type validity
// rejects pointer to non-class type, except for [const] char*
////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct check_return_validity
{
    const static bool value = !std::is_pointer<T>::value ||
                              (std::is_class<base_type_t<T>>::value && !std::is_same<base_type_t<T>, std::string>::value) ||
                              std::is_same<base_type_t<T>, char>::value;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace zlua