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
