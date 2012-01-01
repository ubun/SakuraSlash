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

-- sacrifice
function SmartAI:useCardSacrifice(sacrifice, use)
	if self.player:hasSkill("wuyan") then return end
    if self.player:getHp() <= 2 then return end
	self:sort(self.friends_noself, "hp")
	for _, friend in ipairs(self.friends_noself) do
		if friend:isWounded() then
			use.card = sacrifice
			if use.to then
				use.to:append(friend)
			end
			break
		end
	end
end

-- emigration
function SmartAI:useCardEmigration(card, use)
--	table.sort(self.friends, hp_subtract_handcard)
	self:sort(self.friends, "hp")
--	local friends = self:exclude(self.friends, card)
	for _, friend in ipairs(self.friends) do
		if not friend:containsTrick("emigration") and not friend:hasSkill("keji") then			
			use.card = card
			if use.to then
				use.to:append(friend)
			end
			break
		end
	end
end
