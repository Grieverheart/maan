#ifndef __MODULE_CREATOR_HPP
#define __MODULE_CREATOR_HPP

//TODO: This should be a class. Instead of class_ -> end_class, we will
//use the comma operator. Example usage:
//maan::module_(L, "glm")
//    .class_<glm::vec3>("vec3")
//        .def_constructor<float, float, float>(),
//    .function_<double, const glm::vec3&>("norm", norm);

void begin_module(lua_State* L, const char* name = nullptr){
    if(name){
        lua_newtable(L);
        lua_pushvalue(L, -1);
        if(lua_istable(L, -3)){
            lua_setfield(L, -3, name);
        }
        else{
            lua_setglobal(L, name);
        }
    }
    else{
        lua_pushglobaltable(L);
    }
}

void end_module(lua_State* L){
    lua_pop(L, 1);
}

#endif
