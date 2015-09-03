
local point = Point(1.0, 2.0)

local vec = vec3(1.2, -0.4, 0.6)

local foo = Foo(vec)
foo.foo_.x = 3

print(pointNorm(point))
print(point.x, point.y)
print(vec.x, vec.y, vec.z)
print(vec)
print(foo.foo_)
print(foo.foo_.x, foo.foo_.y, foo.foo_.z)

