#pragma once
#include "common.h"
#include <string>
#include <vector>

namespace zlua
{
struct register_counter
{
    static int type_id_cnt;
};
int register_counter::type_id_cnt = 0;

template <typename Derived, typename Base>
size_t calc_base_offset()
{
    Derived *pd = (Derived *)0x100;
    Base *pb = (Base *)pd;
    auto diff = (char *)pb - (char *)pd;
    return diff;
}

template <typename T>
class type_info;

struct inheritance_info
{
    std::string name;
    size_t offset;
};

namespace impl
{

template <typename... T>
struct inherit_helper;

template <typename T, typename Base>
struct inherit_helper<T, Base>
{
    static void inherit()
    {
        assert(type_info<Base>::is_registered() && "inherited base type is not registered");

        inheritance_info info;
        info.name = type_info<Base>::name();
        info.offset = calc_base_offset<T, Base>();

        type_info<T>::add_inheritance_info(info);
    }
};

template <typename T, typename Base, typename... Bases>
struct inherit_helper<T, Base, Bases...>
{
    static void inherit()
    {
        inherit_helper<T, Base>::inherit();
        inherit_helper<T, Bases...>::inherit();
    }
};

template <typename T>
struct inherit_helper<T>
{
    static void inherit() {}
};

} // namespace impl

template <typename T>
class type_info
{
public:
    // basics
    static void set_name(const char *name)
    {
        name_ = name;
        metatable_name_ = "zlua." + name_;
        type_idx_ = ++register_counter::type_id_cnt;
    }

    static const char *name() { return name_.c_str(); }
    static const char *metatable_name() { return metatable_name_.c_str(); }
    static int type_idx() { return type_idx_; }

    static bool is_registered() { return !name_.empty(); }

    // inheritance
    template <typename... Bases>
    static void inherits_from()
    {
        impl::inherit_helper<T, Bases...>::inherit();
    }

    static void add_inheritance_info(const inheritance_info &info)
    {
        inheritance_info_.push_back(info);
    }
    static bool is_inherited() { return !inheritance_info_.empty(); }
    static const std::vector<inheritance_info> &get_inheritance_info() { return inheritance_info_; }

private:
    static std::string name_;
    static std::string metatable_name_;
    static std::vector<inheritance_info> inheritance_info_;
    static int type_idx_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::string type_info<T>::name_;

template <typename T>
std::string type_info<T>::metatable_name_;

template <typename T>
std::vector<inheritance_info> type_info<T>::inheritance_info_;

template <typename T>
int type_info<T>::type_idx_ = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // namespace zlua
