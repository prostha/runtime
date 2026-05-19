local ffi = require("ffi")
local math3d = require("voxyl.math")
local ecs = require("voxyl.ecs")

-- 1. Define the C-structs so Lua knows how to read the raw memory pointers
ffi.cdef[[
    typedef struct { double x, y, z; } ecs_vec3_t;
    typedef struct { double value; } ecs_value_t;
]]

local environment = ecs.world.new()

-- 2. Enroll using exact byte sizes of the C structs
local position = environment:enroll(ffi.sizeof("ecs_vec3_t"))
local history  = environment:enroll(ffi.sizeof("ecs_vec3_t"))
local velocity = environment:enroll(ffi.sizeof("ecs_vec3_t"))
local force    = environment:enroll(ffi.sizeof("ecs_vec3_t"))
local mass     = environment:enroll(ffi.sizeof("ecs_value_t"))
local drag     = environment:enroll(ffi.sizeof("ecs_value_t"))
local bounce   = environment:enroll(ffi.sizeof("ecs_value_t"))

local anchored = environment:enroll(0) -- Tag components (0 bytes)
local sleeping = environment:enroll(0)
local dead     = environment:enroll(0)

local massive = environment:query()
massive:with(position)
massive:with(mass)
massive:with(force)
massive:without(anchored)

-- 3. Use ffi.cast to map the returned void* pointers to our structs
local star = environment:spawn()
local star_pos = ffi.cast("ecs_vec3_t*", environment:attach(star, position))
star_pos.x, star_pos.y, star_pos.z = 0, 0, 0
local star_mass = ffi.cast("ecs_value_t*", environment:attach(star, mass))
star_mass.value = 1.989e30
environment:attach(star, anchored)

local planet = environment:spawn()
local p_pos = ffi.cast("ecs_vec3_t*", environment:attach(planet, position))
p_pos.x, p_pos.y, p_pos.z = 149.6e9, 0, 0
local p_hist = ffi.cast("ecs_vec3_t*", environment:attach(planet, history))
p_hist.x, p_hist.y, p_hist.z = 149.6e9, 0, 0
local p_vel = ffi.cast("ecs_vec3_t*", environment:attach(planet, velocity))
p_vel.x, p_vel.y, p_vel.z = 0, 29783, 0
local p_force = ffi.cast("ecs_vec3_t*", environment:attach(planet, force))
p_force.x, p_force.y, p_force.z = 0, 0, 0
local p_mass = ffi.cast("ecs_value_t*", environment:attach(planet, mass))
p_mass.value = 5.972e24
local p_drag = ffi.cast("ecs_value_t*", environment:attach(planet, drag))
p_drag.value = 0.47

local moon = environment:spawn()
local m_pos = ffi.cast("ecs_vec3_t*", environment:attach(moon, position))
m_pos.x, m_pos.y, m_pos.z = 149.984e9, 0, 0
local m_hist = ffi.cast("ecs_vec3_t*", environment:attach(moon, history))
m_hist.x, m_hist.y, m_hist.z = 149.984e9, 0, 0
local m_vel = ffi.cast("ecs_vec3_t*", environment:attach(moon, velocity))
m_vel.x, m_vel.y, m_vel.z = 0, 30805, 0
local m_force = ffi.cast("ecs_vec3_t*", environment:attach(moon, force))
m_force.x, m_force.y, m_force.z = 0, 0, 0
local m_mass = ffi.cast("ecs_value_t*", environment:attach(moon, mass))
m_mass.value = 7.34e22
local m_bounce = ffi.cast("ecs_value_t*", environment:attach(moon, bounce))
m_bounce.value = 0.3

local probe = environment:spawn()
local pr_pos = ffi.cast("ecs_vec3_t*", environment:attach(probe, position))
pr_pos.x, pr_pos.y, pr_pos.z = 149.990e9, 0, 0
local pr_hist = ffi.cast("ecs_vec3_t*", environment:attach(probe, history))
pr_hist.x, pr_hist.y, pr_hist.z = 149.990e9, 0, 0
local pr_vel = ffi.cast("ecs_vec3_t*", environment:attach(probe, velocity))
pr_vel.x, pr_vel.y, pr_vel.z = 0, 30837, 0
local pr_force = ffi.cast("ecs_vec3_t*", environment:attach(probe, force))
pr_force.x, pr_force.y, pr_force.z = 0, 0, 0
local pr_mass = ffi.cast("ecs_value_t*", environment:attach(probe, mass))
pr_mass.value = 420
local pr_drag = ffi.cast("ecs_value_t*", environment:attach(probe, drag))
pr_drag.value = 0.47

local query_clear = environment:query()
query_clear:with(force)
query_clear:without(anchored)
query_clear:without(sleeping)
query_clear:without(dead)

local function system_clear()
    query_clear:run(function(count, entities, data)
        -- Cast the void array to our struct array
        local forces = ffi.cast("ecs_vec3_t*", data[1])
        for index = 0, count - 1 do
            forces[index].x = 0
            forces[index].y = 0
            forces[index].z = 0
        end
    end, { force })
end

local query_gravity = environment:query()
query_gravity:with(position)
query_gravity:with(mass)
query_gravity:with(force)
query_gravity:without(anchored)

local function system_gravity()
    query_gravity:run(function(count, entities, data)
        local positions = ffi.cast("ecs_vec3_t*", data[1])
        local masses = ffi.cast("ecs_value_t*", data[2])
        local forces = ffi.cast("ecs_vec3_t*", data[3])

        for index = 0, count - 1 do
            local identity = entities[index + 1]
            local place = positions[index]
            local weight = masses[index].value
            local current = forces[index]

            massive:run(function(total, others, inner_data)
                local locations = ffi.cast("ecs_vec3_t*", inner_data[1])
                local weights = ffi.cast("ecs_value_t*", inner_data[2])

                for inner = 0, total - 1 do
                    if others[inner + 1] ~= identity then
                        local target_loc = locations[inner]

                        -- Convert back to math3d for vector math
                        local place_v = math3d.vector3(place.x, place.y, place.z)
                        local target_v = math3d.vector3(target_loc.x, target_loc.y, target_loc.z)

                        local offset = target_v - place_v
                        local square = offset:length_sq()

                        if square >= 1 then
                            local scalar = 6.674e-11 * weight * weights[inner].value / (square * math.sqrt(square))
                            local applied = offset * scalar

                            -- Write back to raw memory
                            current.x = current.x + applied.x
                            current.y = current.y + applied.y
                            current.z = current.z + applied.z
                        end
                    end
                end
            end, { position, mass })
        end
    end, { position, mass, force })
end

local query_drag = environment:query()
query_drag:with(velocity)
query_drag:with(force)
query_drag:with(drag)
query_drag:without(anchored)
query_drag:without(sleeping)

local function system_drag()
    query_drag:run(function(count, entities, data)
        local velocities = ffi.cast("ecs_vec3_t*", data[1])
        local forces = ffi.cast("ecs_vec3_t*", data[2])
        local drags = ffi.cast("ecs_value_t*", data[3])

        for index = 0, count - 1 do
            local v = velocities[index]
            local motion = math3d.vector3(v.x, v.y, v.z)
            local speed = motion:length()
            if speed > 0 then
                local scale = -0.5 * 1.225 * drags[index].value * speed
                local applied = motion * scale
                forces[index].x = forces[index].x + applied.x
                forces[index].y = forces[index].y + applied.y
                forces[index].z = forces[index].z + applied.z
            end
        end
    end, { velocity, force, drag })
end

local query_integrate = environment:query()
query_integrate:with(position)
query_integrate:with(history)
query_integrate:with(force)
query_integrate:with(mass)
query_integrate:without(anchored)
query_integrate:without(sleeping)
query_integrate:without(dead)

local function system_integrate(time)
    query_integrate:run(function(count, entities, data)
        local positions = ffi.cast("ecs_vec3_t*", data[1])
        local histories = ffi.cast("ecs_vec3_t*", data[2])
        local forces = ffi.cast("ecs_vec3_t*", data[3])
        local masses = ffi.cast("ecs_value_t*", data[4])

        for index = 0, count - 1 do
            local p = positions[index]
            local h = histories[index]
            local f = forces[index]
            local inverse = 1 / masses[index].value

            local place = math3d.vector3(p.x, p.y, p.z)
            local past = math3d.vector3(h.x, h.y, h.z)
            local current = math3d.vector3(f.x, f.y, f.z)

            local combined = (place * 2) - past + (current * (inverse * time * time))

            h.x, h.y, h.z = p.x, p.y, p.z
            p.x, p.y, p.z = combined.x, combined.y, combined.z
        end
    end, { position, history, force, mass })
end

local query_sleep = environment:query()
query_sleep:with(velocity)
query_sleep:without(anchored)
query_sleep:without(sleeping)
query_sleep:without(dead)

local function system_sleep()
    query_sleep:run(function(count, entities, data)
        local velocities = ffi.cast("ecs_vec3_t*", data[1])
        local targets = {}

        for index = 0, count - 1 do
            local v = velocities[index]
            -- Avoid creating math3d.vector3 just to check length for speed
            local sq = (v.x * v.x) + (v.y * v.y) + (v.z * v.z)
            if sq < 1e-4 then
                table.insert(targets, entities[index + 1])
            end
        end
        for index = 1, #targets do
            environment:attach(targets[index], sleeping)
        end
    end, { velocity })
end

local query_wake = environment:query()
query_wake:with(velocity)
query_wake:with(sleeping)

local function system_wake()
    query_wake:run(function(count, entities, data)
        local velocities = ffi.cast("ecs_vec3_t*", data[1])
        local targets = {}
        for index = 0, count - 1 do
            local v = velocities[index]
            local sq = (v.x * v.x) + (v.y * v.y) + (v.z * v.z)
            if sq > 2.5e-3 then
                table.insert(targets, entities[index + 1])
            end
        end
        for index = 1, #targets do
            environment:detach(targets[index], sleeping)
        end
    end, { velocity, sleeping })
end

local function on_touched(first, second)
    environment:detach(first, sleeping)
    environment:detach(second, sleeping)

    if environment:has(first, velocity) and environment:has(first, bounce) and environment:has(second, bounce) then
        local motion = ffi.cast("ecs_vec3_t*", environment:get(first, velocity))
        local spring = ffi.cast("ecs_value_t*", environment:get(first, bounce)).value
        local cushion = ffi.cast("ecs_value_t*", environment:get(second, bounce)).value

        motion.y = -motion.y * (spring + cushion) * 0.5
    end

    if environment:has(first, history) and environment:has(first, position) then
        local past = ffi.cast("ecs_vec3_t*", environment:get(first, history))
        local place = ffi.cast("ecs_vec3_t*", environment:get(first, position))
        past.x, past.y, past.z = place.x, place.y, place.z
    end
end

local function on_destroyed(identity)
    if environment:has(identity, sleeping) then
        environment:detach(identity, sleeping)
    end
end