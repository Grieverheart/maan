#include <cstdio>
#include <cmath>

extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <glm/glm.hpp>
#include "function_.hpp"
#include "class_.hpp"

class Point{
public:
    Point(double x, double y):
        x(x), y(y)
    {}

    Point(double x):
        x(x), y(x)
    {}

    Point& operator+(const Point& other){
        x += other.x;
        y += other.y;
        return *this;
    }

    double norm(void)const{
        return sqrt(x * x + y * y);
    }

    void print(void)const{
        printf("%f, %f\n", x, y);
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

double norm(const Point& p){
    return sqrt(p.x * p.x + p.y * p.y);
}

double norm(const glm::vec3& p){
    return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

void print_something(void){
    printf("Something!\n");
}

int main(int argc, char* argv[]){

    using namespace maan;
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    class_<Point>(L, "Point")
        .def_constructor<double, double>()
        .def_constructor<double>()
        .def_add()
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("print", &Point::print)
        .def("norm", &Point::norm);

    class_<glm::vec3>(L, "vec3")
        .def_constructor<float, float, float>()
        .def_constructor<float>()
        .def_constructor<glm::vec3>()
        .def_add()
        .def_sub()
        .def_eq()
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z);

    class_<Foo>(L, "Foo")
        .def_constructor<const glm::vec3&>()
        .def_readwrite("foo_", &Foo::foo_);

    function_<double, const Point&>(L, "norm", norm);
    function_<double, const glm::vec3&>(L, "norm", norm);
    function_(L, "print_something", print_something);
        
    if(luaL_dofile(L, "test.lua")){
        printf("There was an error.\n %s\n", lua_tostring(L, -1));
    }
    
    lua_close(L);
    
    return 0;
}
