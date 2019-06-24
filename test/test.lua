local role = Role.new("Name", 18)
print("role.name " .. role.name .. ", role.age " .. role.age)
role:print_something(1, 2, "3")
role:test_ptr(role)
