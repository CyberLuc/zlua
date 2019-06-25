# zlua
Yet another C++/Lua bind library, a very very early version.

## Usage
C++ side:
````C++ test.cpp
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
````

Lua side:
````lua test.lua
    local role = Role.new("anonymous", 0)
    print("role.name " .. role.name .. ", role.age " .. role.age)
    role.name = "zlua"
    role.age = 1
    role:print_something(1, 2, "3")
    role:test_ref(role)
    role:test_ptr(role)

    local info = Info.new()
    info.id = 111
    info.content = "hello from lua"

    role:print_info(info)
````

Output:
````shell
    role.name anonymous, role.age 0
    print_something, 1, 2, 3
    test_ref, zlua, 2
    test_ptr, zlua, 2
    print_info info.id 111, info.content hello from lua
````

## Todo list
* constructor support √
* reference support √
* enum support
* inheritance support
* lua created object lifetime management
* multiple constructor support
* error handle
