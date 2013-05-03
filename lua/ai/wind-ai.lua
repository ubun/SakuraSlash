-- this scripts contains the AI classes for generals of wind package

-- bianhu
sgs.ai_skill_playerchosen["bianhu"] = function(self, targets)
	return self.player
end
sgs.ai_skill_cardask["@bianhu"] = function(self, data)
	local use = data:toCardUse()
	if self:isEnemy(use.from) and use.card:inherits("ExNihilo") then
		local allcards = self.player:getCards("he")
		for _, card in sgs.qlist(allcards) do
			if (pattern == "..S" and card:getSuit() == sgs.Card_Spade) or
				(pattern == "..H" and card:getSuit() == sgs.Card_Heart) or
				(pattern == "..C" and card:getSuit() == sgs.Card_Club) or
				(pattern == "..D" and card:getSuit() == sgs.Card_Diamond) then
				return card:getEffectiveId()
			end
		end
	end
--	if self:isFriend(use.to) and use.card:inherits("Dismantlement") then
--		return true
--	end
	return "."
end

-- fenju
sgs.ai_skill_invoke["fenju"] = function(self, data)
	return #self.friends_noself > 0
end
sgs.ai_skill_playerchosen["fenju"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) then
			return player
		end
	end
end

-- fating
sgs.ai_skill_invoke["@fating"] = function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
--		self:log("fating!!!!!!!!")
		local all_cards = self.player:getCards("h")
		local cards = {}
		for _, card in sgs.qlist(all_cards) do
			if card:inherits("BasicCard") then
				table.insert(cards, card)
			end
		end
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@FatingCard=" .. card_id
		end
	end
	return "."
end

-- manyu
sgs.ai_skill_invoke["manyu"] = function(self,prompt)
	return #self.friends_noself > 0
end
sgs.ai_skill_playerchosen["manyu"] = function(self, targets)
	return self.friends_noself[1]
end

-- tuanzhang
local tuanzhang_skill={}
tuanzhang_skill.name = "tuanzhang"
table.insert(sgs.ai_skills, tuanzhang_skill)
tuanzhang_skill.getTurnUseCard = function(self)
	local card
	local hcards = self.player:getCards("h")
	hcards = sgs.QList2Table(hcards)
	self:sortByUseValue(hcards, true)
	for _, hcard in ipairs(hcards) do
		if hcard:inherits("Peach") then
			card = hcard
			break
		end
	end
	if not card then return end
	self:sort(self.friends, "hp")
	if self.friends[1]:isWounded() then
		return sgs.Card_Parse("@TuanzhangCard=" .. card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["TuanzhangCard"]=function(card,use,self)
	self:sort(self.friends, "hp")
	if use.to then use.to:append(self.friends[1]) end
	use.card = card
end

-- huachi
sgs.ai_skill_use["@@huachi"] = function(self, prompt)
	local final
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getGeneral():isMale() then
			final = enemy
		end
	end
	if final then
		return "@HuachiCard=.->" .. final:objectName()
	else
		return "."
	end
end

-- ouxiang
sgs.ai_skill_invoke["ouxiang"] = function(self, data)
	return self.player:getHandcardNum() < 10
end

-- lingjia
sgs.ai_skill_playerchosen["lingjia"] = sgs.ai_skill_playerchosen["fenju"]
sgs.ai_skill_cardask["@lingjia"] = function(self, data)
	local carduse = data:toCardUse()
	if self:isEnemy(carduse.from) then
		local allcards = self.player:getCards("he")
		allcards = sgs.QList2Table(allcards)
		for _, fcard in ipairs(allcards) do
			if self.player:getMark("lingjia") == carduse.card:getNumber() then
				return fcard:getEffectiveId()
			end
		end
	end
	return "."
end

-- nijian
sgs.ai_skill_invoke["nijian"] = sgs.ai_skill_invoke["wuwei"]
sgs.ai_skill_cardask["@nijian"] = function(self, data)
	local allcards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(allcards, true)
	for _, card in ipairs(allcards) do
		if card:isRed() then
			return card:getEffectiveId()
		end
	end
	return "."
end

-- jiequan
sgs.ai_skill_playerchosen["jiequan"] = function(self, targets)
	local targetlist = sgs.QList2Table(targets)
	self:sort(targetlist)
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then
			return target
		end
	end
	return targetlist[1]
end

-- yinsi
sgs.ai_skill_choice["yinsi"] = function(self, choices, data)
	local target = data:toPlayer()
	if not target then return "cancel" end
	if self:isEnemy(target) and self.player:inMyAttackRange(target) then
		return "enemy"
	elseif self:isFriend(target) then
		return "friend"
	else
		return "cancel"
	end
end

-- weijiao
sgs.ai_skill_use["@@weijiao"] = function(self, prompt)
	self:sort(self.enemies, "handcard")

	local first_index, second_index
	for i=1, #self.enemies-1 do
		if self:hasSkills(sgs.need_kongcheng, self.enemies[i]) and self.enemies[i]:getHandcardNum() == 1 then
		elseif not self.enemies[i]:isKongcheng() then
			if not first_index then
				first_index = i
			elseif self.enemies[i]:getGender() ~= self.enemies[first_index]:getGender() then
				second_index = i
			end
		end
		if second_index then break end
	end

	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 1)) and
				self.enemies[first_index]:objectName() ~= other:objectName() and not other:isKongcheng() and
				self.enemies[first_index]:getGender() ~= other:getGender() then
				return ("@WeijiaoCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end
	if not second_index then return "." end
	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@WeijiaoCard=.->%s+%s"):format(first, second)
end

-- shiyi
sgs.ai_skill_invoke["shiyi"] = function(self, data)
	local r = math.random(0, 1)
	return r == 0
end

-- weixiao
local weixiao_skill={}
weixiao_skill.name = "weixiao"
table.insert(sgs.ai_skills, weixiao_skill)
weixiao_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("WeixiaoCard") or self.player:isNude() then return nil end
	local hasgit = false
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			hasgit = true
		end
	end
	if not hasgit then return end
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	local card = cards[1]
	for _, hcard in ipairs(cards) do
		if hcard:inherits("Slash") or hcard:inherits("Duel") then
			card = hcard
			break
		end
	end
	return sgs.Card_Parse("@WeixiaoCard=" .. card:getEffectiveId())
end
sgs.ai_skill_use_func["WeixiaoCard"]=function(card,use,self)
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			if use.to then use.to:append(friend) end
			use.card = card
			return
		end
	end
end

-- kuai&dianwan
sgs.ai_skill_invoke["kuai"] = true
sgs.ai_skill_invoke["dianwan"] = true

-- qianmian
sgs.ai_skill_choice["qianmian"] = function(self, choices)
	if self.player:getRole() == "rebel" then
		return "loyalist"
	elseif self.player:getRole() == "loyalist" then
		return "renegade"
	else
		return "cancel"
	end
end

-- shuangyu
sgs.ai_skill_choice["shuangyu"] = function(self, choices, data)
	local source = data:toPlayer()
	if source:isLord() then
		return "lord"
	elseif self:isFriend(source) and not self.player:isLord() then
		return self.player:getRole()
	end
	local r = math.random(1, 7)
	if r < 4 then
		return "loyalist"
	elseif r > 4 then
		return "rebel"
	else
		return "renegade"
	end
end

-- qingdi
sgs.ai_skill_invoke["qingdi"] = function(self, data)
	self:sort(self.enemies, "hp")
	if #self.enemies > 0 then
		self.qingditarget = self.enemies[1]
		return true
	end
	return false
end
sgs.ai_skill_playerchosen["qingdi"] = function(self, targets)
	return self.qingditarget
end

-- zhiyu
local zhiyu_skill={}
zhiyu_skill.name = "zhiyu"
table.insert(sgs.ai_skills, zhiyu_skill)
zhiyu_skill.getTurnUseCard = function(self)
	if self.player:isNude() or self.player:hasUsed("ZhiyuCard") then return nil end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			return sgs.Card_Parse("@ZhiyuCard=" .. card:getEffectiveId())
		end
	end
	return nil
end
sgs.ai_skill_use_func["ZhiyuCard"]=function(card,use,self)
	self:sort(self.friends, "defense")
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end
sgs.ai_skill_invoke["zhiyu"] = true

-- yanshi
sgs.ai_skill_invoke["yanshi"] = function(self, data)
	local damage = data:toDamageStar()
	if not damage then return true end
	local cards = damage.to:getHandcards()
	local shit_num = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits("Shit") then
			shit_num = shit_num + 1
			if card:getSuit() == sgs.Card_Spade then
				shit_num = shit_num + 1
			end
		end
	end
	if shit_num > 1 then return false end
	return true
end

-- dushu
sgs.ai_skill_cardask["@dushu"] = function(self, data)
	local player = data:toPlayer()
	if self:isEnemy(player) then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if fcard:inherits("Peach") then
				return fcard:getEffectiveId()
			end
		end
	end
	return "."
end

