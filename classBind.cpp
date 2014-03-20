#include <cstdio>
#include <new>
#include <string>
#include <tuple>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int ref = 0;

int lua_ClassProperty(lua_State* L){
    return 0;
}

void* operator new(std::size_t size, lua_State* L){
    return lua_newuserdata(L, size);
}

template<class T>
struct classRep;

#define startClassDefinition(T)\
    template<>\
    struct classRep<T>{\
        typedef T type_

#define endClassDefinition()\
    };
    
#define defGetter(var) \
    static int get_ ## var(lua_State* L){\
        type_* object = static_cast<type_*>(lua_touserdata(L, 1));\
        lua_pushnumber(L, object->var);\
        return 1;\
    }
    
#define defSetter(var) \
    static int set_ ## var(lua_State* L){\
        type_* object = static_cast<type_*>(lua_touserdata(L, 1));\
        object->var = luaL_checknumber(L, 2);\
        return 1;\
    }

template<class T, typename ...arg_types>
void create_LuaObject(lua_State* L, arg_types ...args){
    new(L) T(args...);
}

/*--------------------------tuple argument unpacking--------------------------*/

template<int ...> struct sequence{};
template<int N, int ...S> struct generator: generator<N - 1, N - 1, S...>{};
template<int ...S> struct generator<0, S...>{typedef sequence<S...> type;};

template<typename Sequence>
struct apply_tuple_impl;

template<template<int...> class Sequence, int... Indices>
struct apply_tuple_impl<Sequence<Indices...>>{
	template<class F, typename... ArgsT>
	static auto apply_tuple(F&& func, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(ArgsT...)>::type {
		return func(std::forward<ArgsT>(std::get<Indices>(args))...);
	}
	template<class F, class U, typename... ArgsT>
	static auto apply_tuple(F&& func, U&& arg, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(U, ArgsT...)>::type {
		return func(std::forward<U>(arg), std::forward<ArgsT>(std::get<Indices>(args))...);
	}
};

template<
	class F, typename... ArgsT,
	typename Sequence = typename generator<sizeof...(ArgsT)>::type
>
auto apply_tuple(F&& func, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(ArgsT...)>::type {
	return apply_tuple_impl<Sequence>::apply_tuple(std::forward<F>(func), std::forward<std::tuple<ArgsT...>>(args));
}

template<
	class F, class U, typename... ArgsT,
	typename Sequence = typename generator<sizeof...(ArgsT)>::type
>
auto apply_tuple(F&& func, U&& arg, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(U, ArgsT...)>::type {
	return apply_tuple_impl<Sequence>::apply_tuple(std::forward<F>(func), std::forward<U>(arg), std::forward<std::tuple<ArgsT...>>(args));
}
/*----------------------------------------------------------------------------*/
    
class Point{
public:
    Point(double x, double y):
        x(x), y(y)
    {}
    
    double x, y;
};

startClassDefinition(Point);
defGetter(x);
defSetter(x);
defGetter(y);
defSetter(y);
endClassDefinition();
    
template<typename ...ArgsT>
int lua_Point(lua_State* L){
    std::tuple<ArgsT...> args;
    std::get<0>(args) = luaL_checknumber(L, 1);
    std::get<1>(args) = luaL_checknumber(L, 2);
    apply_tuple(create_LuaObject<Point, ArgsT...>, L, std::move(args));
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    lua_setmetatable(L, -2);
    return 1;
}

int lua_Point__index(lua_State* L){
//table, index
    printf("%s, %d\n", lua_tostring(L, 2), lua_gettop(L));
	lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if(lua_isnil(L, -1)) return 1;
    
    if(lua_tocfunction(L, -1) == lua_ClassProperty){
        lua_getupvalue(L, -1, 1);
        lua_pushvalue(L, 1);
        lua_call(L, 1, 1);
    }
    return 1;
}

int lua_Point__newindex(lua_State* L){
    printf("%s, %d\n", lua_tostring(L, 2), lua_gettop(L));
	lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if(lua_isnil(L, -1)){
        lua_pop(L, 1);
        lua_insert(L, 2);
        lua_rawset(L, -3);
        return 0;
    }
    
    if(lua_tocfunction(L, -1) == lua_ClassProperty){
        lua_getupvalue(L, -1, 2);
        lua_remove(L, 2);
        lua_insert(L, 1);
        lua_pop(L, 1);
        lua_call(L, lua_gettop(L) - 1, 0);
        return 0;
    }
    
    lua_pop(L, 1);
    lua_insert(L, 2);
    lua_rawset(L, 2);
    return 0;
    
}

void register_lua_Point(lua_State* L){
    
    lua_newtable(L);
	lua_pushvalue(L, 1);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_Point__index);
    lua_rawset(L, -3);
    
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_Point__newindex);
    lua_rawset(L, -3);
    
    lua_pushstring(L, "x");
    lua_pushcfunction(L, classRep<Point>::get_x);
    lua_pushcfunction(L, classRep<Point>::set_x);
    lua_pushcclosure(L, lua_ClassProperty, 2);
    lua_rawset(L, -3);
    
    lua_pushstring(L, "y");
    lua_pushcfunction(L, classRep<Point>::get_y);
    lua_pushcfunction(L, classRep<Point>::set_y);
    lua_pushcclosure(L, lua_ClassProperty, 2);
    lua_rawset(L, -3);
    
    lua_register(L, "Point", (lua_Point<double, double>));
}


int main(int argc, char* argv[]){
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    register_lua_Point(L);

	// class_<Point>(L, "Point")
		// .def_constructor<double, double>()
		// .def_readwrite<Point, double>("x", &Point::x)
		// .def_readwrite("y", &Point::y)
	// .register_();
		
    luaL_dofile(L, "test.lua");
    
    lua_close(L);
    
    return 0;
}
