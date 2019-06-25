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

    Both `enum` and `enum class` are supported. Use them at will.

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

print("Enum.Zero = " .. Enum.Zero)
print("Enum.One = " .. Enum.One)
print("Enum.Two = " .. Enum.Two)
print("Enum.Three = " .. Enum.Three)
print("Enum.Four = " .. (Enum.Four or ""))

print("EnumClass.Zero = " .. EnumClass.Zero)
print("EnumClass.One = " .. EnumClass.One)
print("EnumClass.Two = " .. EnumClass.Two)
print("EnumClass.Three = " .. EnumClass.Three)
print("EnumClass.Four = " .. (EnumClass.Four or ""))

role:print_enum(Enum.Three)
role:print_enum_class(EnumClass.Three)

````

Output:
````shell
g++ -std=c++11 -O0 -llua test.cpp -o ./test
role.name anonymous, role.age 0
print_something, 1, 2, 3
test_ref, zlua, 1, is_same_addr true
test_ptr, zlua, 1, is_same_addr true
print_info info.id 111, info.content hello from lua
Enum.Zero = 0
Enum.One = 1
Enum.Two = 2
Enum.Three = 3
Enum.Four = 
EnumClass.Zero = 0
EnumClass.One = 1
EnumClass.Two = 2
EnumClass.Three = 3
EnumClass.Four = 
print_enum 3
print_enum_class 3
````

## Todo list
* constructor support √
* reference support √
* enum support √
* more enum support: add count, validity check, etc
* inheritance support
* lua created object lifetime management
* multiple constructor support
* error handle
