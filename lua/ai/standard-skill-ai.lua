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

local jieyin_skill={}
jieyin_skill.name="jieyin"
table.insert(sgs.ai_skills,jieyin_skill)
jieyin_skill.getTurnUseCard=function(self)
        if self.player:getHandcardNum()<2 then return nil end
        if self.player:hasUsed("JieyinCard") then return nil end
		
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		
		local first, second
		self:sortByUseValue(cards,true)
		for _, card in ipairs(cards) do
			if card:getTypeId() ~= sgs.Card_Equip then
				if not first then first  = cards[1]:getEffectiveId()
				else second = cards[2]:getEffectiveId()
				end
			end
			if second then break end
		end
		
		if not second then return end
		local card_str = ("@JieyinCard=%d+%d"):format(first, second)
		assert(card_str)
		return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["JieyinCard"]=function(card,use,self)
	self:sort(self.friends, "hp")
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
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
		if friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local kurou_skill={}
kurou_skill.name="kurou"
table.insert(sgs.ai_skills,kurou_skill)
kurou_skill.getTurnUseCard=function(self,inclusive)
        if  (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or		
		(self.player:getHp() - self.player:getHandcardNum() >= 2) then
                return sgs.Card_Parse("@KurouCard=.")
        end
		
		--if not inclusive then return nil end
		
	if self.player:getWeapon() and self.player:getWeapon():inherits("Crossbow") then
        for _, enemy in ipairs(self.enemies) do
            if self.player:canSlash(enemy,true) and self.player:getHp()>1 then
                return sgs.Card_Parse("@KurouCard=.")
            end
        end
    end
end

sgs.ai_skill_use_func["KurouCard"]=function(card,use,self)
	
	if not use.isDummy then self:speak("kurou") end
	
	use.card=card
end

local jijiang_skill={}
jijiang_skill.name="jijiang"
table.insert(sgs.ai_skills,jijiang_skill)
jijiang_skill.getTurnUseCard=function(self)
        if self.player:hasUsed("JijiangCard") or not self:slashIsAvailable() then return end
		local card_str = "@JijiangCard=."
		local slash = sgs.Card_Parse(card_str)
	    assert(slash)
        
        return slash
end

sgs.ai_skill_use_func["JijiangCard"]=function(card,use,self)
	self:sort(self.enemies, "defense")
		local target_count=0
                for _, enemy in ipairs(self.enemies) do
                        if ((self.player:canSlash(enemy, not no_distance)) or 
                        (use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange)))
                                 and
                                self:objectiveLevel(enemy)>3 and
                                self:slashIsEffective(card, enemy) then

                                use.card=card
                                if use.to then 
                                    use.to:append(enemy) 
                                end
                                target_count=target_count+1
                                if self.slash_targets<=target_count then return end
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

local zhiheng_skill={}
zhiheng_skill.name="zhiheng"
table.insert(sgs.ai_skills, zhiheng_skill)
zhiheng_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("ZhihengCard") then 
		return sgs.Card_Parse("@ZhihengCard=.")
	end
end

sgs.ai_skill_use_func["ZhihengCard"] = function(card, use, self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		for _, zcard in sgs.qlist(zcards) do
			if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
				table.insert(unpreferedCards,zcard:getId())
			end	
		end
	end
	
	if #unpreferedCards == 0 then 
		if self:getCardsNum("Slash")>1 then 
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end
		
		local num=self:getCardsNum("Jink")-1							
		if self.player:getArmor() then num=num+1 end
		if num>0 then
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and num>0 then 
					table.insert(unpreferedCards,card:getId())
					num=num-1
				end
			end
		end
        for _,card in ipairs(cards) do
            if card:inherits("EquipCard") then
                if card:inherits("Weapon") or
                (card:inherits("DefensiveCar") and self.player:getDefensiveCar()) or
                card:inherits("OffensiveCar") or
                (card:inherits("Armor") and self.player:getArmor()) or
                 card:inherits("AmazingGrace") or
                 card:inherits("Lightning") then
                    table.insert(unpreferedCards,card:getId())
                end
            end
        end
	
		if self.player:getWeapon() then														
			table.insert(unpreferedCards, self.player:getWeapon():getId())
		end
				
		if self:isEquip("SilverLion") and self.player:isWounded() then
			table.insert(unpreferedCards, self.player:getArmor():getId())
		end	
				
		local equips=self.player:getEquips()
		for _,equip in sgs.qlist(equips) do
			if equip:inherits("OffensiveCar") and self.player:getWeapon() then
				table.insert(unpreferedCards, equip:getId())
				break
			end
		end	
	end	
	
	if #unpreferedCards>0 then 
		use.card = sgs.Card_Parse("@ZhihengCard="..table.concat(unpreferedCards,"+")) 
		return 
	end
end

sgs.ai_skill_invoke["luoyi"]=function(self,data)
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)

    for _,card in ipairs(cards) do
        if card:inherits("Slash") then

            for _,enemy in ipairs(self.enemies) do
                if self.player:canSlash(enemy, true) and
                self:slashIsEffective(card, enemy) and
                ( (not enemy:getArmor()) or (enemy:getArmor():objectName()=="renwang_shield") or (enemy:getArmor():objectName()=="vine") ) and
                enemy:getHandcardNum()< 2 then							
                    if not self.player:containsTrick("indulgence") then
						self:speak("luoyi")
                        return true
                    end
                end
            end
        end
    end
    return false
end

