#ifndef __FUNCTION__HPP
#define __FUNCTION__HPP

#include <functional>

#include "detail/lift.hpp"
#include "detail/__gc.hpp"
#include "create.hpp"
#include "convert.hpp"

namespace maan{
    namespace detail{
        template<typename...ArgsT>
        std::tuple<ArgsT...> get_args(lua_State* L){
            return std::forward_as_tuple(get_LuaValue<ArgsT>(L)...);
        }

        struct OverloadableFunctor{
            virtual int call(lua_State* L);
            virtual OverloadableFunctor* get_next(void);
        };

        static int call_overloadable_functor(lua_State* L){
            auto func = static_cast<OverloadableFunctor*>(lua_touserdata(L, lua_upvalueindex(1)));
            return func->call(L);
        }

        template<class T>
        struct Functor;

        //TODO: Perhaps we could somehow add a next pointer to chain overloads.
        template<class R, typename...ArgsT>
        struct Functor<R(ArgsT...)>: OverloadableFunctor{
            using F = std::function<R(ArgsT...)>;
            Functor(F func):
                func_(func), next_(nullptr)
            {}

            int call(lua_State* L){
                push_LuaValue(L, lift(func_, get_args<ArgsT...>(L)));
                return 1;
            }

            OverloadableFunctor* get_next(void){
                return next_;
            }

            F func_;
            OverloadableFunctor* next_;
        };

        template<class T>
        Functor<T>* create_LuaFunction(lua_State* L, std::function<T> func){
            typedef detail::Functor<T> F;
            F* luaFunc = create_LuaObject<F>(L, func);  //..., userdata
            lua_newtable(L);                            //..., userdata, table
            lua_pushvalue(L, -1);                       //..., userdata, table, table
            lua_setmetatable(L, -3);                    //..., userdata, table

            lua_pushstring(L, "__gc");                  //..., userdata, table, "__gc"
            lua_pushcfunction(L, __gc<F>);              //..., userdata, table, "__gc", __gc<T>
            lua_rawset(L, -3);                          //..., userdata, table
            lua_pop(L, 1);                              //..., userdata

            return luaFunc;
        }
    }

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, R (&func)(ArgsT...)){
        detail::create_LuaFunction<T>(L, func);
        lua_pushcclosure(L, detail::call_overloadable_functor, 1);
        lua_setglobal(L, name);
    }

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, std::function<R(ArgsT...)> func){
        detail::create_LuaFunction<T>(L, func);
        lua_pushcclosure(L, detail::call_overloadable_functor, 1);
        lua_setglobal(L, name);
    }
}

#endif
