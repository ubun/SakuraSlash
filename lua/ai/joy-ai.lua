-- when enemy using the peach
sgs.ai_skill_invoke["grab_peach"] = function(self, data)
	local struct= data:toCardUse()
	return self:isEnemy(struct.from) and (struct.to:isEmpty() or self:isEnemy(struct.to:first()))
end


sgs.ai_skill_playerchosen["yx_sword"] = function(self, targets)
	local who = self.room:getTag("YxSwordVictim"):toPlayer()
	if who then
		if who:getRole() == "rebel" then
			for _, player in sgs.qlist(targets) do
				if self:isFriend(player) then
					return player
				end
			end
		elseif who:getRole() == "loyalist" then
			if self:isEnemy(who) then return self.room:getLord() end
		end
	end
	
	return self.enemies[1]
end

function SmartAI:useGaleShell(card, use)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <=1 and not self:hasSkills("jijiu|wusheng|longhun",enemy) then
			use.card = card
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
end

