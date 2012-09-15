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

--shuangxiong

sgs.ai_skill_invoke["shuangxiong"]=function(self,data)
	if self.player:isSkipped(sgs.Player_Play) or self.player:getHp() < 2 then
		return false
	end

	local cards=self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local handnum=0

	for _,card in ipairs(cards) do
		if self:getUseValue(card)<8 then
			handnum=handnum+1
		end
	end

	handnum=handnum/2
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if (self:getCardsNum("Slash", enemy)+enemy:getHp()<=handnum) and (self:getCardsNum("Slash")>=self:getCardsNum("Slash", enemy)) then return true end
	end

	return self.player:getHandcardNum()>=self.player:getHp()
end
