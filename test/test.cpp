#include <iostream>
#include "../zlua.h"
using namespace std;

struct Role
{
    std::string name;
    int age = 0;

    Role(const std::string &name_, int age_)
    // Role(const char *name_, int age_)
        : name(name_), age(age_) {}

    std::string &get_name() { return name; }
    int get_age() { return age; }

    void print_something(int a, int b, const char *p)
    {
        cout << __PRETTY_FUNCTION__ << ", " << a << ", " << b << ", " << p << endl;
    }

    void test_ref(Role& role)
    {
        cout << __PRETTY_FUNCTION__ << ", " << role.name << ", " << role.age << endl;
    }

    void test_ptr(Role* role)
    {
        cout << __PRETTY_FUNCTION__ << ", " << role->name << ", " << role->age << endl;
    }
};

Role role("zlua", 1);
int get_role(lua_State *ls)
{
    zlua::stack_op<Role>::push(ls, &role);
    return 1;
}

template<typename T>
void print_type()
{
    cout << zlua::type_name<T>() << endl;
}

void fff(int &n)
{
    n = 20;
    cout << n << endl;
}

int main()
{
    // using wrapped_tuple_t = typename zlua::wrap_tuple_reference<std::tuple<Role&>>::type;
    // print_type<wrapped_tuple_t>();

    return 0;

    zlua::Engine engine;
    engine.reg<Role, void(const char *, int)>("Role")
        .def("name", &Role::name)
        .def("age", &Role::age)
        .def("get_role", &get_role)
        .def("get_name", &Role::get_name)
        .def("get_age", &Role::get_age)
        .def("print_something", &Role::print_something)
        // .def("test_ref", &Role::test_ref)
        .def("test_ptr", &Role::test_ptr)
        //
        ;

    engine.load_file("./test.lua");

    return 0;
}
