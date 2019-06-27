#pragma once
#include "common.h"
#include "core.h"
#include "meta.h"
#include <vector>

namespace zlua
{
class Engine;
template <typename T, typename Ctor, typename... Bases>
class Registrar;

template <typename T, typename Enabled = void>
struct vector_registrar
{
    using vec_t = std::vector<T>;

    static void reg(lua_State *ls)
    {
        std::string vec_name = std::string("vector.") + type_info<T>::name();

        Registrar<vec_t, ctor()>(ls, vec_name.c_str())
            .def("push_back", (void (vec_t::*)(const T &)) & vec_t::push_back)
            .def("pop_back", &vec_t::pop_back)
            .def("at", (const T &(vec_t::*)(size_t) const) & vec_t::at)
            .def("clear", &vec_t::clear)
            .def("size", &vec_t::size)
            //
            ;

        // vector.int.new()
    }
};

template <typename T>
struct vector_registrar<T, typename std::enable_if<is_stl_container<T>::value>::type>
{
    static void reg(lua_State *ls) {}
};

template <typename T, typename Ctor>
struct prepare_type
{
    static void prepare_type_table(lua_State *ls, const char *name)
    {
        lua_newtable(ls);

        lua_pushstring(ls, "new");
        lua_pushcfunction(ls, fetch_creator<T>((Ctor *)0));
        lua_settable(ls, -3);

        lua_pushstring(ls, "clone");
        lua_pushcfunction(ls, &lua_object_cloner_wrapper<T>::clone);
        lua_settable(ls, -3);

        lua_setglobal(ls, name);
    }
};

template <typename T, typename Ctor>
struct prepare_type<std::vector<T>, Ctor>
{
    static void prepare_type_table(lua_State *ls, const char *name)
    {
        bool is_new = false;

        lua_getglobal(ls, "vector");
        if (lua_isnil(ls, -1) != 0)
        {
            lua_newtable(ls);
            is_new = true;
        }

        lua_pushstring(ls, type_info<T>::name());
        lua_newtable(ls);

        lua_pushstring(ls, "new");
        lua_pushcfunction(ls, fetch_creator<std::vector<T>>((Ctor *)0));
        lua_settable(ls, -3);

        lua_pushstring(ls, "clone");
        lua_pushcfunction(ls, &lua_object_cloner_wrapper<std::vector<T>>::clone);
        lua_settable(ls, -3);

        lua_settable(ls, -3);

        if (is_new)
        {
            lua_setglobal(ls, "vector");
        }
        else
        {
            lua_pop(ls, 1);
        }
    }
};

template <typename T, typename Ctor, typename... Bases>
class Registrar
{
    friend class Engine;

public:
    ~Registrar()
    {
        if (this->name_ != nullptr)
        {
            std::string vec_name = std::string("vector.") + type_info<T>::name();
            vector_registrar<T>::reg(this->ls_);
        }
    }

    template <typename F>
    Registrar &def(const char *fname, F f)
    {
        lua_getglobal(this->ls_, this->name_);
        lua_pushstring(this->ls_, fname);
        lua_pushcfunction(this->ls_, f);
        lua_settable(this->ls_, -3);
        return *this;
    }

    // member function
    template <typename R, typename... Args>
    Registrar &def(const char *fname, R (T::*f)(Args...))
    {
        static_assert(check_params_validity<Args...>::value,
                      "can't register function with parameter of non-const reference or pointer to non-class type to lua (except for const char*)");
        static_assert(check_return_validity<R>::value,
                      "can't register function with return type of pointer to non-class/std::string to lua (except for [const] char*)");

        using method_t = userdata::method_t<R (T::*)(Args...)>;

        luaL_getmetatable(this->ls_, type_info<T>::metatable_name());
        lua_pushstring(this->ls_, fname);

        auto *wrapper = static_cast<method_t *>(lua_newuserdata(this->ls_, sizeof(method_t)));
        new (wrapper) method_t(f);

        lua_pushcclosure(this->ls_, &lua_function_forwarder<T, R, Args...>, 1);
        lua_rawset(this->ls_, -3);

        lua_pop(this->ls_, 1);
        return *this;
    }

    // member function
    template <typename R, typename... Args>
    Registrar &def(const char *fname, R (T::*f)(Args...) const)
    {
        static_assert(check_params_validity<Args...>::value,
                      "can't register function with parameter of non-const reference or pointer to non-class type to lua (except for const char*)");
        static_assert(check_return_validity<R>::value,
                      "can't register function with return type of pointer to non-class/std::string to lua (except for [const] char*)");

        using method_t = userdata::method_t<R (T::*)(Args...) const>;

        luaL_getmetatable(this->ls_, type_info<T>::metatable_name());
        lua_pushstring(this->ls_, fname);

        auto *wrapper = static_cast<method_t *>(lua_newuserdata(this->ls_, sizeof(method_t)));
        new (wrapper) method_t(f);

        lua_pushcclosure(this->ls_, &lua_function_forwarder<T, R, Args...>, 1);
        lua_rawset(this->ls_, -3);

        lua_pop(this->ls_, 1);
        return *this;
    }

    // member variable
    template <typename P>
    Registrar &def(const char *mname, P T::*m)
    {
        luaL_getmetatable(this->ls_, type_info<T>::metatable_name());
        lua_pushstring(this->ls_, mname);

        using property_wrapper_t = userdata::property_wrapper_t<P T::*>;
        auto *property_wrapper = (property_wrapper_t *)lua_newuserdata(this->ls_, sizeof(property_wrapper_t));
        new (property_wrapper) property_wrapper_t;

        property_wrapper->access_handler = &access_property_function<T, P>;
        property_wrapper->write_handler = &write_property_function<T, P>;
        property_wrapper->property_holder = m;
        property_wrapper->property = static_cast<void *>(&property_wrapper->property_holder);

        lua_rawset(this->ls_, -3);

        lua_pop(this->ls_, 1);
        return *this;
    }

    template <typename... Ts>
    Registrar &inherit()
    {
        type_info<T>::template inherit_from<Ts...>();
        return *this;
    }

    // private:
    Registrar(lua_State *ls, const char *name)
        : ls_(ls)
    {
        ZLUA_CHECK_THROW(ls, !type_info<T>::is_registered(), "register type<" + type_name<T>() + "> in name '" + name + "' failed, already registered with name " + type_info<T>::name());
        type_info<T>::set_name(name);

        type_info<T>::template inherit_from<Bases...>();

        this->name_ = name;

        prepare_type<T, Ctor>::prepare_type_table(ls, name);

        // this->prepare_type_table();
        this->prepare_metatable();
    }

    void prepare_type_table()
    {
        lua_newtable(this->ls_);

        lua_pushstring(this->ls_, "new");
        lua_pushcfunction(this->ls_, fetch_creator<T>((Ctor *)0));
        lua_settable(this->ls_, -3);

        lua_pushstring(this->ls_, "clone");
        lua_pushcfunction(this->ls_, &lua_object_cloner_wrapper<T>::clone);
        lua_settable(this->ls_, -3);

        lua_setglobal(this->ls_, this->name_);
    }

    void prepare_metatable()
    {
        luaL_newmetatable(this->ls_, type_info<T>::metatable_name());

        lua_pushstring(this->ls_, "__index");
        lua_pushcfunction(this->ls_, (&metatable_index_function<T>));
        lua_rawset(this->ls_, -3);

        lua_pushstring(this->ls_, "__newindex");
        lua_pushcfunction(this->ls_, (&metatable_newindex_function<T>));
        lua_rawset(this->ls_, -3);

        lua_pushstring(this->ls_, "__gc");
        lua_pushcfunction(this->ls_, (&lua_object_deleter<T>));
        lua_rawset(this->ls_, -3);

        lua_pop(this->ls_, 1);
    }

    // forbid assignment/copy ctor
    Registrar(const Registrar &) = delete;
    Registrar &operator=(const Registrar &) = delete;
    Registrar(Registrar &&rhs) : ls_(rhs.ls_), name_(rhs.name_)
    {
        rhs.ls_ = nullptr;
        rhs.name_ = nullptr;
    }

    lua_State *ls_;
    const char *name_;
};

template <typename E>
class EnumRegistrar
{
public:
    friend class Engine;

public:
    EnumRegistrar &def(const char *ename, E e)
    {
        lua_getglobal(this->ls_, this->name_);

        lua_pushstring(this->ls_, ename);
        lua_pushinteger(this->ls_, static_cast<typename std::underlying_type<E>::type>(e));
        lua_settable(this->ls_, -3);

        return *this;
    }

private:
    EnumRegistrar(lua_State *ls, const char *name)
        : ls_(ls)
    {
        // ZLUA_CHECK_THROW(ls, !type_info<T>::is_registered(), "register type<" + type_name<T>() + "> in name '" + name + "' failed, already registered with name " + type_info<T>::name());
        type_info<E>::set_name(name);
        this->name_ = name;

        lua_newtable(this->ls_);
        lua_setglobal(this->ls_, this->name_);
    }

    // forbid assignment/copy ctor
    EnumRegistrar(const EnumRegistrar &) = delete;
    EnumRegistrar &operator=(const EnumRegistrar &) = delete;
    EnumRegistrar(EnumRegistrar &&rhs) : ls_(rhs.ls_), name_(rhs.name_)
    {
        rhs.ls_ = nullptr;
        rhs.name_ = nullptr;
    }

    lua_State *ls_;
    const char *name_;
};

} // namespace zlua
