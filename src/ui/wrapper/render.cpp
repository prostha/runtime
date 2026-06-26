#include "ui/wrapper/render.hpp"
#include "ui/wrapper/context.hpp"
#include "ui/widget.hpp"
#include "core/ecs/world.hpp"
#include "core/ecs/find.hpp"
#include "core/primitives/vector4.hpp"
#include <yoga/Yoga.h>
#include <vector>

namespace core::ui {

    void dispatch(const void* handle, const World* world, const std::uint32_t tag) {
        if (!handle || !world) return;

        std::vector<primitives::Vector4> bounds;
        std::vector<std::uint32_t> indices;

        const Find query = world->query().with(tag);

        world->loop(query, [&](const std::size_t count, const Id* list, const std::vector<void*>& tabs) {
            if (tabs.empty() || !tabs[0] || count == 0) return;

            const auto* widgets = static_cast<const Widget*>(tabs[0]);

            bounds.reserve(bounds.size() + count);
            indices.reserve(indices.size() + count);

            for (std::size_t index = 0; index < count; ++index) {
                const auto& widget = widgets[index];
                if (widget.display == Display::None || !widget.node) continue;

                primitives::Vector4 rect;
                rect.x = YGNodeLayoutGetLeft(widget.node);
                rect.y = YGNodeLayoutGetTop(widget.node);
                rect.z = YGNodeLayoutGetWidth(widget.node);
                rect.w = YGNodeLayoutGetHeight(widget.node);

                bounds.push_back(rect);
                indices.push_back(list[index]);
            }
        });
    }

}