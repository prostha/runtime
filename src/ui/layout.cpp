#include "../include/core/types/components/rect.hpp"
#include "../include/core/types/components/bounds.hpp"
#include "../include/core/types/components/flow.hpp"
#include "../include/core/types/components/visibility.hpp"
#include "../include/core/ecs/zone.hpp"
#include "../include/core/ecs/find.hpp"
#include <vector>

using namespace core;
using namespace core::components;

namespace core::ui {

    void compute(const Id entity, Zone& zone, const uint32_t rect, const uint32_t bounds, const uint32_t flow, const uint32_t view) {
        if (const auto* visibility = static_cast<Visibility*>(zone.peek(entity, view)); visibility && !visibility->active) return;

        const auto* area = static_cast<Rect*>(zone.peek(entity, rect));
        const auto* size = static_cast<Bounds*>(zone.peek(entity, bounds));
        const auto* gait = static_cast<Flow*>(zone.peek(entity, flow));

        if (!area || !size) return;

        const std::vector<Id>& children = zone.crew(entity);

        for (const Id child : children) {
            compute(child, zone, rect, bounds, flow, view);
        }

        if (!gait || gait->axis == 0) return;

        float offset = 0.0f;
        for (const Id child : children) {
            if (const auto* sight = static_cast<Visibility*>(zone.peek(child, view)); sight && !sight->active) continue;

            auto* item = static_cast<Rect*>(zone.peek(child, rect));
            const auto* mold = static_cast<Bounds*>(zone.peek(child, bounds));

            if (!item || !mold) continue;

            if (gait->axis == 1) {
                item->position.x = area->position.x + size->padding.x + offset + mold->margin.x;
                item->position.y = area->position.y + size->padding.y + mold->margin.y;
                offset += mold->size.x + mold->margin.x + gait->gap;
            } else if (gait->axis == 2) {
                item->position.x = area->position.x + size->padding.x + mold->margin.x;
                item->position.y = area->position.y + size->padding.y + offset + mold->margin.y;
                offset += mold->size.y + mold->margin.y + gait->gap;
            }
        }
    }

    void layout(Zone& zone, const uint32_t rect, const uint32_t bounds, const uint32_t flow, const uint32_t view) {
        const Find query = zone.seek().with(rect).with(bounds);

        query.process([&](const std::size_t count, const Id* entities, const std::vector<void*>& blocks) {
            (void)blocks;
            for (std::size_t index = 0; index < count; ++index) {
                if (const Id entity = entities[index]; zone.boss(entity) == Null) {
                    compute(entity, zone, rect, bounds, flow, view);
                }
            }
        }, query.plan());
    }

}