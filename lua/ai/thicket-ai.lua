-- chaidan
local chaidan_skill={}
chaidan_skill.name = "chaidan"
table.insert(sgs.ai_skills, chaidan_skill)
chaidan_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ChaidanCard") then
		return sgs.Card_Parse("@ChaidanCard=.")
	end
end
sgs.ai_skill_use_func["ChaidanCard"] = function(card, use, self)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if not friend:getJudgingArea():isEmpty() then
			if use.to then use.to:append(friend) end
			use.card = card
			return
		end
	end
	return
end

-- conghui
sgs.ai_skill_use["@@conghui"] = function(self, prompt)
	if self.player:getHandcardNum() - self.player:getHp() > 1 then return end
	if not self.player:isKongcheng() then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		return "@ConghuiCard=" .. cards[1]:getEffectiveId() .. "->."
	end
	return "."
end

-- qingyi
sgs.ai_skill_invoke["qingyi"] = function(self, data)
	local shao
	for _, friend in ipairs(self.friends_noself) do
		if friend:getKingdom() == "shao" then
			shao = friend
			self.qingyitarget = shao
			break
		end
	end
	return shao
end
sgs.ai_skill_playerchosen["qingyi"] = function(self, targets)
	return self.qingyitarget
end

-- hongmeng
local hongmeng_skill = {}
hongmeng_skill.name = "hongmeng"
table.insert(sgs.ai_skills, hongmeng_skill)
hongmeng_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("HongmengCard") then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards, true)
	for _, acard in ipairs(cards) do
		if acard:getSuit() == sgs.Card_Heart then
			card = acard
			break
		end
	end
	if card then
		return sgs.Card_Parse("@HongmengCard=" .. card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["HongmengCard"] = function(card,use,self)
    use.card=card
end

-- zilian
sgs.ai_skill_use["@@zilian"] = function(self, prompt)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card_ids = {}
	for i = 1, #cards do
		if not cards[i]:inherits("Peach") and not cards[i]:inherits("Jink") then
			table.insert(card_ids, cards[i]:getEffectiveId())
		end
	end
	return "@ZilianCard=" .. table.concat(card_ids, "+") .. "->."
end

-- zhongpu
sgs.ai_skill_invoke["zhongpu"] = function(self, data)
	return #self.friends_noself > 0
end
sgs.ai_skill_playerchosen["zhongpu"] = function(self, targets)
	self:sort(self.friends_noself, "handcard")
	self.zhongputarget = self.friends_noself[1]
	return self.zhongputarget
end
sgs.ai_skill_choice["zhongpu"] = function(self, choices)
	if self.zhongputarget:isWounded() then
		return "recover"
	else
		return "draw"
	end
end

-- shanjing & mangju
sgs.ai_skill_invoke["shanjing"] = true
sgs.ai_skill_invoke["mangju"] = true

-- anyong
sgs.ai_skill_invoke["anyong"] = function(self, data)
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enemy:getHp() then
			self.anyongtarget = shao
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["qingyi"] = function(self, targets)
	return self.anyongtarget
end
