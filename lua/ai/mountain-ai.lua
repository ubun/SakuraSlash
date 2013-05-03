-- this scripts contains the AI classes for generals of mountain package

-- bansha
sgs.ai_skill_invoke["bansha"] = sgs.ai_skill_invoke["duoren"]

-- foyuan
local foyuan_skill={}
foyuan_skill.name = "foyuan"
table.insert(sgs.ai_skills, foyuan_skill)
foyuan_skill.getTurnUseCard=function(self)
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
	for _, card in ipairs(cards) do
		if card:getTypeId() == sgs.Card_Basic and card:isBlack() then
		    local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("supply_shortage:foyuan[%s:%s]=%d"):format(suit, number, card_id)
			local supply_shortage = sgs.Card_Parse(card_str)
			assert(supply_shortage)
			return supply_shortage
		end
	end
end

-- jihun
sgs.ai_skill_cardask["@jihun"] = function(self, data)
	return "."
end

-- chengshi
sgs.ai_skill_invoke["chengshi"] = function(self, data)
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			self.chengshi = friend
			return true
		end
	end
end
sgs.ai_skill_playerchosen["chengshi"] = function(self, targets)
	return self.chengshi
end

-- yimeng
sgs.ai_skill_invoke["yimeng"] = true
sgs.ai_skill_playerchosen["yimeng"] = function(self, targets)
	return self.enemies[1]
end

-- zhibao
sgs.ai_skill_cardask["@zhibao"] = function(self, data)
	local damage = data:toDamage()
	local card = sgs.Sanguosha:cloneCard("duel", sgs.Card_NoSuit, 0)
	if self:cardProhibit(card, damage.from) then return "." end
	if self:isEnemy(damage.from) then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do
			if card:inherits("BasicCard") then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- feiti

-- shehang
local shehang_skill = {}
shehang_skill.name = "shehang"
table.insert(sgs.ai_skills, shehang_skill)
shehang_skill.getTurnUseCard = function(self)
--	if not self:slashIsAvailable() then return end
	if self.room:getCurrent() ~= self.player then return end
	local weapon
	self:sort(self.enemies, "threat")
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasEquip() and self:hasSkills(sgs.lose_equip_skill, friend) then
			weapon = friend:getEquips().first()
			break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not self:hasSkill(sgs.lose_equip_skill, enemy) and enemy:hasEquip() and not weapon then
			weapon = enemy:getEquips().last()
			break
		end
	end

	if weapon then
		local suit = weapon:getSuitString()
		local number = weapon:getNumberString()
		local card_id = weapon:getEffectiveId()
		local card_str = ("slash:shehang[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		assert(slash)
		return slash
	end
end

-- shehang-slash&jink
sgs.ai_skill_invoke["shehang"] = function(self, data)
	local asked = data:toString()
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasEquip() then
			return true
		end
	end
	if self.player:getHp() < 2 then
		for _, target in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if target:hasEquip() then
				return true
			end
		end
	end
	return false
end
sgs.ai_skill_playerchosen["shehang"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and not self:hasSkills(sgs.lose_equip_skill, player) then
			return player
		elseif self:isFriend(player) and (self:hasSkills(sgs.lose_equip_skill, player) or self.player:getHp() < 2) then
			return player
		end
	end
end
sgs.ai_skill_cardchosen["shehang"] = function(self, who)
	local ecards = who:getCards("e")
	ecards = sgs.QList2Table(ecards)
	self:sortByUseValue(ecards)
	return ecards[1]
end

-- canwu
local canwu_skill={}
canwu_skill.name = "canwu"
table.insert(sgs.ai_skills, canwu_skill)
canwu_skill.getTurnUseCard=function(self)
    if not self.player:hasUsed("CanwuCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		return sgs.Card_Parse("@CanwuCard=" .. max_card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["CanwuCard"]=function(card,use,self)
	self:sort(self.friends_noself, "hp")
	for _, friend in ipairs(self.friends_noself) do
		if friend:isWounded() and friend:getGeneral():isMale() and not friend:isKongcheng() then
		    if use.to then use.to:append(friend) end
            use.card=card
            break
		end
	end
end

-- chongwu
sgs.ai_skill_invoke["chongwu"] = function(self, data)
	if self:isWeak() then return false end
	local player = data:toPlayer()
	if self:isFriend(player) then return false end
	return player:getHp() < 1
end

-- luosha
sgs.ai_skill_invoke["luosha"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- zhaodai
zhaodai_skill={}
zhaodai_skill.name = "zhaodai"
table.insert(sgs.ai_skills, zhaodai_skill)
zhaodai_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ZhaodaiCard") then return end
	local slash = self:getCard("BasicCard")
	if not slash then return end
	return sgs.Card_Parse("@ZhaodaiCard=" .. slash:getEffectiveId())
end
sgs.ai_skill_use_func["ZhaodaiCard"] = function(card, use, self)
	self:sort(self.friends,"threat")
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			use.card = card
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
end
sgs.ai_skill_choice["zhaodai"] = function(self, choice, data)
	local effect = data:toCardEffect()
	if self:isEnemy(effect.to) then
		return "zi"
	elseif effect.from:getHandcardNum() > effect.to:getHandcardNum() then
		return "tian"
	else
		return "zi"
	end
end

-- kaxiang
sgs.ai_skill_invoke["kaxiang"] = function(self, data)
	local damage = data:toDamage()
	if damage.damage < 1 then return false end

	local max_card = self:getMaxCard()
	local max_care = self:getMaxCard(damage.from)
	if max_card and max_care and max_card:getNumber() > max_care:getNumber() then
		if self:isWeak() then
			return true
		else
			return self:isEnemy(damage.from)
		end
	else
		return false
	end
end
sgs.ai_skill_pindian["kaxiang"] = function(minusecard, self, requestor, maxcard)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local compare_func = function(a, b)
		return a:getNumber() > b:getNumber()
	end
	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if not card:inherits("Analeptic") and not card:inherits("Peach") then
			return card
		end
	end
end

-- jingshen
local jingshen_skill={}
jingshen_skill.name = "jingshen"
table.insert(sgs.ai_skills, jingshen_skill)
jingshen_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("JingshenCard") then return end
	local hcards = self.player:getCards("h")
	hcards = sgs.QList2Table(hcards)
	self:sortByUseValue(hcards, true)
	if self.player:getHandcardNum() > 3 then
		return sgs.Card_Parse("@JingshenCard=" .. hcards[1]:getEffectiveId())
	end
end
sgs.ai_skill_use_func["JingshenCard"]=function(card,use,self)
	use.card = card
end
sgs.ai_skill_playerchosen["jingshen"] = function(self, targets)
	self:sort(self.friends, "handcard")
	return self.friends[1]
end

