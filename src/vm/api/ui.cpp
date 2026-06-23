#include "../include/vm/api/ui.hpp"
#include "../include/core/types/components/rect.hpp"
#include "../include/core/types/components/bounds.hpp"
#include "../include/core/types/components/flow.hpp"
#include "../include/core/types/components/visibility.hpp"
#include "../include/ui/layout.hpp"
#include <sol/sol.hpp>

using namespace core;
using namespace core::components;

void ui(lua_State* state) {
    sol::state_view lua(state);

    lua.new_usertype<Rect>("Rect",
        sol::constructors<Rect()>(),
        "position", &Rect::position,
        "rotation", &Rect::rotation,
        "scale", &Rect::scale,
        "anchor", &Rect::anchor
    );

    lua.new_usertype<Bounds>("Bounds",
        sol::constructors<Bounds()>(),
        "size", &Bounds::size,
        "margin", &Bounds::margin,
        "padding", &Bounds::padding
    );

    lua.new_usertype<Flow>("Flow",
        sol::constructors<Flow()>(),
        "axis", &Flow::axis,
        "distribution", &Flow::distribution,
        "alignment", &Flow::alignment,
        "gap", &Flow::gap,
        "wrap", &Flow::wrap
    );

    lua.new_usertype<Visibility>("Visibility",
        sol::constructors<Visibility()>(),
        "active", &Visibility::active,
        "intercept", &Visibility::intercept
    );

    sol::table interface = lua.create_named_table("ui");
    interface["layout"] = &ui::layout;
}