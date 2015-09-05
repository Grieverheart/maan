#ifndef __OPERATOR_NEW_HPP
#define __OPERATOR_NEW_HPP

#include "class_info.hpp"

void* operator new(std::size_t size, lua_State* L){
    return lua_newuserdata(L, size);
}

template<class T, typename ...arg_types>
T* create_LuaObject(lua_State* L, arg_types ...args){
    T* ret = new(L) T(args...);
    lua_rawgetp(L, LUA_REGISTRYINDEX, ClassInfo<T>::get_metatable_key());
    if(!lua_isnil(L, -1)) lua_setmetatable(L, -2);
    else lua_pop(L, 1);
    return ret;
}

#endif
