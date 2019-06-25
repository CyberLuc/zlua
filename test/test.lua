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
