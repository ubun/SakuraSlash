-- wuwei
sgs.ai_skill_invoke["wuwei"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.from)
end

-- rexue
sgs.ai_skill_invoke["rexue"] = function(self, data)
	local targets = {}
	for _, friend in ipairs(self.friends_noself) do
		if not friend:isNude() then
			table.insert(targets, friend)
		end
	end
	return #targets > 0
end
sgs.ai_skill_playerchosen["rexue"] = function(self, targets)
	self:sort(self.friends_noself, "hp")
	for _, friend in ipairs(self.friends_noself) do
		if not friend:isNude() and friend:getPile("rexue"):isEmpty() then
			return friend
		end
	end
	local lord = self.room:getLord()
	if not lord:isNude() and self:isFriend(lord) then
		return lord
	else
		for _, friend in ipairs(self.friends_noself) do
			if not friend:isNude() then
				return friend
			end
		end
	end
end
sgs.ai_skill_invoke["rexue_get"] = function(self, data)
	local n = self:getCardsNum("Peach") + self:getCardsNum("Analpetic")
	return n < 1
end

-- jiaojin
local jiaojin_skill={}
jiaojin_skill.name = "jiaojin"
table.insert(sgs.ai_skills, jiaojin_skill)
jiaojin_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum() > 0 and not self.player:hasUsed("JiaojinCard") then
		local hcard = self.player:getRandomHandCard()
		return sgs.Card_Parse("@JiaojinCard=" .. hcard:getId())
	end
end
sgs.ai_skill_use_func["JiaojinCard"] = function(card, use, self)
	local target
	for _, enemy in ipairs(self.enemies) do
		if enemy:getCards("he"):length() > 2 then
			target = enemy
			break
		end
	end
	if not target then target = self.Enemies[1] end
	if use.to then use.to:append(target) end
	use.card = card
	return
end

-- chenshui
sgs.ai_skill_invoke["chenshui"] = function(self, data)
	return self.player:getHandcardNum() > 3 or self.player:getHp() > 2
end
sgs.ai_skill_playerchosen["chenshuiprotect"] = function(self, targets)
	self:sort(self.friends_noself, "hp")
	return self.friends_noself[1]
end

-- jingxing
sgs.ai_skill_invoke["jingxing"] = true
sgs.ai_skill_playerchosen["jingxing"] = function(self, targets)
	return self.player
end

-- mazui
local mazui_skill={}
mazui_skill.name = "mazui"
table.insert(sgs.ai_skills, mazui_skill)
mazui_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("MazuiCard") then
		return sgs.Card_Parse("@MazuiCard=.")
	end
end
sgs.ai_skill_use_func["MazuiCard"] = function(card, use, self)
	self:sort(self.enemies, "handcard")
	if use.to then use.to:append(self.enemies[1]) end
	use.card = card
	return
end

-- fuyuan
sgs.ai_skill_invoke["fuyuan"] = true

-- pantao
sgs.ai_skill_invoke["pantao"] = function(self, data)
	return self.player:getHandcardNum() >= self.player:getHp() and self.player:getHandcardNum() > 1
end

-- shiyan
local shiyan_skill={}
shiyan_skill.name = "shiyan"
table.insert(sgs.ai_skills, shiyan_skill)
shiyan_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("ShiyanCard") or self.player:isKongcheng() then return end
	local hcard = self.player:getRandomHandCard()
	return sgs.Card_Parse("@ShiyanCard=" .. hcard:getId())
end
sgs.ai_skill_use_func["ShiyanCard"] = function(card, use, self)
	use.card = card
end

-- lanman
local lanman_skill={}
lanman_skill.name = "lanman"
table.insert(sgs.ai_skills, lanman_skill)
lanman_skill.getTurnUseCard=function(self,inclusive)
	if self.player:getHandcardNum() >= 4 then return end
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Diamond or inclusive then
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("ex_nihilo:lanman[diamond:%s]=%d"):format(number, card_id)
			local exnihilo = sgs.Card_Parse(card_str)
			assert(exnihilo)
			return exnihilo
		end
	end
end

-- duoren
sgs.ai_skill_invoke["duoren"] = function(self, data)
	return self:isEnemy(data:toPlayer())
end

-- shouhou
sgs.ai_skill_invoke["shouhou"] = function(self, data)
	self:sort(self.friends, "hp")
	return self.friends[1]:isWounded() and self.player:getHandcardNum() >= 3
end
sgs.ai_skill_playerchosen["shouhou"] = function(self, targets)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			return friend
		end
	end
end

-- heqi
sgs.ai_skill_invoke["heqi"] = function(self, data)
	local effect = data:toCardEffect()
	if self:isFriend(effect.from) then
		if effect.from:containsTrick("supplyshortage") or effect.from:containsTrick("indulgence") then
			return true
		end
	end
	return self:isEnemy(effect.from)
end

-- shouqiu
sgs.ai_skill_invoke["@shouqiu"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@ShouqiuCard=" .. card_id
		end
	end

	return "."
end

-- shenyong
local shenyong_skill={}
shenyong_skill.name = "shenyong"
table.insert(sgs.ai_skills,shenyong_skill)
shenyong_skill.getTurnUseCard = function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	local final_card

	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards)  do
		if card:inherits("EquipCard") and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
			final_card = card
			break
		end
	end

	if final_card then
		local suit = final_card:getSuitString()
		local number = final_card:getNumberString()
		local card_id = final_card:getEffectiveId()
		local card_str = ("slash:shenyong[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)
        return slash
	end
end

-- shentou
local shentou_skill={}
shentou_skill.name = "shentou"
table.insert(sgs.ai_skills, shentou_skill)
shentou_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:getTypeId() == sgs.Card_Trick or inclusive then
		    local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("snatch:shentou[%s:%s]=%d"):format(suit, number, card_id)
			local snatch = sgs.Card_Parse(card_str)
			assert(snatch)
			return snatch
		end
	end
end
sgs.ai_skill_invoke["shentou"] = true

-- baiyi
local baiyi_skill={}
baiyi_skill.name = "baiyi"
table.insert(sgs.ai_skills, baiyi_skill)
baiyi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("BaiyiCard") then
		if self.player:getEquips():length() == 1 then
			local equip = self.player:getEquips():first()
			return sgs.Card_Parse("@BaiyiCard=" .. equip:getEffectiveId())
		end
		return nil
	end
end
sgs.ai_skill_use_func["BaiyiCard"] = function(card, use, self)
	if use.to then use.to:append(self.friends_noself[1]) end
	use.card = card
	return
end

-- yirong&tishen
sgs.ai_skill_invoke["yirong"] = true
sgs.ai_skill_invoke["tishen"] = true
sgs.ai_skill_playerchosen["tishen"] = function(self, targets)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			return enemy
		end
	end
	return targets[1]
end

-- shangchi
sgs.ai_skill_choice["shangchi"] = function(self, choices)
	if self.player:getHp() - 1 > self.player:getLostHp() and #self.friends_noself > 0 then
		return "him"
	else
		return "me"
	end
end
sgs.ai_skill_playerchosen["shangchi"] = function(self, targets)
	return self.friends_noself[1]
end

-- diaobing
local diaobing_skill={}
diaobing_skill.name = "diaobing"
table.insert(sgs.ai_skills, diaobing_skill)
diaobing_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("DiaobingCard") then
		return sgs.Card_Parse("@DiaobingCard=.")
	end
end
sgs.ai_skill_use_func["DiaobingCard"] = function(card, use, self)
	self:sort(self.enemies, "hp")
	if use.to then use.to:append(self.enemies[1]) end
	use.card = card
	return
end

-- guilin
sgs.ai_skill_invoke["guilin"] = true

-- moshu
sgs.ai_skill_invoke["moshu"] = function(self, data)
	local player = data:toPlayer()
	if self:isEnemy(player) then
		return true
	end
	return false
end

-- renxing
local renxing_skill={}
renxing_skill.name = "renxing"
table.insert(sgs.ai_skills, renxing_skill)
renxing_skill.getTurnUseCard=function(self)
    if not self.player:hasUsed("RenxingCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		return sgs.Card_Parse("@RenxingCard=" .. max_card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["RenxingCard"]=function(card,use,self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() >= self.player:getHp() and not enemy:isKongcheng() then
		    if use.to then use.to:append(enemy) end
            use.card=card
            break
		end
	end
end

-- heiyi
sgs.ai_skill_invoke["heiyi"] = function(self, data)
	if self.player:getHandcardNum() > 3 and self:isFriend(data:toPlayer()) then
		return true
	end
	return false
end

-- dashou
sgs.ai_skill_invoke["dashou"] = true

-- xunzhi
sgs.ai_skill_playerchosen["xunzhi"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then
			return player
		end
	end
	return
end

-- gaizao
sgs.ai_skill_use["@@gaizao"] = function(self, prompt)
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if not player:getCards("e"):isEmpty() then
			if self:isEnemy(player) and	not self:hasSkills(sgs.lose_equip_skill, player) then
				return "@GaizaoCard=.->" .. player:objectName()
			elseif self:isFriend(player) and self:hasSkills(sgs.lose_equip_skill, player) then
				return "@GaizaoCard=.->" .. player:objectName()
			end
		end
	end
	if not self.player:getCards("e"):isEmpty() then
		return "@GaizaoCard=.->" .. self.player:objectName()
	else
		return "."
	end
end
sgs.ai_skill_cardchosen["gaizao"] = function(self, who)
	local ecards = who:getCards("e")
	ecards = sgs.QList2Table(ecards)
	return ecards[1]
end

-- baomu
sgs.ai_skill_invoke["baomu"] = function(self, data)
	local who = data:toPlayer()
	return self:isFriend(who)
end

-- qiniao
sgs.ai_skill_invoke["qiniao"] = true
sgs.ai_skill_playerchosen["qiniao"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) and not player:hasSkill("weijiao") and
			(player:containsTrick("indulgence") or player:containsTrick("supply_shortage")) then
			return player
		elseif self:isEnemy(player) and player:hasSkill("weijiao") then
			return player
		end
	end
	return self.friends[1]
end

-- long
sgs.ai_skill_invoke["long"] = function(self, data)
	local r = math.random(0, 1)
	return r == 0
end


