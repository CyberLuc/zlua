#pragma once
#include "common.h"
#include <string>
#include <vector>

namespace zlua
{

template <typename T>
class type_info
{
public:
    // basics
    static void set_name(const char *name, ...)
    {
        name_ = name;
        metatable_name_ = "zlua." + name_;

        va_list vl;
        va_start(vl, name);
        while ((name = va_arg(vl, const char *)) != nullptr)
        {
            inherit_from(name);
        }
        va_end(vl);
    }

    static const char *get_name() { return name_.c_str(); }
    static const char *metatable_name() { return metatable_name_.c_str(); }
    static bool is_registered() { return !name_.empty(); }

    // inheritance
    static void inherit_from(const char *name) { inherited_names_.push_back(name); }
    static bool is_inherited() { return !inherited_names_.empty(); }
    static const std::vector<std::string> &get_all_inherited_names() { return inherited_names_; }

private:
    static std::string name_;
    static std::string metatable_name_;
    static std::vector<std::string> inherited_names_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::string type_info<T>::name_;

template <typename T>
std::string type_info<T>::metatable_name_;

template <typename T>
std::vector<std::string> type_info<T>::inherited_names_;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // namespace zlua
