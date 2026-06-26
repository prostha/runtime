#include "ui/widget.hpp"
#include "ui/defs/layout.hpp"
#include "ui/defs/geometry.hpp"
#include "ui/defs/spacing.hpp"
#include "core/ecs/world.hpp"
#include <yoga/Yoga.h>
#include <variant>

namespace core::ui {

    Widget::Widget() {
        node = YGNodeNew();
    }

    Widget::~Widget() {
        if (node) {
            YGNodeFree(node);
        }
    }

    void Widget::compute(const float width, const float height, const World& registry) const {
        if (!node) return;

        YGNodeStyleSetDisplay(node, static_cast<YGDisplay>(display));
        YGNodeStyleSetPositionType(node, static_cast<YGPositionType>(policy));

        auto& modifiable = const_cast<World&>(registry);

        const std::uint32_t layout = modifiable.component("Layout", sizeof(Layout));
        const std::uint32_t margin = modifiable.component("Margin", sizeof(Margin));
        const std::uint32_t inset = modifiable.component("Inset", sizeof(Inset));
        const std::uint32_t constraints = modifiable.component("Constraints", sizeof(Constraints));
        const std::uint32_t gap = modifiable.component("Gap", sizeof(Gap));

        if (registry.has(parent, layout)) {
            const auto& data = *static_cast<const Layout*>(registry.get(parent, layout));

            struct Visitor {
                YGNodeRef node;
                void operator()(const Flexbox& flex) const {
                    YGNodeStyleSetFlexDirection(node, static_cast<YGFlexDirection>(flex.direction));
                    YGNodeStyleSetFlexWrap(node, static_cast<YGWrap>(flex.wrap));
                    YGNodeStyleSetJustifyContent(node, static_cast<YGJustify>(flex.justification));
                    YGNodeStyleSetAlignItems(node, static_cast<YGAlign>(flex.items));
                    YGNodeStyleSetAlignSelf(node, static_cast<YGAlign>(flex.self));
                    YGNodeStyleSetAlignContent(node, static_cast<YGAlign>(flex.content));
                    YGNodeStyleSetFlexGrow(node, flex.grow);
                }
                void operator()(const Grid& grid) const {
                }
            };
            std::visit(Visitor{node}, data);
        }

        if (registry.has(parent, gap)) {
            const auto& data = *static_cast<const Gap*>(registry.get(parent, gap));
            YGNodeStyleSetGap(node, YGGutterColumn, data.main);
            YGNodeStyleSetGap(node, YGGutterRow, data.cross);
        }

        if (registry.has(parent, margin)) {
            const auto& data = *static_cast<const Margin*>(registry.get(parent, margin));
            YGNodeStyleSetMargin(node, YGEdgeTop,    data.top);
            YGNodeStyleSetMargin(node, YGEdgeRight,  data.right);
            YGNodeStyleSetMargin(node, YGEdgeBottom, data.bottom);
            YGNodeStyleSetMargin(node, YGEdgeLeft,   data.left);
        }

        if (policy == Positioning::Absolute && registry.has(parent, inset)) {
            const auto& data = *static_cast<const Inset*>(registry.get(parent, inset));
            YGNodeStyleSetPosition(node, YGEdgeTop,    data.top);
            YGNodeStyleSetPosition(node, YGEdgeRight,  data.right);
            YGNodeStyleSetPosition(node, YGEdgeBottom, data.bottom);
            YGNodeStyleSetPosition(node, YGEdgeLeft,   data.left);
        }

        if (registry.has(parent, constraints)) {
            const auto& data = *static_cast<const Constraints*>(registry.get(parent, constraints));

            if (data.width >= 0.0f) YGNodeStyleSetWidth(node, data.width);
            else YGNodeStyleSetWidthAuto(node);

            if (data.height >= 0.0f) YGNodeStyleSetHeight(node, data.height);
            else YGNodeStyleSetHeightAuto(node);

            if (data.minimum[0] >= 0.0f) YGNodeStyleSetMinWidth(node, data.minimum[0]);
            if (data.minimum[1] >= 0.0f) YGNodeStyleSetMinHeight(node, data.minimum[1]);
            if (data.maximum[0] >= 0.0f) YGNodeStyleSetMaxWidth(node, data.maximum[0]);
        }
    }
}