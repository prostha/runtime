#include "../include/vm/api/ui.hpp"
#include "../include/ui/layout.hpp"
#include "../include/ui/node.hpp"
#include "../include/ui/motion.hpp"
#include <sol/sol.hpp>

void ui(lua_State* state) {
    sol::state_view lua(state);

    sol::table interface = lua.create_named_table("ui");

    auto display = interface["Display"].get_or_create<sol::table>();
    display["Flex"] = core::ui::Display::Flex;
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

    auto position = interface["Position"].get_or_create<sol::table>();
    position["Relative"] = core::ui::Position::Relative;
    position["Absolute"] = core::ui::Position::Absolute;

    auto wrap = interface["Wrap"].get_or_create<sol::table>();
    wrap["None"] = core::ui::Wrap::None;
    wrap["Line"] = interface.create_with(
        "Base", core::ui::Wrap::Line,
        "Reverse", core::ui::Wrap::Reverse
    );

    auto justify = interface["Justify"].get_or_create<sol::table>();
    justify["Start"] = core::ui::Flex::Justify::Start;
    justify["Center"] = core::ui::Flex::Justify::Center;
    justify["End"] = core::ui::Flex::Justify::End;
    justify["Between"] = core::ui::Flex::Justify::Between;
    justify["Around"] = core::ui::Flex::Justify::Around;
    justify["Evenly"] = core::ui::Flex::Justify::Evenly;

    auto align = interface["Align"].get_or_create<sol::table>();
    align["Auto"] = core::ui::Flex::Align::Auto;
    align["Start"] = core::ui::Flex::Align::Start;
    align["Center"] = core::ui::Flex::Align::Center;
    align["End"] = core::ui::Flex::Align::End;
    align["Stretch"] = core::ui::Flex::Align::Stretch;
    align["Baseline"] = core::ui::Flex::Align::Baseline;
    align["Between"] = core::ui::Flex::Align::Between;
    align["Around"] = core::ui::Flex::Align::Around;

    auto ease = interface["Ease"].get_or_create<sol::table>();
    ease["Linear"] = core::ui::Ease::Linear;
    ease["OutQuad"] = core::ui::Ease::OutQuad;
    ease["OutCubic"] = core::ui::Ease::OutCubic;
    ease["OutElastic"] = core::ui::Ease::OutElastic;

    lua.new_usertype<core::ui::Size>("Size",
        "width", &core::ui::Size::width,
        "height", &core::ui::Size::height,
        "minimum", &core::ui::Size::minimum,
        "maximum", &core::ui::Size::maximum
    );

    lua.new_usertype<core::ui::Flex>("Flex",
        "direction", &core::ui::Flex::direction,
        "wrap", &core::ui::Flex::wrap,
        "justify", &core::ui::Flex::justify,
        "items", &core::ui::Flex::items,
        "self", &core::ui::Flex::self,
        "content", &core::ui::Flex::content,
        "grow", &core::ui::Flex::grow,
        "shrink", &core::ui::Flex::shrink,
        "basis", &core::ui::Flex::basis,
        "gap", &core::ui::Flex::gap
    );

    lua.new_usertype<core::ui::Inbound>("Inbound",
        "padding", &core::ui::Inbound::padding
    );

    lua.new_usertype<core::ui::Outbound>("Outbound",
        "margin", &core::ui::Outbound::margin
    );

    lua.new_usertype<core::ui::Edge>("Edge",
        "mode", &core::ui::Edge::mode,
        "inset", &core::ui::Edge::inset
    );

    lua.new_usertype<core::ui::Layout>("Layout",
        sol::constructors<core::ui::Layout()>(),
        "display", &core::ui::Layout::display,
        "size", &core::ui::Layout::size,
        "flex", &core::ui::Layout::flex,
        "inbound", &core::ui::Layout::inbound,
        "outbound", &core::ui::Layout::outbound,
        "edge", &core::ui::Layout::edge
    );

    lua.new_usertype<core::ui::Node>("Node",
        sol::constructors<core::ui::Node()>(),
        "layout", &core::ui::Node::layout,
        "attach", &core::ui::Node::attach,
        "remove", &core::ui::Node::remove,
        "compute", &core::ui::Node::compute
    );

    interface["Layout"] = []() {
        return core::ui::Layout();
    };

    interface["Node"] = []() {
        return core::ui::Node();
    };

    interface["Motion"] = [](const sol::lightuserdata& target, const float end, const float duration, const core::ui::Ease ease) {
        if (auto* property = const_cast<float*>(static_cast<const float*>(target.pointer()))) {
            core::ui::motion.create(property, end, duration, ease);
        }
    };
}
