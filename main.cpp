#include <cstdio>
#include <cmath>

extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "function_.hpp"
#include "class_.hpp"
#include <glm/glm.hpp>

class Point{
public:
    Point(double x, double y):
        x(x), y(y)
    {}

    double norm(void)const{
        return sqrt(x * x + y * y);
    }
    
    double x, y;
};

class Foo{
public:
    Foo(const glm::vec3& foo):
        foo_(foo)
    {}

    glm::vec3 foo_;
};

double pointNorm(Point p){
    return sqrt(p.x * p.x + p.y * p.y);
}

int main(int argc, char* argv[]){
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    class_<Point>(L, "Point")
        .def_constructor<double, double>()
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("norm", &Point::norm);

    class_<glm::vec3>(L, "vec3")
        .def_constructor<float, float, float>()
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::y);

    class_<Foo>(L, "Foo")
        .def_constructor<const glm::vec3&>()
        .def_readwrite("foo_", &Foo::foo_);

    function_(L, "pointNorm", pointNorm);
        
    if(luaL_dofile(L, "test.lua")){
        printf("There was an error.\n %s\n", lua_tostring(L, -1));
    }
    
    lua_close(L);
    
    return 0;
}
