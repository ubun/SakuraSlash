sgs.ai_skill_playerchosen.zero_card_as_slash = function(self, targets)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist, "defense")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and not self:slashProhibit(slash ,target) and self:slashIsEffective(slash,target) then
			return target
		end
	end
	for i=#targetlist, 1, -1 do
		if not self:slashProhibit(slash, targetlist[i]) then
			return targetlist[i]
		end
	end
	return targets:first()
end

sgs.ai_skill_playerchosen.damage = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist,"hp")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then return target end
	end
	return targetlist[#targetlist]
end

sgs.ai_skill_invoke.ice_sword=function(self, data)
	if self.player:hasFlag("drank") then return false end
	local effect = data:toSlashEffect() 
	local target = effect.to
	if self:isFriend(target) then
		if self:isWeak(target) then return true
		elseif target:getLostHp()<1 then return false end
		return true
	else
		if self:isWeak(target) then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target)>3 then return true end
		local num = target:getHandcardNum()
		if self.player:hasSkill("tieji") or (self.player:hasSkill("liegong")
			and (num >= self.player:getHp() or num <= self.player:getAttackRange())) then return false end
		if target:hasSkill("tuntian") then return false end
		if self:hasSkills(sgs.need_kongcheng, target) then return false end
		if target:getCards("he"):length()<4 and target:getCards("he"):length()>1 then return true end
		return false
	end
end

local spear_skill={}
spear_skill.name="spear"
table.insert(sgs.ai_skills,spear_skill)
spear_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)

	if #cards<(self.player:getHp()+1) then return nil end
	if #cards<2 then return nil end
	if self:getCard("Slash") then return nil end

	self:sortByUseValue(cards,true)

	local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()
	
	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()

	local suit="no_suit"
	if cards[1]:isBlack() == cards[2]:isBlack() then suit = suit1 end

	local card_str = ("slash:spear[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)

	local slash = sgs.Card_Parse(card_str)

	return slash
	
end

sgs.ai_skill_cardask["@axe"] = function(self, data, pattern, target)
	if target and self:isFriend(target) then return "." end

	local allcards = self.player:getCards("he")
	allcards = sgs.QList2Table(allcards)
	if self.player:hasFlag("drank") or #allcards-2 >= self.player:getHp() or (self.player:hasSkill("kuanggu") and self.player:isWounded()) then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		local index
		if self:hasSkills(sgs.need_kongcheng) then index = #cards end
		if self.player:getOffensiveCar() then 
			if index then
				if index < 2 then
					index = index + 1
					table.insert(cards, self.player:getOffensiveCar()) 
				end
			end
			table.insert(cards, self.player:getOffensiveCar()) 
		end
		if self.player:getArmor() then
			if index then
				if index < 2 then
					index = index + 1
					table.insert(cards, self.player:getArmor())
				end
			end
			table.insert(cards, self.player:getArmor())
		end
		if self.player:getDefensiveCar() then 
			if index then
				if index < 2 then
					index = index + 1
					table.insert(cards, self.player:getDefensiveCar()) 
				end
			end
			table.insert(cards, self.player:getDefensiveCar()) 
		end
		if #cards >= 2 then
			self:sortByUseValue(cards, true)
			return "$"..cards[1]:getEffectiveId().."+"..cards[2]:getEffectiveId()
		end
	end
end

local qingnang_skill={}
qingnang_skill.name="qingnang"
table.insert(sgs.ai_skills,qingnang_skill)
qingnang_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum()<1 then return nil end
	if self.player:usedTimes("QingnangCard")>0 then return nil end
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	self:sortByKeepValue(cards)

	local card_str = ("@QingnangCard=%d"):format(cards[1]:getId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["QingnangCard"]=function(card,use,self)
	self:sort(self.friends, "defense")
	
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and
			not (friend:hasSkill("longhun") and self:getAllPeachNum() > 0) and
			not (friend:hasSkill("hunzi") and friend:getMark("hunzi") == 0 and self:getAllPeachNum() > 1) then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local rende_skill={}
rende_skill.name="rende"
table.insert(sgs.ai_skills, rende_skill)
rende_skill.getTurnUseCard=function(self)
	for _, player in ipairs(self.friends_noself) do
		if ((player:hasSkill("haoshi") and not player:containsTrick("supply_shortage"))
			or player:hasSkill("longluo")) and player:faceUp() and not self.player:isKongcheng() then
			return sgs.Card_Parse("@RendeCard=.")
		end
	end
	if (self.player:usedTimes("RendeCard") < 2 or self:getOverflow() > 0 or self:getCard("Shit")) and not self.player:isKongcheng() then 
		return sgs.Card_Parse("@RendeCard=.")
	end
end

sgs.ai_skill_use_func["RendeCard"] = function(card, use, self)
    if self.player:usedTimes("RendeCard") < 2 then
		local cards = self.player:getHandcards()
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHp() == 1 then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("Analeptic") or hcard:inherits("Peach") then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			if friend:hasSkill("paoxiao") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("Slash") then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			if friend:hasSkill("qingnang") and friend:getHp() < 2 and friend:getHandcardNum() < 1 then
				for _, hcard in sgs.qlist(cards) do
					if hcard:isRed() and not (hcard:inherits("ExNihilo") or hcard:inherits("Peach")) then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			if friend:hasSkill("jizhi") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getTypeId() == sgs.Card_Trick then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			if friend:hasSkill("guose") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getSuit() == sgs.Card_Diamond then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			if friend:hasSkill("leiji") or friend:hasSkill("jiuchi") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getSuit() == sgs.Card_Spade then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif friend:hasSkill("xiaoji") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("EquipCard") then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			
		end
	end
	
	local shit
	shit = self:getCard("Shit")
	if shit then
		use.card = sgs.Card_Parse("@RendeCard=" .. shit:getId())
		self:sort(self.enemies,"hp")
		if use.to then use.to:append(self.enemies[1]) end
		return
	end
	
	if self:getOverflow()>0 or (self.player:isWounded() and self.player:usedTimes("RendeCard") < 2 and not self.player:isKongcheng()) then 
		if #self.friends_noself == 0 then return end
		
		self:sort(self.friends_noself, "handcard")
		local friend
		for _, player in ipairs(self.friends_noself) do
			if (player:isKongcheng() and (player:hasSkill("kongcheng") or (player:hasSkill("zhiji") and not player:hasSkill("guanxing")))) or
				(not self:isWeak(player) and self:hasSkills(sgs.need_kongcheng,player)) then
			else friend = player break end
		end
		if not friend then return end
		local card_id = self:getCardRandomly(self.player, "h")
		if not sgs.Sanguosha:getCard(card_id):inherits("Shit") then
			use.card = sgs.Card_Parse("@RendeCard=" .. card_id)
			if use.to then use.to:append(friend) end
			return
		end
    end
	
end
