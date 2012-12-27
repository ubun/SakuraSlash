-- this scripts contains the AI classes for generals of fire package

-- xianv
sgs.ai_skill_cardask["@xianv"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		local allcards = self.player:getCards("he")
		for _, card in sgs.qlist(allcards) do
			if card:inherits("EquipCard") then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- jiaoxie
sgs.ai_skill_invoke["jiaoxie"] = true

-- shentou (anshitou's skill)
local ientou_skill = {}
ientou_skill.name = "ientou"
table.insert(sgs.ai_skills,ientou_skill)
ientou_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("IentouCard") and not self.player:isKongcheng() then
		local fri, ene
		local x
		self:sort(self.friends, "hp")
		for _, friend in ipairs(self.friends) do
			if not friend:isLord() then
				fri = friend
				break
			end
		end
		self:sort(self.enemies, "hp2")
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isLord() then
				x = enemy:getHp() - fri:getHp()
				if x < self.player:getHandcardNum() then
					ene = enemy
					break
				end
			end
		end
		if fri and ene then
			local cards = self.player:getCards("h")
			cards=sgs.QList2Table(cards)
			self:sortByUseValue(cards, true)
			local card_ids = {}
			for i = 1, x do
				table.insert(card_ids, cards[i]:getEffectiveId())
			end
			self.ientoua = fri
			self.ientoub = ene
			return sgs.Card_Parse("@IentouCard=" .. table.concat(card_ids, "+"))
		end
	end
end
sgs.ai_skill_use_func["IentouCard"] = function(card, use, self)
	if use.to then
		use.to:append(self.ientoua)
		use.to:append(self.ientoub)
	end
    use.card = card
end

-- fangxin
sgs.ai_card_intention.FangxinCard = sgs.ai_card_intention.Slash

sgs.ai_skill_invoke["fangxin"] = true
local fangxin_skill = {}
fangxin_skill.name = "fangxin"
table.insert(sgs.ai_skills, fangxin_skill)
fangxin_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("Fangxin") or not self:slashIsAvailable() then return end
	local card_str = "@FangxinCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_skill_use_func["FangxinCard"] = function(card,use,self)
	self:sort(self.enemies, "defense")
	local target_count=0
	for _, enemy in ipairs(self.enemies) do
		if ((self.player:canSlash(enemy, not no_distance)) or
			(use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange))) and
			self:objectiveLevel(enemy)>3 and
			self:slashIsEffective(card, enemy) and
			not self:slashProhibit(card, enemy) then
			local cheat_card = sgs.Sanguosha:getCard(self.room:getDrawPile():first())
			if cheat_card:getSuit() == sgs.Card_Heart and not self.player:hasUsed("MoguaCard") and not self.player:isKongcheng() then
				local cards = sgs.QList2Table(self.player:getCards("h"))
				self:sortByUseValue(cards, true)
				for _, car in ipairs(cards) do
					if car:getSuit() ~= sgs.Card_Heart then
						use.card = sgs.Card_Parse("@MoguaCard=" .. car:getId())
						return
					end
				end
			end
			use.card=card
			if use.to then
				use.to:append(enemy)
			end
			target_count=target_count+1
			if self.slash_targets<=target_count then return end
		end
	end
end
--function sgs.ai_cardneed.fangxin(to, card, self)
--	return card:isBlack()
--end

-- yiben
sgs.ai_skill_invoke["yiben"] = true
sgs.ai_skill_playerchosen["yiben"] = sgs.ai_skill_playerchosen["jiequan"]

-- xiebi
sgs.ai_skill_invoke["xiebi"]=function(self,data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- chuyin
sgs.ai_skill_invoke["chuyin"] = true
--[[sgs.ai_skill_discard["chuyin"] = function(self, discard_num, optional, include_equip)
	local to_discard = {}
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByKeepValue(cards, true)
	for _, card in ipairs(cards) do
		if #to_discard >= discard_num then break end
		table.insert(to_discard, card:getId())
	end
	if #to_discard == discard_num then
		return to_discard
	else
		return {}
	end
end]]

-- shanliang
sgs.ai_skill_invoke["shanliang"]=function(self,data)
	local damage = data:toDamage()
	return self.player:getHp() > 2 and self:isFriend(damage.to)
end

