#include "vm/api/ui.hpp"
#include "ui/widget.hpp"
#include "ui/motion.hpp"
#include <sol/sol.hpp>

void ui(lua_State* state) {
    sol::state_view lua(state);

    sol::table interface = lua.create_named_table("ui");

    auto display = interface["Display"].get_or_create<sol::table>();
    display["Flex"] = core::ui::Display::Flex;
    display["Grid"] = core::ui::Display::Grid;
    display["None"] = core::ui::Display::None;

    auto direction = interface["Direction"].get_or_create<sol::table>();
    direction["Row"] = interface.create_with(
        "Base", core::ui::Direction::Row,
        "Reverse", core::ui::Direction::RowReverse
    );
    direction["Column"] = interface.create_with(
        "Base", core::ui::Direction::Column,
        "Reverse", core::ui::Direction::ColumnReverse
    );

    auto policy = interface["Policy"].get_or_create<sol::table>();
    policy["Relative"] = core::ui::Policy::Relative;
    policy["Absolute"] = core::ui::Policy::Absolute;

    auto wrap = interface["Wrap"].get_or_create<sol::table>();
    wrap["None"] = core::ui::Wrap::None;
    wrap["Line"] = interface.create_with(
        "Base", core::ui::Wrap::Line,
        "Reverse", core::ui::Wrap::Reverse
    );

    auto justify = interface["Justify"].get_or_create<sol::table>();
    justify["Start"] = core::ui::Justification::Start;
    justify["Center"] = core::ui::Justification::Center;
    justify["End"] = core::ui::Justification::End;
    justify["Between"] = core::ui::Justification::Between;
    justify["Around"] = core::ui::Justification::Around;
    justify["Evenly"] = core::ui::Justification::Evenly;

    auto align = interface["Align"].get_or_create<sol::table>();
    align["Auto"] = core::ui::Alignment::Auto;
    align["Start"] = core::ui::Alignment::Start;
    align["Center"] = core::ui::Alignment::Center;
    align["End"] = core::ui::Alignment::End;
    align["Stretch"] = core::ui::Alignment::Stretch;
    align["Baseline"] = core::ui::Alignment::Baseline;
    align["Between"] = core::ui::Alignment::Between;
    align["Around"] = core::ui::Alignment::Around;

    auto ease = interface["Ease"].get_or_create<sol::table>();
    ease["Linear"] = core::ui::Ease::Linear;
    ease["OutQuad"] = core::ui::Ease::OutQuad;
    ease["OutCubic"] = core::ui::Ease::OutCubic;
    ease["OutElastic"] = core::ui::Ease::OutElastic;

    lua.new_usertype<core::ui::Dimensions>("Dimensions",
        "policy", &core::ui::Dimensions::policy,
        "width", &core::ui::Dimensions::width,
        "height", &core::ui::Dimensions::height
    );

    lua.new_usertype<core::ui::Flexbox>("Flexbox",
        sol::constructors<core::ui::Flexbox()>(),
        "direction", &core::ui::Flexbox::direction,
        "wrap", &core::ui::Flexbox::wrap,
        "justify", &core::ui::Flexbox::justification,
        "items", &core::ui::Flexbox::items,
        "self", &core::ui::Flexbox::self,
        "content", &core::ui::Flexbox::content,
        "grow", &core::ui::Flexbox::grow,
        "shrink", &core::ui::Flexbox::shrink,
        "basis", &core::ui::Flexbox::basis
    );

    lua.new_usertype<core::ui::Grid>("Grid",
        sol::constructors<core::ui::Grid()>(),
        "columns", &core::ui::Grid::columns,
        "rows", &core::ui::Grid::rows
    );

    lua.new_usertype<core::ui::Surroundings>("Surroundings",
        sol::constructors<core::ui::Surroundings()>(),
        "top", &core::ui::Surroundings::top,
        "right", &core::ui::Surroundings::right,
        "bottom", &core::ui::Surroundings::bottom,
        "left", &core::ui::Surroundings::left
    );

    lua.new_usertype<core::ui::Axis>("Axis",
        sol::constructors<core::ui::Axis()>(),
        "main", &core::ui::Axis::main,
        "cross", &core::ui::Axis::cross
    );

    lua.new_usertype<core::ui::Rgb>("Rgb",
        sol::constructors<core::ui::Rgb()>(),
        "red", &core::ui::Rgb::red,
        "green", &core::ui::Rgb::green,
        "blue", &core::ui::Rgb::blue
    );

    lua.new_usertype<core::ui::Oklch>("Oklch",
        sol::constructors<core::ui::Oklch()>(),
        "lightness", &core::ui::Oklch::lightness,
        "chroma", &core::ui::Oklch::chroma,
        "hue", &core::ui::Oklch::hue
    );

    lua.new_usertype<core::ui::Style>("Style",
        sol::constructors<core::ui::Style()>(),
        "background", sol::property(
            [&lua](core::ui::Style& style) -> sol::object {
                if (auto* rgb = std::get_if<core::ui::Rgb>(&style.background)) return sol::make_object(lua, *rgb);
                if (auto* oklch = std::get_if<core::ui::Oklch>(&style.background)) return sol::make_object(lua, *oklch);
                return sol::nil;
            },
            [](core::ui::Style& style, const sol::object &value) {
                if (value.is<core::ui::Rgb>()) style.background = value.as<core::ui::Rgb>();
                else if (value.is<core::ui::Oklch>()) style.background = value.as<core::ui::Oklch>();
            }
        )
    );

    lua.new_usertype<core::ui::Widget>("Widget",
        sol::constructors<core::ui::Widget()>(),
        "display", &core::ui::Widget::display,
        "size", &core::ui::Widget::dimensions,
        "inbound", &core::ui::Widget::padding,
        "outbound", &core::ui::Widget::margin,
        "gap", &core::ui::Widget::gap,
        "style", &core::ui::Widget::style,
        "flex", sol::property(
            [](core::ui::Widget& widget) -> core::ui::Flexbox* {
                if (auto* flexbox = std::get_if<core::ui::Flexbox>(&widget.layout)) return flexbox;
                return nullptr;
            },
            [](core::ui::Widget& widget, const core::ui::Flexbox& flexbox) {
                widget.layout = flexbox;
            }
        ),
        "grid", sol::property(
            [](core::ui::Widget& widget) -> core::ui::Grid* {
                if (auto* grid = std::get_if<core::ui::Grid>(&widget.layout)) return grid;
                return nullptr;
            },
            [](core::ui::Widget& widget, const core::ui::Grid& grid) {
                widget.layout = grid;
            }
        )
    );

    interface["Widget"] = []() {
        return core::ui::Widget();
    };

    interface["Motion"] = [](const sol::lightuserdata& target, const float end, const float duration, const core::ui::Ease ease) {
        if (auto* property = const_cast<float*>(static_cast<const float*>(target.pointer()))) {
            core::ui::motion.create(property, end, duration, ease);
        }
    };
}