#ifndef __OPERATOR_NEW_HPP
#define __OPERATOR_NEW_HPP

void* operator new(std::size_t size, lua_State* L){
    return lua_newuserdata(L, size);
}

template<class T, typename ...arg_types>
T* create_LuaObject(lua_State* L, arg_types ...args){
    return new(L) T(args...);
}

#endif
