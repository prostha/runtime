local ecs = require("ecs")

-- 1. Initialize the World using our clean global syntax
local world = ecs.World()

-- 2. Define Components (Returns sequential IDs: 0, 1, 2...)
local Position     = ecs.Component({ "x", "y", "z" })
local PrevPosition = ecs.Component({ "x", "y", "z" })
local Velocity     = ecs.Component({ "x", "y", "z" })
local Force        = ecs.Component({ "x", "y", "z" })
local Mass         = ecs.Component({ "value" })
local Heat         = ecs.Component({ "kelvin", "conductivity" })
local Charge       = ecs.Component({ "value" })
local Drag         = ecs.Component({ "value" })
local Bounce       = ecs.Component({ "value" })

-- Tag components have no data layout (0 bytes)
local Anchored     = ecs.Component()
local Sleeping     = ecs.Component()
local Frozen       = ecs.Component()
local Dead         = ecs.Component()

-- 3. Cache reusable standalone Queries for nested system loops
local massive = world:query({ Position, Mass, Force }):without(Anchored)
local charged = world:query({ Position, Charge, Force }):without(Anchored)
local thermal = world:query({ Position, Heat }):without(Frozen)

-- 4. Spawn Entities & Populate Fields
local star = world:spawn()
world:set(star, Position, { 0, 0, 0 })
world:set(star, Mass,     { 1.989e30 })
world:set(star, Heat,     { 5778, 0.03 })
world:set(star, Anchored) -- Tagging

local planet = world:spawn()
world:set(planet, Position,     { 149.6e9, 0, 0 })
world:set(planet, PrevPosition, { 149.6e9, 0, 0 })
world:set(planet, Velocity,     { 0, 29783, 0 })
world:set(planet, Force,        { 0, 0, 0 })
world:set(planet, Mass,         { 5.972e24 })
world:set(planet, Heat,         { 288, 1.5 })
world:set(planet, Drag,         { 0.47 })

local moon = world:spawn()
world:set(moon, Position,     { 149.984e9, 0, 0 })
world:set(moon, PrevPosition, { 149.984e9, 0, 0 })
world:set(moon, Velocity,     { 0, 30805, 0 })
world:set(moon, Force,        { 0, 0, 0 })
world:set(moon, Mass,         { 7.34e22 })
world:set(moon, Heat,         { 220, 0.2 })
world:set(moon, Bounce,       { 0.3 })

local probe = world:spawn()
world:set(probe, Position,     { 149.990e9, 0, 0 })
world:set(probe, PrevPosition, { 149.990e9, 0, 0 })
world:set(probe, Velocity,     { 0, 30837, 0 })
world:set(probe, Force,        { 0, 0, 0 })
world:set(probe, Mass,         { 420 })
world:set(probe, Charge,       { 1.2e-6 })
world:set(probe, Drag,         { 0.47 })


-- 5. Highly Optimized Archetype Systems
-- Rather than processing one entity per call, the callback receives:
-- (count, entity_array, component_arrays...)

world:system("clear_forces", { Force })
    :without(Anchored, Sleeping, Dead)
    :run(function(dt, count, entities, forces)
        -- Pure array-index loop: zero table garbage generation!
        for i = 0, count - 1 do
            forces[i].x = 0
            forces[i].y = 0
            forces[i].z = 0
        end
    end)

world:system("gravity", { Position, Mass, Force })
    :without(Anchored)
    :run(function(dt, count, entities, positions, masses, forces)
        for i = 0, count - 1 do
            local id    = entities[i]
            local pos   = positions[i]
            local mass  = masses[i]
            local force = forces[i]

            -- Read nested data in batches from archetypes matching 'massive'
            massive:each(function(other_count, other_entities, o_positions, o_masses)
                for j = 0, other_count - 1 do
                    local other = other_entities[j]
                    if other ~= id then
                        local opos = o_positions[j]
                        local omass = o_masses[j]

                        local dx = opos.x - pos.x
                        local dy = opos.y - pos.y
                        local dz = opos.z - pos.z
                        local r2 = dx*dx + dy*dy + dz*dz

                        if r2 >= 1 then
                            local inv = 6.674e-11 * mass.value * omass.value / (r2 * math.sqrt(r2))
                            force.x = force.x + dx * inv
                            force.y = force.y + dy * inv
                            force.z = force.z + dz * inv
                        end
                    end
                end
            end)
        end
    end)

world:system("coulomb", { Position, Charge, Force })
    :without(Anchored)
    :run(function(dt, count, entities, positions, charges, forces)
        for i = 0, count - 1 do
            local id     = entities[i]
            local pos    = positions[i]
            local charge = charges[i]
            local force  = forces[i]

            charged:each(function(other_count, other_entities, o_positions, o_charges)
                for j = 0, other_count - 1 do
                    local other = other_entities[j]
                    if other ~= id then
                        local opos = o_positions[j]
                        local ocharge = o_charges[j]

                        local dx = opos.x - pos.x
                        local dy = opos.y - pos.y
                        local dz = opos.z - pos.z
                        local r2 = dx*dx + dy*dy + dz*dz

                        if r2 >= 0.01 then
                            local sign = (charge.value * ocharge.value > 0) and -1 or 1
                            local inv  = sign * 8.99e9 * math.abs(charge.value * ocharge.value) / (r2 * math.sqrt(r2))
                            force.x = force.x + dx * inv
                            force.y = force.y + dy * inv
                            force.z = force.z + dz * inv
                        end
                    end
                end
            end)
        end
    end)

world:system("drag", { Velocity, Force, Drag })
    :without(Anchored, Sleeping)
    :run(function(dt, count, entities, velocities, forces, drags)
        for i = 0, count - 1 do
            local vel   = velocities[i]
            local force = forces[i]
            local speed = math.sqrt(vel.x*vel.x + vel.y*vel.y + vel.z*vel.z)
            if speed > 0 then
                local scale = -0.5 * 1.225 * drags[i].value * speed
                force.x = force.x + vel.x * scale
                force.y = force.y + vel.y * scale
                force.z = force.z + vel.z * scale
            end
        end
    end)

world:system("integrate", { Position, PrevPosition, Force, Mass })
    :without(Anchored, Sleeping, Dead)
    :run(function(dt, count, entities, positions, prev_positions, forces, masses)
        for i = 0, count - 1 do
            local pos   = positions[i]
            local prev  = prev_positions[i]
            local force = forces[i]
            local inv   = 1 / masses[i].value

            local nx = 2 * pos.x - prev.x + force.x * inv * dt * dt
            local ny = 2 * pos.y - prev.y + force.y * inv * dt * dt
            local nz = 2 * pos.z - prev.z + force.z * inv * dt * dt

            prev.x, prev.y, prev.z = pos.x, pos.y, pos.z
            pos.x,  pos.y,  pos.z  = nx,    ny,    nz
        end
    end)

world:system("conduction", { Position, Heat })
    :without(Frozen)
    :run(function(dt, count, entities, positions, heats)
        for i = 0, count - 1 do
            local id   = entities[i]
            local pos  = positions[i]
            local heat = heats[i]

            thermal:each(function(other_count, other_entities, o_positions, o_heats)
                for j = 0, other_count - 1 do
                    local other = other_entities[j]
                    if other ~= id then
                        local opos  = o_positions[j]
                        local oheat = o_heats[j]

                        local dx   = opos.x - pos.x
                        local dy   = opos.y - pos.y
                        local dz   = opos.z - pos.z
                        local dist = math.sqrt(dx*dx + dy*dy + dz*dz)

                        if dist <= 4 then
                            heat.kelvin = heat.kelvin + heat.conductivity * (oheat.kelvin - heat.kelvin) * dt
                        end
                    end
                end
            end)
        end
    end)

world:system("sleep", { Velocity })
    :without(Anchored, Sleeping, Dead)
    :run(function(dt, count, entities, velocities)
        for i = 0, count - 1 do
            local vel = velocities[i]
            if vel.x*vel.x + vel.y*vel.y + vel.z*vel.z < 1e-4 then
                world:set(entities[i], Sleeping)
            end
        end
    end)

world:system("wake", { Velocity, Sleeping })
    :run(function(dt, count, entities, velocities)
        for i = 0, count - 1 do
            local vel = velocities[i]
            if vel.x*vel.x + vel.y*vel.y + vel.z*vel.z > 2.5e-3 then
                world:remove(entities[i], Sleeping)
            end
        end
    end)

-- 6. Event Listeners (Safe, direct getters)
world:on("Touched", function(a, b)
    world:remove(a, Sleeping)
    world:remove(b, Sleeping)

    local va = world:get(a, Velocity)
    local ba = world:get(a, Bounce)
    local bb = world:get(b, Bounce)

    if va and ba and bb then
        va.y = -va.y * (ba.value + bb.value) * 0.5
    end

    local prev = world:get(a, PrevPosition)
    local pos  = world:get(a, Position)
    if prev and pos then
        prev.x, prev.y, prev.z = pos.x, pos.y, pos.z
    end
end)

world:on("Destroyed", function(id)
    world:remove(id, Sleeping)
end)