#include "ui/widget.hpp"
#include <yoga/Yoga.h>

namespace core::ui {

    void Widget::compute(float width, float height, const auto& lookup) const {
        YGNodeRef node = this->yg_node;
        if (!node) return;

        YGNodeStyleSetDisplay(node, static_cast<YGDisplay>(display));

        if (std::holds_alternative<Flexbox>(layout)) {
            const auto&[direction, wrap, justification, items, self, content, grow, shrink, basis] = std::get<Flexbox>(layout);

            YGNodeStyleSetFlexDirection(node, static_cast<YGFlexDirection>(direction));
            YGNodeStyleSetFlexWrap(node, static_cast<YGWrap>(wrap));
            YGNodeStyleSetJustifyContent(node, static_cast<YGJustify>(justification));
            YGNodeStyleSetAlignItems(node, static_cast<YGAlign>(items));
            YGNodeStyleSetAlignSelf(node, static_cast<YGAlign>(self));
            YGNodeStyleSetAlignContent(node, static_cast<YGAlign>(content));
            YGNodeStyleSetFlexGrow(node, grow);
            YGNodeStyleSetFlexShrink(node, shrink);

            if (basis >= 0.0f) {
                YGNodeStyleSetFlexBasis(node, basis);
            } else {
                YGNodeStyleSetFlexBasisAuto(node);
            }
        }

        YGNodeStyleSetGap(node, YGGutterRow, gap.main);
        YGNodeStyleSetGap(node, YGGutterColumn, gap.cross);

        if (dimensions.width >= 0.0f) YGNodeStyleSetWidth(node, dimensions.width);
        else YGNodeStyleSetWidthAuto(node);

        if (dimensions.height >= 0.0f) YGNodeStyleSetHeight(node, dimensions.height);
        else YGNodeStyleSetHeightAuto(node);

        if (dimensions.minimum >= 0.0f) YGNodeStyleSetMinWidth(node, dimensions.minimum);
        if (dimensions.minimum >= 0.0f) YGNodeStyleSetMinHeight(node, dimensions.minimum);
        if (dimensions.maximum >= 0.0f) YGNodeStyleSetMaxWidth(node, dimensions.maximum);
        if (dimensions.maximum >= 0.0f) YGNodeStyleSetMaxHeight(node, dimensions.maximum);

        YGNodeStyleSetPadding(node, YGEdgeTop,    padding.top);
        YGNodeStyleSetPadding(node, YGEdgeRight,  padding.right);
        YGNodeStyleSetPadding(node, YGEdgeBottom, padding.bottom);
        YGNodeStyleSetPadding(node, YGEdgeLeft,   padding.left);

        YGNodeStyleSetMargin(node, YGEdgeTop,    margin.top);
        YGNodeStyleSetMargin(node, YGEdgeRight,  margin.right);
        YGNodeStyleSetMargin(node, YGEdgeBottom, margin.bottom);
        YGNodeStyleSetMargin(node, YGEdgeLeft,   margin.left);

        YGNodeStyleSetPositionType(node, static_cast<YGPositionType>(dimensions.policy));

        if (dimensions.inset >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeTop,    dimensions.inset);
        if (dimensions.inset >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeRight,  dimensions.inset);
        if (dimensions.inset >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeBottom, dimensions.inset);
        if (dimensions.inset >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeLeft,   dimensions.inset);
    }

}