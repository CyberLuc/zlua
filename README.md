# zlua
Yet another C++/Lua bind library, a very very early version.

## Features
* Reference/Pointer Support

    Supports to use `[const] T&` and `[const] T*` to classes as function parameter types.

    You can `new` an object in lua, and pass it back to C++ function either as a reference or pointer.

    But for other plain types, such as `int`, `double` and `char`, their reference and pointer parts are prohibited. You cannot register functions with reference or pointer to these types. Just use these types directly.

    If your function's return value is some success flag and you want to pass a reference or pointer to function to get its real result, you have to find another way. Multiple return value can handle this situation.

    Moreover, non-const `std::string&` and `[const] std::string*` are prohibited to use, too.

* const/non-const Support

    All objects created in lua are non-const objects. But you can pass a const object reference or pointer from C++ to lua.

    When accessing a non-const member function or write to a member variable of a const object, zlua will throw an exception complaing constness violation.

* Inheritance Support

    TODO

* Enum Support

    TODO

* Object Lifetime Management Support

    You can create objects in lua without worrying about memory leaks or hvaing to delete it manually some time later.

    Object will be deleted automatically when lua gc detects there has no valid reference exist.

* Multiple Return Value Support

    TODO

    So-called `Multiple Return Value Support`. Actually it's just some functions that defined to return a std::tuple. zlua pushes tuple_elements onto stack individually, and you get multiple return values in lua.

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
test_ref, zlua, 1, is_same_addr true
test_ptr, zlua, 1, is_same_addr true
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
