-- this scripts contains the AI classes for cards of thunder bird package

sgs.ai_skill_invoke.bow = true
sgs.ai_skill_invoke.tantei = true

-- lizhigushi
function SmartAI:useCardInspiration(inspiration, use)
	self:sort(self.friends, "hp")
	local f = 0
	for _, friend in ipairs(self.friends) do
		f = f + friend:getLostHp()
	end
	self:sort(self.enemies, "hp")
	local e = 0
	for _, enemy in ipairs(self.enemies) do
		e = e + enemy:getLostHp()
	end
	if e > f then return "." end
	use.card = inspiration
end

-- shejiweiren
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

-- lianji
function SmartAI:useCardPotential(potential, use)
	if self.player:hasUsed("Potential") then return "." end
	use.card = potential
end

-- touxi
function SmartAI:useCardTurnover(turnover, use)
	if not self.enemies[1] then return end
	self:sort(self.enemies, "hp")
	use.card = turnover
	if use.to then
		use.to:append(self.enemies[1])
	end
end

-- kongshoubailang
function SmartAI:useCardWolf(wolf, use)
	if self:isWeak() then return end
	self:sort(self.friends_noself, "handcard")
	local f = 0
	for _, friend in ipairs(self.friends_noself) do
		f = f + friend:getHandcardNum()
	end
	self:sort(self.enemies, "handcard")
	local e = 0
	for _, enemy in ipairs(self.enemies) do
		e = e + enemy:getHandcardNum()
	end
	if f > e then return end
	use.card = wolf
end

-- chongzai
function SmartAI:useCardLocust(card, use)
	use.card = card
end

-- watch
sgs.ai_skill_invoke["watch"] = function(self, data)
	local effect = data:toSlashEffect()
	local card = self:getMaxCard()
	if card:getNumber() == 13 then
		self.largest = card:getEffectiveId()
		return self:isEnemy(effect.to)
	end
	return false
end
