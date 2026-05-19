#include "../../../include/vm/api/math.hpp"
#include <lua.hpp>
#include <cstring>
#include <new>

#include "../../../include/math/vector3.hpp"
#include "../../../include/math/vector4.hpp"
#include "../../../include/math/quaternion.hpp"
#include "../../../include/math/matrix4.hpp"
#include "../../../include/math/transform.hpp"

template <typename T>
T* push(lua_State* L, const char* metatable_name, const T& value) {
    void* userdata = lua_newuserdata(L, sizeof(T));
    new (userdata) T(value);
    luaL_getmetatable(L, metatable_name);
    lua_setmetatable(L, -2);
    return static_cast<T*>(userdata);
}

namespace vector3 {
    static int create(lua_State* L) {
        const auto x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
        const auto y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
        const auto z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
        push<Vector3>(L, "vector3", Vector3(x, y, z));
        return 1;
    }

    static int get(lua_State* L) {
        const auto* vector = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "x") == 0) {
            lua_pushnumber(L, vector->x);
        } else if (strcmp(key, "y") == 0) {
            lua_pushnumber(L, vector->y);
        } else if (strcmp(key, "z") == 0) {
            lua_pushnumber(L, vector->z);
        } else {
            lua_getmetatable(L, 1);
            lua_pushstring(L, key);
            lua_rawget(L, -2);
        }
        return 1;
    }

    static int set(lua_State* L) {
        auto* vector = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        const char* key = luaL_checkstring(L, 2);
        const auto value = static_cast<float>(luaL_checknumber(L, 3));

        if (strcmp(key, "x") == 0) {
            vector->x = value;
        } else if (strcmp(key, "y") == 0) {
            vector->y = value;
        } else if (strcmp(key, "z") == 0) {
            vector->z = value;
        }
        return 0;
    }

    static int add(lua_State* L) {
        const auto* alpha = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        const auto* beta = static_cast<Vector3*>(luaL_checkudata(L, 2, "vector3"));
        push<Vector3>(L, "vector3", *alpha + *beta);
        return 1;
    }

    static int subtract(lua_State* L) {
        const auto* alpha = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        const auto* beta = static_cast<Vector3*>(luaL_checkudata(L, 2, "vector3"));
        push<Vector3>(L, "vector3", *alpha - *beta);
        return 1;
    }

    static int multiply(lua_State* L) {
        const auto* alpha = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        if (lua_isnumber(L, 2)) {
            push<Vector3>(L, "vector3", *alpha * static_cast<float>(lua_tonumber(L, 2)));
        } else {
            const auto* beta = static_cast<Vector3*>(luaL_checkudata(L, 2, "vector3"));
            push<Vector3>(L, "vector3", *alpha * *beta);
        }
        return 1;
    }

    static int to_string(lua_State* L) {
        const auto* vector = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        lua_pushfstring(L, "Vector3(%f, %f, %f)", static_cast<double>(vector->x), static_cast<double>(vector->y), static_cast<double>(vector->z));
        return 1;
    }

    static int length(lua_State* L) {
        const auto* vector = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        lua_pushnumber(L, static_cast<float>(vector->length()));
        return 1;
    }

    static int normalized(lua_State* L) {
        const auto* vector = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        push<Vector3>(L, "vector3", vector->normalized());
        return 1;
    }

    static int cross(lua_State* L) {
        const auto* alpha = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
        const auto* beta = static_cast<Vector3*>(luaL_checkudata(L, 2, "vector3"));
        push<Vector3>(L, "vector3", alpha->cross(*beta));
        return 1;
    }

    static const luaL_Reg methods[] = {
        {"length", length},
        {"normalized", normalized},
        {"cross", cross},
        {"__add", add},
        {"__sub", subtract},
        {"__mul", multiply},
        {"__index", get},
        {"__newindex", set},
        {"__tostring", to_string},
        {nullptr, nullptr}
    };
}

namespace quaternion {
    static int create(lua_State* L) {
        const auto x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
        const auto y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
        const auto z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
        const auto w = static_cast<float>(luaL_optnumber(L, 4, 1.0));
        push<Quaternion>(L, "quaternion", Quaternion(x, y, z, w));
        return 1;
    }

    static int get(lua_State* L) {
        const auto* quaternion = static_cast<Quaternion*>(luaL_checkudata(L, 1, "quaternion"));
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "x") == 0) {
            lua_pushnumber(L, quaternion->x);
        } else if (strcmp(key, "y") == 0) {
            lua_pushnumber(L, quaternion->y);
        } else if (strcmp(key, "z") == 0) {
            lua_pushnumber(L, quaternion->z);
        } else if (strcmp(key, "w") == 0) {
            lua_pushnumber(L, quaternion->w);
        } else {
            lua_getmetatable(L, 1);
            lua_pushstring(L, key);
            lua_rawget(L, -2);
        }
        return 1;
    }

    static int set(lua_State* L) {
        auto* quaternion = static_cast<Quaternion*>(luaL_checkudata(L, 1, "quaternion"));
        const char* key = luaL_checkstring(L, 2);
        const auto value = static_cast<float>(luaL_checknumber(L, 3));

        if (strcmp(key, "x") == 0) {
            quaternion->x = value;
        } else if (strcmp(key, "y") == 0) {
            quaternion->y = value;
        } else if (strcmp(key, "z") == 0) {
            quaternion->z = value;
        } else if (strcmp(key, "w") == 0) {
            quaternion->w = value;
        }
        return 0;
    }

    static int multiply(lua_State* L) {
        const auto* alpha = static_cast<Quaternion*>(luaL_checkudata(L, 1, "quaternion"));
        if (luaL_testudata(L, 2, "vector3")) {
            const auto* beta = static_cast<Vector3*>(luaL_checkudata(L, 2, "vector3"));
            push<Vector3>(L, "vector3", *alpha * *beta);
        } else {
            const auto* beta = static_cast<Quaternion*>(luaL_checkudata(L, 2, "quaternion"));
            push<Quaternion>(L, "quaternion", *alpha * *beta);
        }
        return 1;
    }

    static int euler(lua_State* L) {
        const auto* quaternion = static_cast<Quaternion*>(luaL_checkudata(L, 1, "quaternion"));
        push<Vector3>(L, "vector3", quaternion->euler());
        return 1;
    }

    static const luaL_Reg methods[] = {
        {"euler", euler},
        {"__mul", multiply},
        {"__index", get},
        {"__newindex", set},
        {nullptr, nullptr}
    };
}

namespace vector4 {
    static int create(lua_State* L) {
        const auto x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
        const auto y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
        const auto z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
        const auto w = static_cast<float>(luaL_optnumber(L, 4, 1.0));
        push<Vector4>(L, "vector4", Vector4(x, y, z, w));
        return 1;
    }

    static int get(lua_State* L) {
        const auto* vector = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));

        if (const char* key = luaL_checkstring(L, 2); strcmp(key, "x") == 0) {
            lua_pushnumber(L, vector->x);
        } else if (strcmp(key, "y") == 0) {
            lua_pushnumber(L, vector->y);
        } else if (strcmp(key, "z") == 0) {
            lua_pushnumber(L, vector->z);
        } else if (strcmp(key, "w") == 0) {
            lua_pushnumber(L, vector->w);
        } else {
            lua_getmetatable(L, 1);
            lua_pushstring(L, key);
            lua_rawget(L, -2);
        }
        return 1;
    }

    static int set(lua_State* L) {
        auto* vector = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        const char* key = luaL_checkstring(L, 2);
        const auto value = static_cast<float>(luaL_checknumber(L, 3));

        if (strcmp(key, "x") == 0) {
            vector->x = value;
        } else if (strcmp(key, "y") == 0) {
            vector->y = value;
        } else if (strcmp(key, "z") == 0) {
            vector->z = value;
        } else if (strcmp(key, "w") == 0) {
            vector->w = value;
        }
        return 0;
    }

    static int add(lua_State* L) {
        const auto* alpha = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        const auto* beta = static_cast<Vector4*>(luaL_checkudata(L, 2, "vector4"));
        push<Vector4>(L, "vector4", *alpha + *beta);
        return 1;
    }

    static int subtract(lua_State* L) {
        const auto* alpha = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        const auto* beta = static_cast<Vector4*>(luaL_checkudata(L, 2, "vector4"));
        push<Vector4>(L, "vector4", *alpha - *beta);
        return 1;
    }

    static int multiply(lua_State* L) {
        const auto* alpha = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        if (lua_isnumber(L, 2)) {
            push<Vector4>(L, "vector4", *alpha * static_cast<float>(lua_tonumber(L, 2)));
        } else {
            const auto* beta = static_cast<Vector4*>(luaL_checkudata(L, 2, "vector4"));
            push<Vector4>(L, "vector4", *alpha * *beta);
        }
        return 1;
    }

    static int length(lua_State* L) {
        const auto* vector = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        lua_pushnumber(L, static_cast<float>(vector->length()));
        return 1;
    }

    static int normalized(lua_State* L) {
        const auto* vector = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        push<Vector4>(L, "vector4", vector->normalized());
        return 1;
    }

    static int to_string(lua_State* L) {
        const auto* vector = static_cast<Vector4*>(luaL_checkudata(L, 1, "vector4"));
        lua_pushfstring(L, "Vector4(%f, %f, %f, %f)", static_cast<double>(vector->x), static_cast<double>(vector->y), static_cast<double>(vector->z), static_cast<double>(vector->w));
        return 1;
    }

    static const luaL_Reg methods[] = {
        {"length", length},
        {"normalized", normalized},
        {"__add", add},
        {"__sub", subtract},
        {"__mul", multiply},
        {"__index", get},
        {"__newindex", set},
        {"__tostring", to_string},
        {nullptr, nullptr}
    };
}

namespace matrix4 {
    static int create(lua_State* L) {
        if (lua_isnumber(L, 1)) {
            const auto scalar = static_cast<float>(lua_tonumber(L, 1));
            push<Matrix4>(L, "matrix4", Matrix4(scalar));
        } else {
            push<Matrix4>(L, "matrix4", Matrix4::identity());
        }
        return 1;
    }

    static int get(lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        lua_getmetatable(L, 1);
        lua_pushstring(L, key);
        lua_rawget(L, -2);
        return 1;
    }

    static int add(lua_State* L) {
        const auto* alpha = static_cast<Matrix4*>(luaL_checkudata(L, 1, "matrix4"));
        const auto* beta = static_cast<Matrix4*>(luaL_checkudata(L, 2, "matrix4"));
        push<Matrix4>(L, "matrix4", *alpha + *beta);
        return 1;
    }

    static int subtract(lua_State* L) {
        const auto* alpha = static_cast<Matrix4*>(luaL_checkudata(L, 1, "matrix4"));
        const auto* beta = static_cast<Matrix4*>(luaL_checkudata(L, 2, "matrix4"));
        push<Matrix4>(L, "matrix4", *alpha - *beta);
        return 1;
    }

    static int multiply(lua_State* L) {
        const auto* alpha = static_cast<Matrix4*>(luaL_checkudata(L, 1, "matrix4"));
        if (lua_isnumber(L, 2)) {
            push<Matrix4>(L, "matrix4", *alpha * static_cast<float>(lua_tonumber(L, 2)));
        } else if (luaL_testudata(L, 2, "vector3")) {
            const auto* beta = static_cast<Vector3*>(luaL_checkudata(L, 2, "vector3"));
            push<Vector3>(L, "vector3", *alpha * *beta);
        } else {
            const auto* beta = static_cast<Matrix4*>(luaL_checkudata(L, 2, "matrix4"));
            push<Matrix4>(L, "matrix4", *alpha * *beta);
        }
        return 1;
    }

    static int transposed(lua_State* L) {
        const auto* matrix = static_cast<Matrix4*>(luaL_checkudata(L, 1, "matrix4"));
        push<Matrix4>(L, "matrix4", matrix->transposed());
        return 1;
    }

    static int inverted(lua_State* L) {
        const auto* matrix = static_cast<Matrix4*>(luaL_checkudata(L, 1, "matrix4"));
        push<Matrix4>(L, "matrix4", matrix->inverted());
        return 1;
    }

    static int to_string(lua_State* L) {
        auto* matrix = static_cast<Matrix4*>(luaL_checkudata(L, 1, "matrix4"));
        lua_pushfstring(L, "Matrix4([[ %f, %f, %f, %f ], ...])", static_cast<double>((*matrix)(0,0)), static_cast<double>((*matrix)(0,1)), static_cast<double>((*matrix)(0,2)), static_cast<double>((*matrix)(0,3)));
        return 1;
    }

    static const luaL_Reg methods[] = {
        {"transposed", transposed},
        {"inverted", inverted},
        {"__add", add},
        {"__sub", subtract},
        {"__mul", multiply},
        {"__index", get},
        {"__tostring", to_string},
        {nullptr, nullptr}
    };
}

namespace transform {
    static int create(lua_State* L) {
        if (luaL_testudata(L, 1, "vector3") && luaL_testudata(L, 2, "quaternion") && luaL_testudata(L, 3, "vector3")) {
            const auto* translation = static_cast<Vector3*>(luaL_checkudata(L, 1, "vector3"));
            const auto* rotation = static_cast<Quaternion*>(luaL_checkudata(L, 2, "quaternion"));
            const auto* scale = static_cast<Vector3*>(luaL_checkudata(L, 3, "vector3"));
            push<Transform>(L, "transform", Transform(*translation, *rotation, *scale));
        } else {
            push<Transform>(L, "transform", Transform());
        }
        return 1;
    }

    static int get(lua_State* L) {
        const auto* transform = static_cast<Transform*>(luaL_checkudata(L, 1, "transform"));
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "translation") == 0) {
            push<Vector3>(L, "vector3", transform->translation);
        } else if (strcmp(key, "rotation") == 0) {
            push<Quaternion>(L, "quaternion", transform->rotation);
        } else if (strcmp(key, "scale") == 0) {
            push<Vector3>(L, "vector3", transform->scale);
        } else {
            lua_getmetatable(L, 1);
            lua_pushstring(L, key);
            lua_rawget(L, -2);
        }
        return 1;
    }

    static int set(lua_State* L) {
        auto* transform = static_cast<Transform*>(luaL_checkudata(L, 1, "transform"));

        if (const char* key = luaL_checkstring(L, 2); strcmp(key, "translation") == 0) {
            transform->translation = *static_cast<Vector3*>(luaL_checkudata(L, 3, "vector3"));
        } else if (strcmp(key, "rotation") == 0) {
            transform->rotation = *static_cast<Quaternion*>(luaL_checkudata(L, 3, "quaternion"));
        } else if (strcmp(key, "scale") == 0) {
            transform->scale = *static_cast<Vector3*>(luaL_checkudata(L, 3, "vector3"));
        }
        return 0;
    }

    static int multiply(lua_State* L) {
        const auto* alpha = static_cast<Transform*>(luaL_checkudata(L, 1, "transform"));
        const auto* beta = static_cast<Transform*>(luaL_checkudata(L, 2, "transform"));
        push<Transform>(L, "transform", *alpha * *beta);
        return 1;
    }

    static int matrix(lua_State* L) {
        const auto* transform = static_cast<Transform*>(luaL_checkudata(L, 1, "transform"));
        push<Matrix4>(L, "matrix4", transform->matrix());
        return 1;
    }

    static int to_string(lua_State* L) {
        lua_pushstring(L, "Transform()");
        return 1;
    }

    static const luaL_Reg methods[] = {
        {"matrix", matrix},
        {"__mul", multiply},
        {"__index", get},
        {"__newindex", set},
        {"__tostring", to_string},
        {nullptr, nullptr}
    };
}

extern "C" int luaopen_math(lua_State* L) {
    luaL_newmetatable(L, "vector3");
    luaL_setfuncs(L, vector3::methods, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, "quaternion");
    luaL_setfuncs(L, quaternion::methods, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, "vector4");
    luaL_setfuncs(L, vector4::methods, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, "matrix4");
    luaL_setfuncs(L, matrix4::methods, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, "transform");
    luaL_setfuncs(L, transform::methods, 0);
    lua_pop(L, 1);

    lua_newtable(L);

    lua_pushcfunction(L, vector3::create);
    lua_setfield(L, -2, "vector3");

    lua_pushcfunction(L, quaternion::create);
    lua_setfield(L, -2, "quaternion");

    lua_pushcfunction(L, vector4::create);
    lua_setfield(L, -2, "vector4");

    lua_pushcfunction(L, matrix4::create);
    lua_setfield(L, -2, "matrix4");

    lua_pushcfunction(L, transform::create);
    lua_setfield(L, -2, "transform");

    return 1;
}