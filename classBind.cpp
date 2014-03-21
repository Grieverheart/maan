#include <cstdio>
#include <new>
#include <string>
#include <tuple>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int lua_ClassProperty(lua_State* L){
    return 0;
}

void* operator new(std::size_t size, lua_State* L){
    return lua_newuserdata(L, size);
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

template<class T>
class class_{
public:
    using type_ = T;

    class_(lua_State* L, const char* name):
        name_(name), L_(L)
    {
        lua_newtable(L_);
        lua_pushvalue(L_, 1);
        metatable_ref_ = luaL_ref(L_, LUA_REGISTRYINDEX);

        lua_pushstring(L_, "__index");
        lua_pushcfunction(L_, __index);
        lua_rawset(L_, -3);
        
        lua_pushstring(L_, "__newindex");
        lua_pushcfunction(L_, __newindex);
        lua_rawset(L_, -3);
    }

    template<typename ...ArgsT>
    class_& def_constructor(void){
        lua_pushinteger(L_, metatable_ref_);
        lua_pushcclosure(L_, (constructor<ArgsT...>), 1);
        lua_setglobal(L_, name_);
        return *this;
    }

    template<class M>
    class_& def_readwrite(const char* name, M type_::*var_ptr){
        lua_pushstring(L_, name);

        ptrdiff_t member_offset = reinterpret_cast<ptrdiff_t>(&(((type_*)0)->*var_ptr));

        lua_pushinteger(L_, member_offset);
        lua_pushcclosure(L_, get_var<M>, 1);

        lua_pushinteger(L_, member_offset);
        lua_pushcclosure(L_, set_var<M>, 1);

        lua_pushcclosure(L_, lua_ClassProperty, 2);
        lua_rawset(L_, -3);
        return *this;
    }
    
    template<typename ...ArgsT>
    static int constructor(lua_State* L){
        int metatable_ref = lua_tointeger(L, lua_upvalueindex(1));
        std::tuple<ArgsT...> args;
        std::get<0>(args) = luaL_checknumber(L, 1);
        std::get<1>(args) = luaL_checknumber(L, 2);
        apply_tuple(create_LuaObject<type_, ArgsT...>, L, std::move(args));
        lua_rawgeti(L, LUA_REGISTRYINDEX, metatable_ref);
        lua_setmetatable(L, -2);
        return 1;
    }

    template<class M>
    static int get_var(lua_State* L){
        type_* object = static_cast<type_*>(lua_touserdata(L, 1));
        auto var_ptr = reinterpret_cast<M*>(object + lua_tointeger(L, lua_upvalueindex(1))); //ERROR: Not Working
        lua_pushnumber(L, *var_ptr);
        return 1;
    }

    template<class M>
    static int set_var(lua_State* L){
        type_* object = static_cast<type_*>(lua_touserdata(L, 1));
        auto var_ptr = reinterpret_cast<M*>(object + lua_tointeger(L, lua_upvalueindex(1)));
        *var_ptr = static_cast<M>(luaL_checknumber(L, 2));
        return 0;
    }

    static int __index(lua_State* L){                                      //userdata, index
        printf("%s, %d\n", lua_tostring(L, 2), lua_gettop(L));      //
        lua_getmetatable(L, 1);                                     //userdata, index, metatable
        lua_pushvalue(L, 2);                                        //userdata, index, metatable, index
        lua_rawget(L, -2);                                          //userdata, index, metatable[index]
        if(lua_isnil(L, -1)) return 1;                              //
                                                                    //
        if(lua_tocfunction(L, -1) == lua_ClassProperty){            //userdata, index, closure
            lua_getupvalue(L, -1, 1);                               //userdata, index, closure, func_1
            lua_pushvalue(L, 1);                                    //userdata, index, func, func_1, userdata
            lua_call(L, 1, 1);
        }
        return 1;
    }

    static int __newindex(lua_State* L){
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

private:
    const char* name_;
    int metatable_ref_;
    lua_State* L_;
};
    
class Point{
public:
    Point(double x, double y):
        x(x), y(y)
    {}
    
    double x, y;
};

int main(int argc, char* argv[]){
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    class_<Point>(L, "Point")
        .def_constructor<double, double>()
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y);

    lua_settop(L, 0);
        
    luaL_dofile(L, "test.lua");
    
    lua_close(L);
    
    return 0;
}
