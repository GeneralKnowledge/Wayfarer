-- Copyright © 2026 Wayfarer Contributors.
-- SPDX-License-Identifier: GPL-3.0-or-later
--
-- Stage 0 vertical slice glue:
--   * starts the headless Wayfarer simulation
--   * spawns a visible Pioneer NPC freighter
--   * steers it with Pioneer AI as the sim trader travels
--   * keeps player flight controls unchanged

local Event = require 'Event'
local Game = require 'Game'
local Space = require 'Space'
local Timer = require 'Timer'
local Wayfarer = require 'Wayfarer'

local npcShip = nil
local lastStatus = nil
local bodyHints = {
	mining_world = nil,
	industrial_world = nil,
	frontier_station = nil,
}

local function pickBodies()
	bodyHints.mining_world = nil
	bodyHints.industrial_world = nil
	bodyHints.frontier_station = nil

	local stations = {}
	local planets = {}
	for i = 1, #Space.GetBodies() do
		local b = Space.GetBodies()[i]
		local t = b.superType or b.type
		-- Body type checks vary; prefer stations then rocky worlds.
		if b.docking or (b.type and tostring(b.type):find("STARPORT")) then
			stations[#stations + 1] = b
		elseif b.type and (tostring(b.type):find("PLANET") or tostring(b.type):find("WORLD")) then
			planets[#planets + 1] = b
		end
	end

	-- Fallbacks using GetBodies around player.
	if #stations == 0 or #planets == 0 then
		for _, b in pairs(Space.GetBodies()) do
			if b ~= Game.player then
				if not bodyHints.mining_world then
					bodyHints.mining_world = b
				elseif not bodyHints.industrial_world then
					bodyHints.industrial_world = b
				elseif not bodyHints.frontier_station then
					bodyHints.frontier_station = b
				end
			end
		end
		return
	end

	bodyHints.mining_world = planets[1] or stations[1]
	bodyHints.industrial_world = planets[2] or stations[1]
	bodyHints.frontier_station = stations[1] or planets[1]
end

local function spawnNpc()
	if npcShip and npcShip:exists() then
		return
	end
	pickBodies()
	local near = bodyHints.mining_world or Game.player
	-- Pioneer freighter model used as visual stand-in for Wayfarer merchant hull.
	local ok, ship = pcall(function()
		return Space.SpawnShipNear("lodos", near, 50, 80)
	end)
	if not ok or not ship then
		ok, ship = pcall(function()
			return Space.SpawnShipNear("kanara", near, 50, 80)
		end)
	end
	if ok and ship then
		npcShip = ship
		ship:SetLabel("TSV Merchant")
		print("Wayfarer: spawned NPC trader " .. tostring(ship.label))
	else
		print("Wayfarer: failed to spawn NPC ship")
	end
end

local function steerNpc()
	if not Wayfarer or not Wayfarer.IsActive() or not npcShip or not npcShip:exists() then
		return
	end
	if Wayfarer.NpcCount() < 1 then
		return
	end

	local status = Wayfarer.NpcStatus(0)
	local destName = Wayfarer.NpcDestination(0)
	if status ~= lastStatus then
		print(string.format("Wayfarer NPC: %s -> %s (%s)", Wayfarer.NpcName(0), tostring(destName), status))
		lastStatus = status
	end

	if status == "In Transit" then
		pickBodies()
		local target = nil
		if destName == "Industrial World" then
			target = bodyHints.industrial_world
		elseif destName == "Mining World" then
			target = bodyHints.mining_world
		elseif destName == "Frontier Station" then
			target = bodyHints.frontier_station
		end
		target = target or bodyHints.industrial_world or Game.player
		pcall(function()
			npcShip:AIFlyTo(target)
		end)
	end
end

local function onGameStart()
	if not Wayfarer then
		print("Wayfarer: Lua API not registered")
		return
	end
	Wayfarer.StartStage0()
	print("Wayfarer Stage 0 simulation started")
	Timer:CallAt(Game.time + 2, function()
		spawnNpc()
	end)
	Timer:CallEvery(2, function()
		if not Game.player then
			return false
		end
		steerNpc()
		return true
	end)
end

local function onGameEnd()
	if Wayfarer then
		Wayfarer.Shutdown()
	end
	npcShip = nil
	lastStatus = nil
end

local function onShipDocked(ship, station)
	if ship ~= Game.player then
		return
	end
	if not Wayfarer or not Wayfarer.IsActive() then
		return
	end
	-- Map docking to frontier delivery when a contract is accepted.
	Wayfarer.PlayerTravelTo("frontier_station")
	local ok = Wayfarer.DeliverContract()
	if ok then
		print("Wayfarer: docked delivery complete")
	end
end

Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onShipDocked", onShipDocked)
