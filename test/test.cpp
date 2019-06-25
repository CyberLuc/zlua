#include <iostream>
#include "../zlua.h"
using namespace std;

struct Info
{
    int id = 0;
    std::string content;
};

enum Enum
{
    Zero,
    One,
    Two,
    Three,
};

enum class EnumClass
{
    Zero,
    One,
    Two,
    Three,
};

struct Role
{
    std::string name;
    int age;

    Role(const std::string &name_, int age_)
        : name(name_), age(age_) {}

    std::string &get_name() { return name; }
    int get_age() { return age; }

    void print_something(int a, int b, const char *p)
    {
        cout << __FUNCTION__ << ", " << a << ", " << b << ", " << p << endl;
    }

    void test_ref(Role &role)
    {
        cout << __FUNCTION__ << ", " << role.name << ", " << role.age << ", is_same_addr " << boolalpha << (this == &role) << endl;
    }

    void test_ptr(Role *role)
    {
        cout << __FUNCTION__ << ", " << role->name << ", " << role->age << ", is_same_addr " << boolalpha << (this == role) << endl;
    }

    void print_info(Info *info)
    {
        cout << __FUNCTION__ << " info.id " << info->id << ", info.content " << info->content << endl;
    }

    void print_enum(Enum e)
    {
        cout << __FUNCTION__ << " " << e << endl;
    }

    void print_enum_class(EnumClass e)
    {
        cout << __FUNCTION__ << " " << static_cast<typename std::underlying_type<EnumClass>::type>(e) << endl;
    }
};

template <typename T>
void print_type()
{
    cout << zlua::type_name<T>() << endl;
}

int main()
{
    zlua::Engine engine;
    engine.reg<Info, ctor()>("Info")
        .def("id", &Info::id)
        .def("content", &Info::content)
        //
        ;

    engine.reg<Role, ctor(const std::string &, int)>("Role")
        .def("name", &Role::name)
        .def("age", &Role::age)
        .def("get_name", &Role::get_name)
        .def("get_age", &Role::get_age)
        .def("print_something", &Role::print_something)
        .def("print_info", &Role::print_info)
        .def("print_enum", &Role::print_enum)
        .def("print_enum_class", &Role::print_enum_class)
        .def("test_ref", &Role::test_ref)
        .def("test_ptr", &Role::test_ptr)
        //
        ;

    engine.reg<Enum>("Enum")
        .def("Zero", Enum::Zero)
        .def("One", Enum::One)
        .def("Two", Enum::Two)
        .def("Three", Enum::Three)
        //
        ;

    engine.reg<EnumClass>("EnumClass")
        .def("Zero", EnumClass::Zero)
        .def("One", EnumClass::One)
        .def("Two", EnumClass::Two)
        .def("Three", EnumClass::Three)
        //
        ;

    engine.load_file("./test.lua");

    return 0;
}
