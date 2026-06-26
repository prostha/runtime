#include "vm/api/ui.hpp"
#include "ui/widget.hpp"
#include "ui/defs/traits.hpp"
#include "ui/defs/geometry.hpp"
#include "ui/defs/layout.hpp"
#include "ui/defs/spacing.hpp"
#include "ui/defs/style.hpp"
#include <sol/sol.hpp>

void ui(lua_State* state) {
    sol::state_view lua(state);

    sol::table interface = lua.create_named_table("ui");

    auto display = interface["Display"].get_or_create<sol::table>();
    display["Flex"] = core::ui::Display::Flex;
    display["Grid"] = core::ui::Display::Grid;
    display["None"] = core::ui::Display::None;

    auto positioning = interface["Positioning"].get_or_create<sol::table>();
    positioning["Relative"] = core::ui::Positioning::Relative;
    positioning["Absolute"] = core::ui::Positioning::Absolute;

    auto direction = interface["Direction"].get_or_create<sol::table>();
    direction["Row"] = core::ui::Direction::Row;
    direction["RowReverse"] = core::ui::Direction::RowReverse;
    direction["Column"] = core::ui::Direction::Column;
    direction["ColumnReverse"] = core::ui::Direction::ColumnReverse;

    auto wrap = interface["Wrap"].get_or_create<sol::table>();
    wrap["None"] = core::ui::Wrap::None;
    wrap["Line"] = core::ui::Wrap::Line;
    wrap["Reverse"] = core::ui::Wrap::Reverse;

    auto justification = interface["Justification"].get_or_create<sol::table>();
    justification["Start"] = core::ui::Justification::Start;
    justification["Center"] = core::ui::Justification::Center;
    justification["End"] = core::ui::Justification::End;
    justification["Between"] = core::ui::Justification::Between;
    justification["Around"] = core::ui::Justification::Around;
    justification["Evenly"] = core::ui::Justification::Evenly;

    auto alignment = interface["Alignment"].get_or_create<sol::table>();
    alignment["Auto"] = core::ui::Alignment::Auto;
    alignment["Start"] = core::ui::Alignment::Start;
    alignment["Center"] = core::ui::Alignment::Center;
    alignment["End"] = core::ui::Alignment::End;
    alignment["Stretch"] = core::ui::Alignment::Stretch;
    alignment["Baseline"] = core::ui::Alignment::Baseline;
    alignment["Between"] = core::ui::Alignment::Between;
    alignment["Around"] = core::ui::Alignment::Around;

    lua.new_usertype<core::ui::Widget>("Widget",
        sol::constructors<core::ui::Widget()>(),
        "display", &core::ui::Widget::display,
        "policy", &core::ui::Widget::policy,
        "parent", &core::ui::Widget::parent,
        "compute", [](const core::ui::Widget& widget, const float width, const float height, const sol::object& registry) {
            widget.compute(width, height, registry);
        }
    );

    interface["Widget"] = [](sol::optional<sol::table> configuration) {
        core::ui::Widget instance;
        if (configuration) {
            instance.display = configuration->get_or("display", instance.display);
            instance.policy = configuration->get_or("policy", instance.policy);
            instance.parent = configuration->get_or("parent", instance.parent);
        }
        return instance;
    };

    interface["Constraints"] = [](sol::optional<sol::table> configuration) {
        core::ui::Constraints instance;
        if (configuration) {
            instance.width = configuration->get_or("width", instance.width);
            instance.height = configuration->get_or("height", instance.height);

            if (sol::optional<sol::table> bounds = (*configuration)["minimum"]) {
                instance.minimum[0] = bounds->get_or(1, instance.minimum[0]);
                instance.minimum[1] = bounds->get_or(2, instance.minimum[1]);
            }

            if (sol::optional<sol::table> limits = (*configuration)["maximum"]) {
                instance.maximum[0] = limits->get_or(1, instance.maximum[0]);
                instance.maximum[1] = limits->get_or(2, instance.maximum[1]);
            }
        }
        return instance;
    };

    interface["Layout"] = [](sol::optional<sol::table> configuration) {
        core::ui::Flexbox flexbox;
        if (configuration) {
            flexbox.direction = configuration->get_or("direction", flexbox.direction);
            flexbox.wrap = configuration->get_or("wrap", flexbox.wrap);
            flexbox.justification = configuration->get_or("justification", flexbox.justification);
            flexbox.items = configuration->get_or("items", flexbox.items);
            flexbox.self = configuration->get_or("self", flexbox.self);
            flexbox.content = configuration->get_or("content", flexbox.content);
            flexbox.grow = configuration->get_or("grow", flexbox.grow);
            flexbox.shrink = configuration->get_or("shrink", flexbox.shrink);
            flexbox.basis = configuration->get_or("basis", flexbox.basis);
        }
        return core::ui::Layout(flexbox);
    };

    interface["Margin"] = [](sol::optional<sol::table> configuration) {
        core::ui::Margin instance;
        if (configuration) {
            instance.top = configuration->get_or("top", instance.top);
            instance.right = configuration->get_or("right", instance.right);
            instance.bottom = configuration->get_or("bottom", instance.bottom);
            instance.left = configuration->get_or("left", instance.left);
        }
        return instance;
    };

    interface["Padding"] = [](sol::optional<sol::table> configuration) {
        core::ui::Padding instance;
        if (configuration) {
            instance.top = configuration->get_or("top", instance.top);
            instance.right = configuration->get_or("right", instance.right);
            instance.bottom = configuration->get_or("bottom", instance.bottom);
            instance.left = configuration->get_or("left", instance.left);
        }
        return instance;
    };

    interface["Gap"] = [](sol::optional<sol::table> configuration) {
        core::ui::Gap instance;
        if (configuration) {
            instance.main = configuration->get_or("main", instance.main);
            instance.cross = configuration->get_or("cross", instance.cross);
        }
        return instance;
    };

    interface["Style"] = [](sol::optional<sol::table> configuration) {
        core::ui::Style instance;
        if (configuration) {
            if (sol::optional<sol::table> values = (*configuration)["background"]) {
                core::ui::Rgb rgb{};
                rgb.red = values->get_or("red", rgb.red);
                rgb.green = values->get_or("green", rgb.green);
                rgb.blue = values->get_or("blue", rgb.blue);
                instance.background = rgb;
            }
        }
        return instance;
    };
}