#pragma once
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <utility>
#include <type_traits>

namespace zlua
{

template <typename T>
struct remove_crp
{
    using type =
        typename std::remove_const<
            typename std::remove_pointer<
                typename std::remove_reference<
                    T>::type>::type>::type;
};
template <typename T>
using remove_crp_t = typename remove_crp<T>::type;

template <typename T>
struct remove_cr
{
    using type =
        typename std::remove_const<
            typename std::remove_reference<
                T>::type>::type;
};
template <typename T>
using remove_cr_t = typename remove_cr<T>::type;

template <typename T>
struct remove_cp
{
    using type =
        typename std::remove_const<
            typename std::remove_pointer<
                T>::type>::type;
};
template <typename T>
using remove_cp_t = typename remove_cp<T>::type;

template <typename T>
struct remove_rp
{
    using type =
        typename std::remove_pointer<
            typename std::remove_reference<
                T>::type>::type;
};

template <typename T>
using remove_rp_t = typename remove_rp<T>::type;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct pointed
{
    using type = typename std::remove_pointer<T>::type;
};

template <typename T>
using pointed_type_t = typename pointed<T>::type;

template <typename T>
struct referenced
{
    using type = typename std::remove_reference<T>::type;
};

template <typename T>
using referenced_type_t = typename referenced<T>::type;

template <typename T>
struct unwrap_reference;

template <typename T>
using base_type_t = typename remove_crp<typename std::remove_extent<typename unwrap_reference<T>::type>::type>::type;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct reference_wrapper
{
    reference_wrapper() : ptr(nullptr) {}
    reference_wrapper(T &t) : ptr(&t) {}
    reference_wrapper(const reference_wrapper<T> &w) : ptr(w.ptr) {}

    T &get() { return *this->ptr; }
    const T &get() const { return *this->ptr; }

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

    operator const T &() const
    {
        return *this->ptr;
    }

private:
    T *ptr;
};

template <typename T, typename Enabled = void>
struct filter
{
    using type = T;
};

template <typename T>
struct filter<T, typename std::enable_if< std::is_same<base_type_t<T>, std::string>::value>::type>
{
    using type = const char*;
};

// T -> T
// T& -> std::reference_wrapper<T>
// const T& -> std::reference_wrapper<const T>
template <typename T, typename Enabled = void>
struct wrap_reference
{
    using type = typename filter<T>::type;
};

template<typename T>
struct is_string_type;

template <typename T>
struct wrap_reference<T, typename std::enable_if<
            std::is_reference<T>::value && !is_string_type<T>::value >::type>
{
    using type = reference_wrapper<typename std::remove_reference<typename filter<T>::type>::type>;
};

template <typename T>
struct unwrap_reference
{
    using type = T;
};

template <typename T>
struct unwrap_reference<reference_wrapper<T>>
{
    using type = T &;
};

template <typename T>
struct unwrap_reference<reference_wrapper<const T>>
{
    using type = const T &;
};

template <typename tuple_t>
struct wrap_tuple_reference
{
    template <typename T>
    struct detail
    {
        using type = std::tuple<typename wrap_reference<typename filter<T>::type>::type>;
    };

    template <typename... Args>
    struct detail<std::tuple<Args...>>
    {
        using type = std::tuple<typename wrap_reference<Args>::type...>;
    };

    using type = typename detail<tuple_t>::type;
};

template <typename tuple_t>
struct unwrap_tuple_reference
{
    template <typename T>
    struct unwrap_helper;

    template <typename... Args>
    struct unwrap_helper<std::tuple<Args...>>
    {
        using type = std::tuple<typename unwrap_reference<Args>::type...>;
    };

    using type = typename unwrap_helper<tuple_t>::type;
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename T, typename R>
using const_member_variable_pointer_t = const R(T::*);

template <typename T, typename R>
using member_variable_pointer_t = R(T::*);

template <typename T, typename Enabled = void>
struct is_const_member_variable_pointer
{
    const static bool value = false;
};

template <typename T>
struct is_const_member_variable_pointer<T, typename std::enable_if<std::is_member_object_pointer<T>::value>::type>
{
    template <typename U, typename R>
    static char detail(member_variable_pointer_t<U, R>);
    template <typename U, typename R>
    static int detail(const_member_variable_pointer_t<U, R>);

    const static bool value = sizeof(decltype(detail(T()))) == sizeof(int);
};

template <typename T, typename Enabled = void>
struct remove_member_variable_pointer_const
{
    using type = T;
};

template <typename T>
struct remove_member_variable_pointer_const<T, typename std::enable_if<
                                                   is_const_member_variable_pointer<T>::value>::type>
{
    template <typename U, typename R>
    static member_variable_pointer_t<U, R> detail(const_member_variable_pointer_t<U, R>);

    using type = decltype(detail(T()));
};

template <typename T>
using remove_member_variable_pointer_const_t = typename remove_member_variable_pointer_const<T>::type;

template <typename T, typename Enabled = void>
struct add_member_variable_pointer_const
{
    using type = T;
};

template <typename T>
struct add_member_variable_pointer_const<T, typename std::enable_if<
                                                !is_const_member_variable_pointer<T>::value>::type>
{
    template <typename U, typename R>
    static const_member_variable_pointer_t<U, R> detail(member_variable_pointer_t<U, R>);

    using type = decltype(detail(T()));
};

template <typename T>
using add_member_variable_pointer_const_t = typename add_member_variable_pointer_const<T>::type;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
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

template <typename T, typename Enabled = void>
struct remove_member_function_pointer_const
{
    using type = T;
};

template <typename T>
struct remove_member_function_pointer_const<T, typename std::enable_if<
                                                   is_const_member_function_pointer<T>::value>::type>
{
    template <typename U, typename R, typename... Args>
    static member_function_pointer_t<U, R, Args...> detail(const_member_function_pointer_t<U, R, Args...>);

    using type = decltype(detail(T()));
};

template <typename T>
using remove_member_function_pointer_const_t = typename remove_member_function_pointer_const<T>::type;

template <typename T, typename Enabled = void>
struct add_member_function_pointer_const
{
    using type = T;
};

template <typename T>
struct add_member_function_pointer_const<T, typename std::enable_if<
                                                !is_const_member_function_pointer<T>::value>::type>
{
    template <typename U, typename R, typename... Args>
    static const_member_function_pointer_t<U, R, Args...> detail(member_function_pointer_t<U, R, Args...>);

    using type = decltype(detail(T()));
};

template <typename T>
using add_member_function_pointer_const_t = typename add_member_function_pointer_const<T>::type;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct is_stl_container
{
    using Base = base_type_t<T>;

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

    template <typename U>
    static char detail(U *);

    const static bool value = sizeof(decltype(detail((T *)(nullptr)))) == sizeof(int);
};

template <typename T>
struct is_integral_type
{
    const static bool value = std::is_integral<T>::value && !std::is_same<char, T>::value && !std::is_same<bool, T>::value;
};

template <typename T>
struct is_string_type
{
    const static bool value = std::is_same<T, char>::value || std::is_same<T, std::string>::value;
};

template <typename T>
struct is_float_type
{
    const static bool value = std::is_floating_point<T>::value;
};

template <typename T>
struct is_bool_type
{
    const static bool value = std::is_same<T, bool>::value;
};

template <typename T>
struct is_object_type
{
    const static bool value =
        std::is_class<T>::value && !is_stl_container<T>::value;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // namespace zlua