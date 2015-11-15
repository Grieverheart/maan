
local point = Point(1.0, 2.0)
point:print()

local vec1 = glm.vec3(1.2, -0.4, 0.6)
local vec2 = glm.vec3(0.6, -0.4, 1.2)
local vecc = 0.5 * glm.vec3(vec1)

local foo = Foo(vec1)
foo.foo_.x = 3

print(point:norm())
print(point.x, point.y)
print(vecc.x, vecc.y, vecc.z)
print(vec1.x, vec1.y, vec1.z)
print(vec1)
print(foo.foo_)
print(foo.foo_.x, foo.foo_.y, foo.foo_.z)

local point2 = Point(2.0, 1.0)
point = point + point2
print(point.x, point.y)

local vec = vec1 + vec2
print(vec.x, vec.y, vec.z)
vec = vec1 + 1.0
print(vec.x, vec.y, vec.z)
vec = vec1 * 2.0
print(vec.x, vec.y, vec.z)
--print(vec1 == vec2)

print_something("nothing")
print(norm(point), point:norm())
print(norm(vec1), math.sqrt(vec1.x * vec1.x + vec1.y * vec1.y + vec1.z * vec1.z))
ppp.ppprint()
