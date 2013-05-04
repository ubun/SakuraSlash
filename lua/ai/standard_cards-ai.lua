local function hasExplicitRebel(room)
	for _, player in sgs.qlist(room:getAllPlayers()) do
		if sgs.ai_explicit[player:objectName()] and sgs.ai_explicit[player:objectName()]:match("rebel") then return true end
	end
	return false
end

function SmartAI:slashProhibit(card,enemy)
	if card == nil then
		card = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	end

	if self:isFriend(enemy) then
		if card:inherits("FireSlash") or self.player:hasWeapon("fan") then
			if self:isEquip("Vine", enemy) then return true end
		end
		if enemy:isChained() and card:inherits("NatureSlash") and #(self:getChainedFriends())>1 and
			self:slashIsEffective(card,enemy) then return true end
		if self:getCardsNum("Jink",enemy) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card,enemy) then return true end
		if enemy:isLord() and self:isWeak(enemy) and self:slashIsEffective(card,enemy) then return true end
		if (enemy:hasSkill("duanchang") or enemy:hasSkill("huilei") or enemy:hasSkill("dushi")) and self:isWeak(enemy) then return true end
		if self:isEquip("GudingBlade") and enemy:isKongcheng() then return true end
	else
		if enemy:hasSkill("liuli") then
			if enemy:getHandcardNum() < 1 then return false end
			for _, friend in ipairs(self.friends_noself) do
				if enemy:canSlash(friend,true) and self:slashIsEffective(card, friend) then return true end
			end
		end

		if enemy:hasSkill("leiji") then
			local hcard = enemy:getHandcardNum()
			if self.player:hasSkill("tieji") or
				(self.player:hasSkill("liegong") and (hcard>=self.player:getHp() or hcard<=self.player:getAttackRange())) then return false end

			if enemy:getHandcardNum() >= 2 then return true end
			if self:isEquip("EightDiagram", enemy) then
				local equips = enemy:getEquips()
				for _,equip in sgs.qlist(equips) do
					if equip:getSuitString() == "spade" then return true end
				end
			end
		end

		if enemy:hasSkill("tiandu") then
			if self:isEquip("EightDiagram", enemy) then return true end
		end

		if enemy:hasSkill("ganglie") then
			if self.player:getHandcardNum()+self.player:getHp() < 5 then return true end
		end


		if enemy:isChained() and #(self:getChainedFriends()) > #(self:getChainedEnemies()) and self:slashIsEffective(card,enemy) then
			return true
		end
		
		if enemy:hasSkill("wuhun") and self:isWeak(enemy) then
			local mark = 0
			for _, player in sgs.qlist(self.room:getAlivePlayers()) do
				if player:getMark("@nightmare") > mark then mark = player:getMark("@nightmare") end
			end
			if mark > 0 then
				for _,friend in ipairs(self.friends) do
					if friend:getMark("@nightmare") == mark and (not self:isWeak(friend) or friend:isLord()) and
						not (#self.enemies==1 and #self.friends + #self.enemies == self.room:alivePlayerCount()) then return true end
				end
				if self.player:getRole()~="rebel" and self.room:getLord():getMark("@nightmare") == mark and
					not (#self.enemies==1 and #self.friends + #self.enemies == self.room:alivePlayerCount()) then return true end
			end
		end

		if enemy:hasSkill("duanchang") and #self.enemies>1 and self:isWeak(enemy) and (self.player:isLord() or not self:isWeak()) then
			return true
		end

		if enemy:hasSkill("huilei") and #self.enemies>1 and self:isWeak(enemy) and
			(self.player:getHandcardNum()>3 or self:getCardsNum("Shit")>0) then
			return true
		end
	end

	return not self:slashIsEffective(card, enemy)
end

function SmartAI:slashIsEffective(slash, to)
	if to:hasSkill("yizhong") and not to:getArmor() then
		if slash:isBlack() then
			return false
		end
	end
	if to:hasSkill("huachi") then
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)
		for _, player in ipairs(players) do
			if player:getMark("@flower") > 0 and self:isFriend(player) then
				return false
			end
		end
	end
	
	local nature = {
		Slash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}

	if not self:damageIsEffective(to, nature[slash:className()]) then return false end

	if self.player:hasWeapon("qinggang_sword") or (self.player:hasFlag("xianzhen_success") and self.room:getTag("XianzhenTarget"):toPlayer() == to) then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine" then
			return slash:inherits("NatureSlash") or self.player:hasWeapon("fan")
		end
	end

	return true
end

function SmartAI:slashIsAvailable(player)
	player = player or self.player
	if player:hasFlag("tianyi_failed") or player:hasFlag("xianzhen_failed") then return false end

	if player:hasWeapon("crossbow") or player:hasSkill("paoxiao") then
		return true
	end

	if player:hasFlag("tianyi_success") then
		return (player:usedTimes("Slash") + player:usedTimes("FireSlash") + player:usedTimes("ThunderSlash")) < 2
	else
		return (player:usedTimes("Slash") + player:usedTimes("FireSlash") + player:usedTimes("ThunderSlash")) < 1
	end
end

function SmartAI:useCardSlash(card, use)
	if card:getSkillName() == "wushen" then no_distance = true end
	if (self.player:getHandcardNum() == 1
	and self.player:getHandcards():first():inherits("Slash")
	and self.player:getWeapon()
	and self.player:getWeapon():inherits("Halberd"))
	or (self.player:hasSkill("shenji") and not self.player:getWeapon()) then
		self.slash_targets = 3
	elseif self.player:hasSkill("shenyong") and self.player:getPhase() == sgs.Player_Play then
		self.slash_targets = 2
	end

	self.predictedRange = self.player:getAttackRange()
	if self:slashIsAvailable() then
		local target_count = 0
		if card:isBlack() and self.room:getTag("Zhenwu"):toString() == "slash" then return end
		if self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return end
		for _, friend in ipairs(self.friends_noself) do						
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card,friend)
			if (self.player:hasSkill("pojun") and friend:getHp() > 4 and self:getCardsNum("Jink", friend) == 0
				and friend:getHandcardNum() < 3)
			or (friend:hasSkill("leiji") 
			and (self:getCardsNum("Jink", friend) > 0 or (not self:isWeak(friend) and self:isEquip("EightDiagram",friend)))
			and (hasExplicitRebel(self.room) or not friend:isLord()))
			or (friend:isLord() and self.player:hasSkill("guagu") and friend:getLostHp() >= 1 and self:getCardsNum("Jink", friend) == 0)
			then
				if not slash_prohibit then
					if ((self.player:canSlash(friend, not no_distance)) or
						(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
						self:slashIsEffective(card, friend) then
						use.card = card
						if use.to then
							use.to:append(friend)
							self:speak("hostile", self.player:getGeneral():isFemale())
						end
						target_count = target_count+1
						if self.slash_targets <= target_count then return end
					end
				end
--				break
			end
		end

		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card,enemy)
			if not slash_prohibit then
				if ((self.player:canSlash(enemy, not no_distance)) or
				(use.isDummy and self.predictedRange and (self.player:distanceTo(enemy) <= self.predictedRange))) and
				self:objectiveLevel(enemy) > 3 and
				self:slashIsEffective(card, enemy) then
					-- fill the card use struct
					if not use.to or use.to:isEmpty() then
						local anal = self:searchForAnaleptic(use,enemy,card)
						if anal and not self:isEquip("SilverLion", enemy) and not self:isWeak() then
							use.card = anal
							return
						end
						if self.player:getGender()~=enemy:getGender() and self:getCardsNum("DoubleSword",self.player,"h") > 0 then
							self:useEquipCard(self:getCard("DoubleSword"), use)
							if use.card then return end
						end
						if enemy:isKongcheng() and self:getCardsNum("GudingBlade", self.player, "h") > 0 then
							self:useEquipCard(self:getCard("GudingBlade"), use)
							if use.card then return end
						end
						if self:getOverflow()>0 and self:getCardsNum("Axe", self.player, "h") > 0 then
							self:useEquipCard(self:getCard("Axe"), use)
							if use.card then return end
						end
						if enemy:getArmor() and self:getCardsNum("Fan", self.player, "h") > 0 and
							(enemy:getArmor():inherits("Vine") or enemy:getArmor():inherits("GaleShell")) then
							self:useEquipCard(self:getCard("Fan"), use)
							if use.card then return end
						end
						if enemy:getDefensiveCar() and self:getCardsNum("KylinBow", self.player, "h") > 0 then
							self:useEquipCard(self:getCard("KylinBow") ,use)
							if use.card then return end
						end
					end
					use.card = card
					if use.to then use.to:append(enemy) end
					target_count = target_count+1
					if self.slash_targets <= target_count then return end
				end
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkill("yiji") and friend:getLostHp() < 1 and
				not (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				local slash_prohibit = false
				slash_prohibit = self:slashProhibit(card, friend)
				if not slash_prohibit then
					if ((self.player:canSlash(friend, not no_distance)) or
						(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
						self:slashIsEffective(card, friend) then
						use.card = card
						if use.to then
							use.to:append(friend)
							self:speak("yiji")
						end
						target_count = target_count+1
						if self.slash_targets <= target_count then return end
					end
				end
				break
			end
		end
	end
end

function SmartAI:useCardPeach(card, use)
	if self.player:isWounded() then
		if not (self.player:hasSkill("rende") and self:getOverflow() > 1 and #self.friends_noself > 0) then
			local peaches = 0
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Peach") then peaches = peaches+1 end
			end

			for _, friend in ipairs(self.friends_noself) do
				if (self.player:getHp()-friend:getHp() > peaches) and (friend:getHp() < 3) and not friend:hasSkill("buqu") then return end
			end

			if self.player:hasSkill("canwu") and self:getOverflow() > 0 then
				self:sort(self.friends, "hp")
				for _, friend in ipairs(self.friends) do
					if friend:isWounded() and friend:getGeneral():isMale() then return end
				end
			end

			use.card = card
		end
	end
end

-- yicai,badao,yitian-slash,moon-spear-slash
sgs.ai_skill_use["slash"] = function(self, prompt)
	if prompt ~= "@askforslash" and prompt ~= "@moon-spear-slash" then return "." end
	local slash = self:getCard("Slash")
	if not slash then return "." end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, true) and not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) then
			return ("%s->%s"):format(slash:toString(), enemy:objectName())
		end
	end
	return "."
end

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies or (self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1)
		or self.player:hasSkill("jizhi") then
		use.card = card
	elseif self.player:hasSkill("wuyan") then
		use.card = card
	end
end

function SmartAI:useCardGodSalvation(card, use)
	local good, bad = 0, 0

	if self.player:hasSkill("wuyan") then 						
		use.card = card
		return
	end

	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10/(friend:getHp())
			if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10/(enemy:getHp())
			if enemy:isLord() then
				bad = bad + 10/(enemy:getHp())
			end
		end
	end

	if good > bad then
		use.card = card
	end
end

local function factorial(n)
	if n <= 0.1 then return 1 end
	return n*factorial(n-1)
end

function SmartAI:useCardDuel(duel, use)
	if self.player:hasSkill("wuyan") or self.player:hasSkill("tianzhen") then return end
	self:sort(self.enemies,"handcard")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		if self:objectiveLevel(enemy) > 3 then
			local n1 = self:getCardsNum("Slash")
			local n2 = enemy:getHandcardNum()
			if enemy:hasSkill("wushuang") then n2 = n2*2 end
			if self.player:hasSkill("wushuang") then n1 = n1*2 end
			local useduel
			if self:hasTrickEffective(duel, enemy) then
				if n1 >= n2 then
					useduel = true
				elseif n2 > n1*2 + 1 then
					useduel = false
				elseif n1 > 0 then
					local percard = 0.35
					if enemy:hasSkill("paoxiao") or enemy:hasWeapon("crossbow") then percard = 0.2 end
					local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
					if math.random() > poss then useduel = true end
				end
				if useduel then
					use.card = duel
					if use.to then
						use.to:append(enemy)
						self:speak("duel", self.player:getGeneral():isFemale())
					end
					return
				end
			end
		end
	end
end

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardSupplyShortage(card, use)
	table.sort(self.enemies, handcard_subtract_hp)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if ((#enemies == 1) or not enemy:hasSkill("tiandu")) and not enemy:containsTrick("supply_shortage") then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardIndulgence(card, use)
	table.sort(self.enemies, hp_subtract_handcard)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("keji") then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

-- the ExNihilo is always used
function SmartAI:useCardExNihilo(card, use)
		use.card = card
		if not use.isDummy then
			self:speak("lucky")
		end
end

-- when self has wizard (zhangjiao, simayi, use it)
function SmartAI:useCardLightning(card, use)
	if self.player:containsTrick("lightning") then return end
	if self.player:hasSkill("weimu") and card:isBlack() then return end

	if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
		if self:hasWizard(self.friends) then
			use.card = card
			return
		end
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)

		local friends = 0
		local enemies = 0

		for _,player in ipairs(players) do
			if self:objectiveLevel(player) >= 4 then
				enemies = enemies + 1
			elseif self:isFriend(player) then
				friends = friends + 1
			end
		end

		local ratio

		if friends == 0 then ratio = 999
		else ratio = enemies/friends
		end

		if ratio > 1.5 then
			use.card = card
			return
		end
	end
end

function SmartAI:useCardDismantlement(dismantlement, use)
	if self.player:hasSkill("wuyan") then return end
	if (not self.has_wizard) and self:hasWizard(self.enemies) then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, dismantlement)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = dismantlement
				if use.to then use.to:append(player) end
				return
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, dismantlement)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(dismantlement, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = dismantlement
				if use.to then use.to:append(friend) end
				return
			end
			if self:isEquip("SilverLion", friend) and friend:isWounded() and (friend:hasSkill("benghuai") or friend:getHp() < 4) then
				hasLion = true
				target = friend
			end
		end
	end

	if hasLion then
		use.card = dismantlement
		if use.to then use.to:append(target) end
		return
	end

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy) < 8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end
	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		if not enemy:isNude() and self:hasTrickEffective(dismantlement, enemy) and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("lianying") then return end
				elseif enemy:hasSkill("zhinang") then
					return
			end
			use.card = dismantlement
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

-- very similar with SmartAI:useCardDismantlement
function SmartAI:useCardSnatch(snatch, use)
	if self.player:hasSkill("wuyan") then return end

	if (not self.has_wizard) and self:hasWizard(self.enemies)  then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, snatch)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = snatch
				if use.to then use.to:append(player) end

				return
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, snatch)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(snatch, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = snatch
				if use.to then use.to:append(friend) end
				return
			end
			if self:isEquip("SilverLion", friend) and friend:isWounded() and (friend:hasSkill("benghuai") or friend:getHp() < 4) then
				hasLion = true
				target = friend
			end
		end
	end

	if hasLion then
		use.card = snatch
		if use.to then use.to:append(target) end
		return
	end

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy) < 8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end
	local enemies = self:exclude(self.enemies, snatch)
	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(snatch, enemy) and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("lianying") then return end
			elseif enemy:hasSkill("zhinang") then
				return
			end
			use.card = snatch
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

function SmartAI:useCardCollateral(card, use)
	self:sort(self.enemies,"threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend)
			and not self.room:isProhibited(self.player, friend, card) then

			for _, enemy in ipairs(self.enemies) do
				if friend:canSlash(enemy) then
					use.card = card
				end
				if use.to then use.to:append(friend) end
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end

	local n = nil
	local final_enemy = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and self:hasTrickEffective(card, enemy)
			and not self:hasSkills(sgs.lose_equip_skill, enemy)
			and not enemy:hasSkill("weimu")
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if use.to then use.to:append(enemy) end
						if use.to then use.to:append(enemy2) end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
			end
			if n then use.card = card end
			if use.to then use.to:append(enemy) end
			if use.to then use.to:append(final_enemy) end
			return

		end
		n = nil
	end
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardIndulgence(card, use)
	table.sort(self.enemies, hp_subtract_handcard)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if self:hasSkills("lijian|fanjian",enemy) and not enemy:containsTrick("indulgence") and not enemy:isKongcheng() and enemy:faceUp() and self:objectiveLevel(enemy) > 3 then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("keji") and enemy:faceUp() and self:objectiveLevel(enemy) > 3 then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end
