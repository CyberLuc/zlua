#include <iostream>
#include "../zlua.h"
using namespace std;

struct Info
{
    int id = 0;
    std::string content;
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
        cout << __FUNCTION__ << ", " << role.name << ", " << role.age << endl;
    }

    void test_ptr(Role *role)
    {
        cout << __FUNCTION__ << ", " << role->name << ", " << role->age << endl;
    }

    void print_info(Info *info)
    {
        cout << __FUNCTION__ << " info.id " << info->id << ", info.content " << info->content << endl;
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
        .def("test_ref", &Role::test_ref)
        .def("test_ptr", &Role::test_ptr)
        //
        ;

    engine.load_file("./test.lua");

    return 0;
}
