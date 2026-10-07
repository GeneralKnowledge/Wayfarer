-- Copyright © 2026 Wayfarer Contributors.
-- SPDX-License-Identifier: GPL-3.0-or-later
-- Stage 0 development/debug economy panel.

local Game = require 'Game'
local ui = require 'pigui'
local Engine = require 'Engine'
local Wayfarer = require 'Wayfarer'

local Vector2 = _G.Vector2
local open = true

local function drawPanel()
	if not Wayfarer or not Wayfarer.IsActive() then
		return
	end
	if not Game.player then
		return
	end

	ui.setNextWindowSize(Vector2(520, 560), "FirstUseEver")
	ui.setNextWindowPos(Vector2(20, 80), "FirstUseEver")
	open = ui.beginWindow("Wayfarer Economy (Stage 0)", open)
	if open then
		ui.textWrapped(Wayfarer.DebugPanel())
		ui.separator()
		if ui.button("Accept Contract", Vector2(160, 0)) then
			local ok = Wayfarer.AcceptContract()
			print(ok and "Wayfarer: contract accepted" or "Wayfarer: accept failed")
		end
		ui.sameLine()
		if ui.button("Deliver Contract", Vector2(160, 0)) then
			-- Player should fly with Pioneer controls; this completes after arrival sync.
			local ok = Wayfarer.DeliverContract()
			print(ok and "Wayfarer: contract delivered" or "Wayfarer: deliver failed")
		end
		ui.sameLine()
		if ui.button("Travel: Frontier", Vector2(160, 0)) then
			Wayfarer.PlayerTravelTo("frontier_station")
			print("Wayfarer: player sim location -> Frontier Station")
		end
	end
	ui.endWindow()
end

ui.registerModule("game", { id = "wayfarer-economy-panel", draw = drawPanel })
