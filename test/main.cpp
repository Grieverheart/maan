#include <cstdio>
#include <cmath>

extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <glm/glm.hpp>
#include "maan.hpp"

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
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    maan::module_(L)
        .class_<Point>("Point")
            .def_constructor<double, double>()
            .def_constructor<double>()
            .def_operator<maan::add, Point>()
            .def_readwrite("x", &Point::x)
            .def_readwrite("y", &Point::y)
            .def("print", &Point::print)
            .def("norm", &Point::norm)
            .endef()
        .class_<Foo>("Foo")
            .def_constructor<const glm::vec3&>()
            .def_readwrite("foo_", &Foo::foo_)
            .endef()
        .function_("norm", (double (*)(const glm::vec3& p))norm)
        .function_("norm", (double (*)(const Point& p))norm)
        .function_("print_something", print_something)
        .namespace_("glm")
            .class_<glm::vec3>("vec3")
                .def_constructor<float, float, float>()
                .def_constructor<float>()
                .def_constructor<glm::vec3>()
                .def_operator<maan::add, glm::vec3>()
                .def_operator<maan::add, float>()
                .def_operator<maan::sub, glm::vec3>()
                .def_operator<maan::sub, float>()
                .def_operator<maan::mul, float>()
                .def_operator<maan::eq, glm::vec3>()
                .def_readwrite("x", &glm::vec3::x)
                .def_readwrite("y", &glm::vec3::y)
                .def_readwrite("z", &glm::vec3::z)
                .endef();
        
    if(luaL_dofile(L, "test.lua")){
        printf("There was an error.\n %s\n", lua_tostring(L, -1));
    }
    
    lua_close(L);
    
    return 0;
}
