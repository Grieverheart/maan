#include <cstdio>
#include <cmath>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "function_.hpp"
#include "class_.hpp"

class Point{
public:
    Point(double x, double y):
        x(x), y(y)
    {}
    
    double x, y;
};

double norm(double x, double y, double z){
    return sqrt(x * x + y * y + z * z);
}

double pointNorm(Point p){
    return sqrt(p.x * p.x + p.y * p.y);
}

int main(int argc, char* argv[]){
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    class_<Point>(L, "Point")
        .def_constructor<double, double>()
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y);

    function_(L, "norm", norm);
    function_(L, "pointNorm", pointNorm);
    lua_settop(L, 0);
        
    luaL_dofile(L, "test.lua");
    
    lua_close(L);
    
    return 0;
}
