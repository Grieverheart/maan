#ifndef __CLASS_CREATOR_HPP
#define __CLASS_CREATOR_HPP

#include "lift.hpp"
#include "operator_new.hpp"
#include "function_.hpp"
#include "class_info.hpp"

int lua_ClassProperty(lua_State* L){
    return 0;
}

template<class T>
class class_{
public:
    using type_ = T;

    class_(lua_State* L, const char* name):
        name_(name), L_(L)
    {
        lua_newtable(L_);
        lua_pushvalue(L_, -1);
        lua_rawsetp(L_, LUA_REGISTRYINDEX, ClassInfo<T>::get_metatable_key());

        lua_pushstring(L_, "__index");
        lua_pushcfunction(L_, __index);
        lua_rawset(L_, -3);

        lua_pushstring(L_, "__newindex");
        lua_pushcfunction(L_, __newindex);
        lua_rawset(L_, -3);

        lua_pushstring(L_, "__name");
        lua_pushstring(L_, name_);
        lua_rawset(L_, -3);
    }

    template<typename ...ArgsT>
    class_& def_constructor(void){
        lua_pushcfunction(L_, (constructor<ArgsT...>));
        lua_setglobal(L_, name_);
        return *this;
    }

    template<class M>
    class_& def_readwrite(const char* name, M type_::*var_ptr){
        lua_pushstring(L_, name);

        auto getter = std::function<M(type_*)>([var_ptr](type_* object) -> M {
            return object->*var_ptr;
        });

        auto setter = std::function<void(type_*, const M&)>([var_ptr](type_* object, const M& val){
            object->*var_ptr = val;
        });

        create_LuaFunction(L_, getter);
        lua_pushcclosure(L_, get_var<M>, 1);

        create_LuaFunction(L_, setter);
        lua_pushcclosure(L_, set_var<M>, 1);

        lua_pushcclosure(L_, lua_ClassProperty, 2);
        lua_rawset(L_, -3);

        return *this;
    }

    template<class M>
    class_& def_readonly(const char* name, M type_::*var_ptr){
        lua_pushstring(L_, name);

        auto getter = new std::function<M(type_*)>([var_ptr](type_* object) -> M {
            return object->*var_ptr;
        });

        lua_pushlightuserdata(L_, static_cast<void*>(getter));
        lua_pushcclosure(L_, get_var<M>, 1);

        lua_pushcclosure(L_, lua_ClassProperty, 1);
        lua_rawset(L_, -3);
        return *this;
    }

    template<typename ...ArgsT>
    static int constructor(lua_State* L){
        lift(create_LuaObject<type_, ArgsT...>, L, get_args<ArgsT...>(L));
        lua_rawgetp(L, LUA_REGISTRYINDEX, ClassInfo<type_>::get_metatable_key());
        lua_setmetatable(L, -2);
        return 1;
    }

    template<class M>
    static int get_var(lua_State* L){
        type_* object = static_cast<type_*>(lua_touserdata(L, 1));
        auto getter = *static_cast<std::function<M(type_*)>*>(lua_touserdata(L, lua_upvalueindex(1))); //ERROR: Not Working
        push_LuaValue(L, getter(object));
        return 1;
    }

    template<class M>
    static int set_var(lua_State* L){
        type_* object = static_cast<type_*>(lua_touserdata(L, 1));
        auto setter = *static_cast<std::function<void(type_*, M)>*>(lua_touserdata(L, lua_upvalueindex(1))); //ERROR: Not Working
        lua_pop(L, 1);
        setter(object, get_LuaValue<M>(L));
        return 0;
    }

    static int __index(lua_State* L){                               //userdata, index
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

    static int __newindex(lua_State* L){                        //userdata, index, value
        lua_getmetatable(L, 1);                                 //userdata, index, value, metatable
        lua_pushvalue(L, 2);                                    //userdata, index, value, metatable, index
        lua_rawget(L, -2);                                      //userdata, index, value, metatable, metatable[index]
        if(lua_isnil(L, -1)){                                   //
            lua_pop(L, 1);                                      //userdata, index, value, metatable
            lua_insert(L, 2);                                   //userdata, metatable, index, value
            lua_rawset(L, -3);                                  //userdata, metatable[index] = value
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
    lua_State* L_;
};

#endif
