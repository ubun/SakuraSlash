-- wuwei
sgs.ai_skill_invoke["wuwei"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.from)
end

-- rexue
sgs.ai_skill_invoke["rexue"] = function(self, data)
	return #self.friends_noself > 0
end
sgs.ai_skill_playerchosen["rexue"] = function(self, targets)
	self:sort(self.friends_noself, "hp")
	return self.friends_noself[1]
end
sgs.ai_skill_invoke["rexue_get"] = function(self, data)
	local n = self:getCardsNum("Slash") + self:getCardsNum("Analpetic")
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
sgs.ai_skill_invoke["shenyong"] = function(self, data)
	local damage = data:toDamage()
	local players = sgs.QList2Table(self.room:getOtherPlayers(damage.to))
	for _, player in ipairs(players) do
		if self:isEnemy(player) and damage.to:canSlash(player) and not player:hasFlag("shenyong") then
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["shenyong"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then
			return player
		end
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

-- shangchi
sgs.ai_skill_choice["shangchi"] = function(self, choices)
	return "me"
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

-- moshu
sgs.ai_skill_choice["moshu"] = function(self, choices)
	local player = self.room:getCurrent()
	if self:isEnemy(player) then return "one" end
	if self:isFriend(player) then return "zero" end
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

-- juelu
sgs.ai_skill_invoke["juelu"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
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

-- baomu
sgs.ai_skill_invoke["baomu"] = function(self, data)
	local who = data:toPlayer()
	return self:isFriend(who)
end

-- qiniao
sgs.ai_skill_invoke["qiniao"] = true
sgs.ai_skill_playerchosen["qiniao"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) and (player:containsTrick("indulgence") or player:containsTrick("supply_shortage")) then
			return player
		end
	end
	return self.friends[1]
end

-- long
sgs.ai_skill_invoke["long"] = true



-- $$$$$$$$$$$$$###############
-- jianxiong
sgs.ai_skill_invoke.jianxiong = function(self, data)
		return not sgs.Shit_HasShit(data:toCard())
end

sgs.ai_skill_invoke.jijiang = function(self, data)
	if self:getCardsNum("Slash")<=0 then
		return true
	end
	return false
end

sgs.ai_skill_choice.jijiang = function(self , choices)
	if not self.player:hasLordSkill("jijiang") then
		if self:getCardsNum("Slash") <= 0 then return "ignore" end
	end

	if self.player:isLord() then
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasSkill("weidi") then
				target = player
				break
			end
		end
		if target and self:isEnemy(target) then return "ignore" end
	elseif self:isFriend(self.room:getLord()) then return "accept" end
	return "ignore"
end

sgs.ai_skill_choice.hujia = function(self , choices)
	if not self.player:hasLordSkill("hujia") then
		if self:getCardsNum("Jink") <= 0 then return "ignore" end
	end
	if self.player:isLord() then
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasSkill("weidi") then
				target = player
				break
			end
		end
		if target and self:isEnemy(target) then return "ignore" end
	elseif self:isFriend(self.room:getLord()) then return "accept" end
	return "ignore"
end

-- hujia
sgs.ai_skill_invoke.hujia = function(self, data)
	local cards = self.player:getHandcards()
	for _, friend in ipairs(self.friends_noself) do
		if friend:getKingdom() == "wei" and self:isEquip("EightDiagram", friend) then return true end
	end
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			return false
		end
	end
	return true
end

-- tuxi
sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
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
				return ("@TuxiCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end

	if not second_index then return "." end

	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end

-- yiji (frequent)
sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data)
	return not self:isFriend(data:toPlayer())
end

-- fankui
sgs.ai_skill_invoke.fankui = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		return (target:hasSkill("xiaoji") and not target:getEquips():isEmpty()) or (self:isEquip("SilverLion",target) and target:isWounded())
	end
	if self:isEnemy(target) then				---fankui without zhugeliang and luxun
			if (target:hasSkill("kongcheng") or target:hasSkill("lianying")) and target:getHandcardNum() == 1 then
				if not target:getEquips():isEmpty() then return true
				else return false
				end
			end
	end
				--self:updateRoyalty(-0.8*sgs.ai_royalty[target:objectName()],self.player:objectName())
	return true
end

-- tieji
sgs.ai_skill_invoke.tieji = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) and (not effect.to:isKongcheng() or effect.to:getArmor())
end

sgs.ai_skill_use["@@liuli"] = function(self, prompt)

	local others=self.room:getOtherPlayers(self.player)
	others=sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do
		if player:hasFlag("slash_source") then
			source = player
			 break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy,true) and not (source:objectName() == enemy:objectName()) then
			local cards = self.player:getCards("he")
			cards=sgs.QList2Table(cards)
			for _,card in ipairs(cards) do
				if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(enemy)>1 then local bullshit
				elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(enemy)
					and self.player:distanceTo(enemy)>1 then
					local bullshit
				else
					return "@LiuliCard="..card:getEffectiveId().."->"..enemy:objectName()
				end
			end
		end
	end
	return "."
end

