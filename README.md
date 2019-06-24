# zlua
Yet another C++/Lua bind library, a very very early version.

## usage
````C++ test.cpp
    #include <iostream>
    #include "zlua.h"
    using namespace std;

    struct Role
    {
        std::string name;
        int age = 0;

        std::string &get_name() { return name; }
        int get_age() { return age; }

        void print_something(int a, int b, const char *p)
        {
            cout << __PRETTY_FUNCTION__ << ", " << a << ", " << b << ", " << p << endl;
        }
    };

    int main {
        zlua::Engine engine;
        engine.reg<Role>("Role")
            .def("name", &Role::name)
            .def("age", &Role::age)
            .def("get_role", &get_role)
            .def("get_name", &Role::get_name)
            .def("get_age", &Role::get_age)
            .def("print_something", &Role::print_something);

        engine.load_file("./test.lua");

        return 0;
    }
````

````lua test.lua
    local role = Role.new("Tom", 18)
    print(role.name, role.age)
    role:print_something(1, 2, "3")
````

## Todo list
* constructor support
* reference support
* inheritance support
* lua created object lifetime management
* enum support
* error handle
