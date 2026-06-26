#include "ui/widget.hpp"
#include "ui/defs/layout.hpp"
#include "ui/defs/geometry.hpp"
#include "ui/defs/spacing.hpp"
#include <yoga/Yoga.h>
#include <variant>

namespace core::ui {

    void Widget::compute(const float width, const float height, const auto& registry) const {
        if (!node) return;

        YGNodeStyleSetDisplay(node, static_cast<YGDisplay>(display));
        YGNodeStyleSetPositionType(node, static_cast<YGPositionType>(policy));

        if (registry.template has<Layout>(parent)) {
            const auto& layout = registry.template get<Layout>(parent);

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
                    YGNodeStyleSetFlexShrink(node, flex.shrink);

                    if (flex.basis >= 0.0f) {
                        YGNodeStyleSetFlexBasis(node, flex.basis);
                    } else {
                        YGNodeStyleSetFlexBasisAuto(node);
                    }
                }
                void operator()(const Grid& grid) const {
                }
            };

            std::visit(Visitor{node}, layout);
        }

        if (registry.template has<Gap>(parent)) {
            const auto& gap = registry.template get<Gap>(parent);
            YGNodeStyleSetGap(node, YGGutterRow, gap.main);
            YGNodeStyleSetGap(node, YGGutterColumn, gap.cross);
        }

        if (registry.template has<Padding>(parent)) {
            const auto& padding = registry.template get<Padding>(parent);
            YGNodeStyleSetPadding(node, YGEdgeTop,    padding.top);
            YGNodeStyleSetPadding(node, YGEdgeRight,  padding.right);
            YGNodeStyleSetPadding(node, YGEdgeBottom, padding.bottom);
            YGNodeStyleSetPadding(node, YGEdgeLeft,   padding.left);
        }

        if (registry.template has<Margin>(parent)) {
            const auto& margin = registry.template get<Margin>(parent);
            YGNodeStyleSetMargin(node, YGEdgeTop,    margin.top);
            YGNodeStyleSetMargin(node, YGEdgeRight,  margin.right);
            YGNodeStyleSetMargin(node, YGEdgeBottom, margin.bottom);
            YGNodeStyleSetMargin(node, YGEdgeLeft,   margin.left);
        }

        if (policy == Positioning::Absolute && registry.template has<Inset>(parent)) {
            const auto& inset = registry.template get<Inset>(parent);
            YGNodeStyleSetPosition(node, YGEdgeTop,    inset.top);
            YGNodeStyleSetPosition(node, YGEdgeRight,  inset.right);
            YGNodeStyleSetPosition(node, YGEdgeBottom, inset.bottom);
            YGNodeStyleSetPosition(node, YGEdgeLeft,   inset.left);
        }

        if (registry.template has<Constraints>(parent)) {
            const auto& limits = registry.template get<Constraints>(parent);

            if (limits.width >= 0.0f) YGNodeStyleSetWidth(node, limits.width);
            else YGNodeStyleSetWidthAuto(node);

            if (limits.height >= 0.0f) YGNodeStyleSetHeight(node, limits.height);
            else YGNodeStyleSetHeightAuto(node);

            if (limits.minimum[0] >= 0.0f) YGNodeStyleSetMinWidth(node, limits.minimum[0]);
            if (limits.minimum[1] >= 0.0f) YGNodeStyleSetMinHeight(node, limits.minimum[1]);
            if (limits.maximum[0] >= 0.0f) YGNodeStyleSetMaxWidth(node, limits.maximum[0]);
            if (limits.maximum[1] >= 0.0f) YGNodeStyleSetMaxHeight(node, limits.maximum[1]);
        } else {
            YGNodeStyleSetWidthAuto(node);
            YGNodeStyleSetHeightAuto(node);
        }

        YGNodeCalculateLayout(node, width, height, YGDirectionLTR);
    }

}