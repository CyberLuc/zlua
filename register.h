#pragma once
#include "common.h"
#include "core.h"
#include "meta.h"

namespace zlua
{

template <typename T, typename Ctor, typename... Bases>
class Registrar
{
    friend class Engine;

public:
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
        // static_assert(assert_no_pod_nonconst_ref_ptr<R, Args...>::value, "can't register function with non-const reference or pointer to arithmetic type to lua");
        using method_wrapper_t = userdata::Method<R (T::*)(Args...)>;

        luaL_getmetatable(this->ls_, type_info<T>::metatable_name());
        lua_pushstring(this->ls_, fname);

        auto *wrapper = static_cast<method_wrapper_t *>(lua_newuserdata(this->ls_, sizeof(method_wrapper_t)));
        new (wrapper) method_wrapper_t(f);

        lua_pushcclosure(this->ls_, &lua_function_forwarder<T, R, Args...>, 1);
        lua_rawset(this->ls_, -3);

        lua_pop(this->ls_, 1);
        return *this;
    }

    // member variable
    template <typename P>
    Registrar &def(const char *mname, P T::*m)
    {
        //std::cout << __PRETTY_FUNCTION__ << " " << meta::TypeInfo<Obj>::to_metatable_name() << "::" << mname << std::endl;

        luaL_getmetatable(this->ls_, type_info<T>::metatable_name());
        lua_pushstring(this->ls_, mname);

        using property_wrapper_t = userdata::PropertyWrapper<P T::*>;
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

private:
    Registrar(lua_State *ls, const char *name)
        : ls_(ls)
    {
        ZLUA_CHECK_THROW(ls, !type_info<T>::is_registered(), "register type<" + type_name<T>() + "> in name '" + name + "' failed, already registered with name " + type_info<T>::name());
        type_info<T>::set_name(name);
        type_info<T>::template inherits_from<Bases...>();

        this->name_ = name;

        this->prepare_type_table();
        this->prepare_metatable();
    }

    void prepare_type_table()
    {
        lua_newtable(this->ls_);

        lua_pushstring(this->ls_, "new");
        lua_pushcfunction(this->ls_, fetch_creator<T>((Ctor *)0));
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
