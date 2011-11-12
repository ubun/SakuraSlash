-- when enemy using the peach
sgs.ai_skill_invoke["grab_peach"] = function(self, data)
	local struct= data:toCardUse()
	return self:isEnemy(struct.from)
end

function SmartAI:useGaleShell(card, use)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <=1 and not enemy:hasSkill("jijiu") then
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
