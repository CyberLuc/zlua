local derived = Derived.getd()
print("\nderived:say2()")
derived:say2()

do return end

print("derived:say3()")
derived:say3()

print("\nderived:say()")
derived:say()

print("\nderived:to_base1():say()")
derived:to_base1():say()

print("\nderived:say2()")
derived:say2()

print("\nderived:to_base1():say2()")
derived:to_base1():say2()

local b = derived
collectgarbage()
print("------")
derived = nil
