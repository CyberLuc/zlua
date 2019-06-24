#pragma once
#include <exception>
#include <string>

#define ZLUA_ARG_CHECK_THROW(ls, cond, idx, msg)     \
    if (!(cond))                                     \
    {                                                \
        throw exception(msg, lua_absindex(ls, idx)); \
    }

#define ZLUA_CHECK_THROW(ls, cond, msg) \
    if (!(cond))                        \
    {                                   \
        throw exception(msg);           \
    }

namespace zlua
{

class exception : public std::exception
{
public:
    exception(const char *detail, int arg_idx = -1) { this->prepare_error_msg(detail, arg_idx); }
    exception(const std::string &detail, int arg_idx = -1) : exception(detail.c_str(), arg_idx) {}
    virtual const char *what() const noexcept { return this->msg_; }

private:
    void prepare_error_msg(const char *detail, int arg_idx)
    {
        if (arg_idx > -1)
        {
            snprintf(this->msg_, sizeof(this->msg_), "(bad argument #%d ('%s'))", arg_idx, detail);
        }
        else
        {
            snprintf(this->msg_, sizeof(this->msg_), "(bad operands ('%s'))", detail);
        }
    }

    char msg_[128];
};

} // namespace zlua