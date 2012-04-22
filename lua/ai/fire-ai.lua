-- this scripts contains the AI classes for generals of fire package

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

sgs.ai_skill_use["@@jieming"] = function(self, prompt)
	self:sort(self.friends)

	local max_x = 0
	local target
	for _, friend in ipairs(self.friends) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()

		if x > max_x then
			max_x = x
			target = friend
		end
	end

	if target then
		return "@JiemingCard=.->" .. target:objectName()
	else
		return "."
	end
end

-- mengjin
sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end

local qiangxi_skill={}
qiangxi_skill.name="qiangxi"
table.insert(sgs.ai_skills,qiangxi_skill)
qiangxi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QiangxiCard") then
		return sgs.Card_Parse("@QiangxiCard=.")
	end
end

sgs.ai_skill_use_func["QiangxiCard"] = function(card, use, self)
	local weapon = self.player:getWeapon()
	if weapon then
		local hand_weapon, cards
		cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Weapon") then
				hand_weapon = card
				break
			end
		end
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if hand_weapon and self.player:inMyAttackRange(enemy) then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				break
			end
			if self.player:distanceTo(enemy) <= 1 then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	else
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
				use.card = sgs.Card_Parse("@QiangxiCard=.")
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end

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
