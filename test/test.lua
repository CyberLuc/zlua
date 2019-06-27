local v = vector.int.new()
print("v:size() = " .. v:size())
print("v:push_back(1)")
v:push_back(1)
print("v:push_back(2)")
v:push_back(2)
print("v:size() = " .. v:size())
print("v:at(0) = " .. v:at(0))
print("v:at(1) = " .. v:at(1))
--print("v:at(1) = " .. v:atc(1))

for i = 0, v:size() - 1 do
    print("  v:at(" .. i .. ") = " .. v:at(i))
end

do return end

local derived = Derived.new()

print("derived:say3()")
derived:say3()

print("\nderived:say()")
derived:say()

print("\nderived:to_base1():say()")
derived:to_base1():say()

print("\nderived:say2()")
derived:say2(nil)

print("\nderived:to_base1():say2()")
derived:to_base1():say2()

local b = derived
collectgarbage()
print("------")
derived = nil

