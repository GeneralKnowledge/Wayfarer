-- Copyright © 2026 Wayfarer Contributors.
-- SPDX-License-Identifier: GPL-3.0-or-later
-- Stage 0 development/debug economy panel.

local Game = require 'Game'
local ui = require 'pigui'
local Wayfarer = require 'Wayfarer'

local Vector2 = _G.Vector2

local windowFlags = ui.WindowFlags { "NoSavedSettings" }

local function drawPanel()
	if not Wayfarer or not Wayfarer.IsActive() then
		return
	end
	if not Game.player then
		return
	end

	ui.setNextWindowSize(Vector2(560, 580), "FirstUseEver")
	ui.setNextWindowPos(Vector2(20, 80), "FirstUseEver")
	ui.window("Wayfarer Economy (Stage 0)", windowFlags, function()
		-- ImGui text wraps poorly with huge blobs; show line-by-line.
		local panel = Wayfarer.DebugPanel() or ""
		for line in string.gmatch(panel .. "\n", "(.-)\n") do
			ui.text(line)
		end
		ui.separator()
		if ui.button("Accept Contract", Vector2(160, 0)) then
			local ok = Wayfarer.AcceptContract()
			print(ok and "Wayfarer: contract accepted" or "Wayfarer: accept failed")
		end
		ui.sameLine()
		if ui.button("Deliver Contract", Vector2(160, 0)) then
			local ok = Wayfarer.DeliverContract()
			print(ok and "Wayfarer: contract delivered" or "Wayfarer: deliver failed")
		end
		ui.sameLine()
		if ui.button("Travel: Frontier", Vector2(160, 0)) then
			Wayfarer.PlayerTravelTo("frontier_station")
			print("Wayfarer: player sim location -> Frontier Station")
		end
	end)
end

ui.registerModule("game", { id = "wayfarer-economy-panel", draw = drawPanel })
