#ifndef __CONVERSIONS_HPP
#define __CONVERSIONS_HPP

template<class condition, typename R = void >
using EnableIf = typename std::enable_if<condition::value, R>::type;

template<class T>
EnableIf<std::is_floating_point<T>,
T> get_LuaValue(lua_State* L){
    T val = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return val;
}

template<class T>
EnableIf<std::is_integral<T>,
void> push_LuaValue(lua_State* L, T val){
    lua_pushinteger(L, val);
}


template<class T>
EnableIf<std::is_integral<T>,
T> get_LuaValue(lua_State* L){
    T val = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return val;
}

template<class T>
EnableIf<std::is_floating_point<T>,
void> push_LuaValue(lua_State* L, T val){
    lua_pushnumber(L, val);
}


template<class T>
EnableIf<std::is_class<T>,
T> get_LuaValue(lua_State* L){
    T* val = static_cast<T*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return *val;
}

//I don't know how to return an object by value
template<class T>
EnableIf<std::is_class<T>,
void> push_LuaValue(lua_State* L, T* val){
    lua_pushuserdata(L, val);
}


template<class T>
EnableIf<std::is_pointer<T>,
T> get_LuaValue(lua_State* L){
    T val = static_cast<T>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return val;
}

template<class T>
EnableIf<std::is_pointer<T>,
void> push_LuaValue(lua_State* L, T val){
    lua_pushuserdata(L, val);
}

#endif
