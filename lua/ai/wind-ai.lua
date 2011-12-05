-- bianhu
sgs.ai_skill_invoke["bianhu"] = function(self, data)
	local use = data:toCardUse()
	if self:isEnemy(use.from) and use.card:inherits("ExNihilo") then
		return true
	end
--	if self:isFriend(use.to) and use.card:inherits("Dismantlement") then
--		return true
--	end
	return false
end
sgs.ai_skill_playerchosen["bianhu"] = function(self, targets)
	return self.player
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
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getGeneral():isMale() then
			return "@HuachiCard=.->" .. enemy:objectName()
		end
	end
	return "."
end

-- ouxiang
sgs.ai_skill_invoke["ouxiang"] = function(self, data)
	return self.player:getHandcardNum() < 10
end

-- lingjia
sgs.ai_skill_playerchosen["lingjia"] = sgs.ai_skill_playerchosen["fenju"]

-- yinsi
sgs.ai_skill_choice["yinsi"] = function(self, choices)
	local target = self.room:getTag("YinsTarget"):toPlayer()
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
			else
				second_index = i
			end
		end
		if second_index then break end
	end

	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 1)) and
				self.enemies[first_index]:objectName() ~= other:objectName() and not other:isKongcheng() then
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

-- qingdi
sgs.ai_skill_invoke["qingdi"] = true
sgs.ai_skill_playerchosen["qingdi"] = function(self, targets)
	self:sort(self.enemies, "hp")
	local target = self.enemies[1]
	if not target then target = targets[1] end
	return target
end

-- zhiyu
local zhiyu_skill={}
zhiyu_skill.name = "zhiyu"
table.insert(sgs.ai_skills, zhiyu_skill)
zhiyu_skill.getTurnUseCard = function(self)
	if self.player:isNude() or self.player:hasUsed("ZhiyuCard") then return nil end
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			local card_str = ("@ZhiyuCard=%d"):format(card:getId())
			return sgs.Card_Parse(card_str)
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

-- liegong, same with tieji
sgs.ai_skill_invoke.liegong = sgs.ai_skill_invoke.tieji

--leiji
sgs.ai_skill_use["@@leiji"]=function(self,prompt)
    self:updatePlayers()
	self:sort(self.enemies,"hp")
	for _,enemy in ipairs(self.enemies) do
		if not self:isEquip("SilverLion", enemy) and not enemy:hasSkill("hongyan") then
			return "@LeijiCard=.->"..enemy:objectName()
		end
	end
	return "."
end

--shensu

sgs.ai_skill_use["@@shensu1"]=function(self,prompt)
        self:updatePlayers(true)
	self:sort(self.enemies,"defense")

	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = getDefense(self.player)
	local hasJud = self.player:getJudgingArea()

	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))

                if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
                elseif self:slashProhibit(nil, enemy) then
                elseif def<6 and eff then return "@ShensuCard=.->"..enemy:objectName()

                elseif selfSub>=2 then return "."
                elseif selfDef<6 then return "." end

	end

	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))

                if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
                elseif self:slashProhibit(nil, enemy) then
                elseif eff and def<8 then return "@ShensuCard=.->"..enemy:objectName()
		else return "." end
	end
	return "."
end

sgs.ai_get_cardType=function(card)
	if card:inherits("Weapon") then return 1 end
	if card:inherits("Armor") then return 2 end
	if card:inherits("OffensiveCar")then return 3 end
	if card:inherits("DefensiveCar") then return 4 end
end

sgs.ai_skill_use["@@shensu2"]=function(self,prompt)
        self:updatePlayers(true)
	self:sort(self.enemies,"defense")

	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = getDefense(self.player)

	local cards = self.player:getCards("he")

	cards=sgs.QList2Table(cards)

	local eCard
	local hasCard={0, 0, 0, 0}

	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then
			hasCard[sgs.ai_get_cardType(card)] = hasCard[sgs.ai_get_cardType(card)]+1
		end
	end

	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then
			if hasCard[sgs.ai_get_cardType(card)]>1 or sgs.ai_get_cardType(card)>3 then
				eCard = card
				break
			end
			if not eCard and not card:inherits("Armor") then eCard = card end
		end
	end

	if not eCard then return "." end

	local effectslash, best_target, target
	local defense = 6
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram") or enemy:hasSkill("bazhen"))

        if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
        elseif self:slashProhibit(nil, enemy) then
        elseif eff then
			if enemy:getHp() == 1 and self:getCardsNum("Jink", enemy) == 0 then best_target = enemy break end
			if def < defense then
				best_target = enemy
				defense = def
			end
			target = enemy
		end
		if selfSub<0 then return "." end
	end

	if best_target then return "@ShensuCard="..eCard:getEffectiveId().."->"..best_target:objectName() end
	if target then return "@ShensuCard="..eCard:getEffectiveId().."->"..target:objectName() end

	return "."
end

local huangtianv_skill={}
huangtianv_skill.name="huangtianv"
table.insert(sgs.ai_skills,huangtianv_skill)

huangtianv_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("HuangtianCard") then return nil end
    if self.player:isLord() then return nil end
    if self.player:getKingdom() ~= "qun" then return nil end
	if not self.room:getLord():hasSkill("huangtian") then return nil end

    local cards = self.player:getCards("h")
    cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if acard:inherits("Jink") then
			card = acard
			break
		end
	end

	if not card then
		return nil
	end

	local card_id = card:getEffectiveId()
	local card_str = "@HuangtianCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)
	return skillcard
end

sgs.ai_skill_use_func["HuangtianCard"]=function(card,use,self)
    local targets = {}
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasLordSkill("Huangtian") then
			table.insert(targets, friend)
		end
	end

	if #targets == 0 then return end

	use.card=card
	self:sort(targets, "defense")
	if use.to then
		use.to:append(targets[1])
    end
end

sgs.ai_skill_askforag.buqu = function(self, card_ids)
-- find duplicated one or the first
	for i, card_id in ipairs(card_ids) do
		for j, card_id2 in sgs.list(card_ids) do
			if i ~= j and card_id == card_id2 then
				return card_id
			end
		end
	end

	return card_ids[1]
end