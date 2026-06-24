#include "../include/ui/layout.hpp"
#include <yoga/Yoga.h>

namespace core::ui {

    void Layout::compute(YGNodeRef node) const {
        if (!node) return;

        YGNodeStyleSetDisplay(node, static_cast<YGDisplay>(display));

        YGNodeStyleSetFlexDirection(node, static_cast<YGFlexDirection>(flex.direction));
        YGNodeStyleSetFlexWrap(node, static_cast<YGWrap>(flex.wrap));
        YGNodeStyleSetJustifyContent(node, static_cast<YGJustify>(flex.justify));
        YGNodeStyleSetAlignItems(node, static_cast<YGAlign>(flex.items));
        YGNodeStyleSetAlignSelf(node, static_cast<YGAlign>(flex.self));
        YGNodeStyleSetAlignContent(node, static_cast<YGAlign>(flex.content));
        YGNodeStyleSetFlexGrow(node, flex.grow);
        YGNodeStyleSetFlexShrink(node, flex.shrink);

        if (flex.basis >= 0.0f) YGNodeStyleSetFlexBasis(node, flex.basis);
        else YGNodeStyleSetFlexBasisAuto(node);

        YGNodeStyleSetGap(node, YGGutterRow, flex.gap[0]);
        YGNodeStyleSetGap(node, YGGutterColumn, flex.gap[1]);

        if (size.width >= 0.0f) YGNodeStyleSetWidth(node, size.width);
        else YGNodeStyleSetWidthAuto(node);

        if (size.height >= 0.0f) YGNodeStyleSetHeight(node, size.height);
        else YGNodeStyleSetHeightAuto(node);

        if (size.minimum[0] >= 0.0f) YGNodeStyleSetMinWidth(node, size.minimum[0]);
        if (size.minimum[1] >= 0.0f) YGNodeStyleSetMinHeight(node, size.minimum[1]);
        if (size.maximum[0] >= 0.0f) YGNodeStyleSetMaxWidth(node, size.maximum[0]);
        if (size.maximum[1] >= 0.0f) YGNodeStyleSetMaxHeight(node, size.maximum[1]);

        YGNodeStyleSetPadding(node, YGEdgeTop,    inbound.padding[0]);
        YGNodeStyleSetPadding(node, YGEdgeRight,  inbound.padding[1]);
        YGNodeStyleSetPadding(node, YGEdgeBottom, inbound.padding[2]);
        YGNodeStyleSetPadding(node, YGEdgeLeft,   inbound.padding[3]);

        YGNodeStyleSetMargin(node, YGEdgeTop,    outbound.margin[0]);
        YGNodeStyleSetMargin(node, YGEdgeRight,  outbound.margin[1]);
        YGNodeStyleSetMargin(node, YGEdgeBottom, outbound.margin[2]);
        YGNodeStyleSetMargin(node, YGEdgeLeft,   outbound.margin[3]);

        YGNodeStyleSetPositionType(node, static_cast<YGPositionType>(edge.mode));
        if (edge.inset[0] >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeTop, edge.inset[0]);
        if (edge.inset[1] >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeRight, edge.inset[1]);
        if (edge.inset[2] >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeBottom, edge.inset[2]);
        if (edge.inset[3] >= 0.0f) YGNodeStyleSetPosition(node, YGEdgeLeft, edge.inset[3]);
    }

}