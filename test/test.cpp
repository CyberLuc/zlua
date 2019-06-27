#include <iostream>
#include "../zlua.h"
using namespace std;

enum Enum
{
    Zero,
    One,
    Two,
    Three,
};

class SuperBase
{
public:
    virtual void say()
    {
        cout << __FUNCTION__ << " from SuperBase" << endl;
    }

    virtual void say3()
    {
        cout << __FUNCTION__ << " from SuperBase" << endl;
    }
};

class Base1 : public SuperBase
{
public:
    // virtual void say()
    // {
    //     cout << __FUNCTION__ << " from Base1" << endl;
    // }

    void say2()
    {
        cout << __FUNCTION__ << " from Base1" << endl;
    }
};

class Base2
{
public:
    virtual void say()
    {
        cout << __FUNCTION__ << " from Base2" << endl;
    }

    char s;
};

class Derived : public Base2, public Base1
{
public:
    virtual void say()
    {
        cout << __FUNCTION__ << " from Derived" << endl;
    }

    void say2()
    {
        cout << __FUNCTION__ << " from Derived" << endl;
    }

    Base1 *to_base1()
    {
        return this;
    }
};

Derived d;

int getd(lua_State *ls)
{
    zlua::stack_op<Derived>::push(ls, (const Derived *)&d);
    return 1;
}

template <typename T>
void fun_template() {}

template <typename R, typename... Args>
std::pair<R, std::tuple<Args...>> deduce_func(R (*)(Args...));

int main()
{
    zlua::Engine engine;
    auto ls = engine.get_lua_state();

    engine.reg<Enum>("Enum")
        .def("Zero", Enum::Zero)
        .def("One", Enum::One)
        .def("Two", Enum::Two)
        .def("Three", Enum::Three)
        //
        ;

    engine.reg<SuperBase, ctor()>("SuperBase")
        .def("say", &SuperBase::say)
        .def("say3", &SuperBase::say3)
        //
        ;

    engine.reg<Base1, ctor(), SuperBase>("Base1")
        // .def("say", &Base1::say)
        .def("say2", &Base1::say2)
        //
        ;

    engine.reg<Base2, ctor()>("Base2")
        .def("say", &Base2::say)
        //
        ;

    engine.reg<Derived, ctor(), Base2, Base1, SuperBase>("Derived")
        .def("to_base1", &Derived::to_base1)
        .def("say", &Derived::say)
        .def("say2", &Derived::say2)
        .def("getd", getd);

    // typedef void (SuperBase::*mptr)();

    // Derived *obj = &d;
    // mptr ptr = &SuperBase::say3;
    // //cout << (void *)obj << " " << zlua::type_name<decltype(ptr)>() << endl;

    // zlua::userdata::object_t<Derived> uod;
    // uod.ptr = obj;

    // zlua::userdata::object_t<SuperBase> *uob = (zlua::userdata::object_t<SuperBase> *)&uod;
    // auto o = (SuperBase *)((char *)uob->ptr + zlua::calc_base_offset<Derived, SuperBase>());
    // (o->*ptr)();

    engine.load_file("./test.lua");

    return 0;
}
