-- when enemy using the peach
sgs.ai_skill_invoke["grab_peach"] = function(self, data)
	local struct= data:toCardUse()
	return self:isEnemy(struct.from) and (struct.to:isEmpty() or self:isEnemy(struct.to:first()))
end

function SmartAI:useGaleShell(card, use)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <=1 and not self:hasSkills("jijiu|wusheng|longhun",enemy) then
			use.card = card
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
end

function SmartAI:useCardShit(card, use)
	if (card:getSuit() == sgs.Card_Heart or card:getSuit() == sgs.Card_Club) and self.player:isChained() and
		#(self:getChainedFriends()) > #(self:getChainedEnemies()) then return end
	if self.player:getHp()>3 and self.player:hasSkill("shenfen") and self.player:hasSkill("kuangbao") then use.card = card return end
	if self.player:hasSkill("kuanggu") and card:getSuitString() ~= "spade" then use.card = card return end
	if card:getSuit() == sgs.Card_Heart and (self:isEquip("GaleShell") or self:isEquip("Vine")) then return end
	if not self.player:isWounded() then
		if self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1 then
			use.card = card
			return
		end
		if sgs[self.player:getGeneralName() .. "_suit_value"] and
			(sgs[self.player:getGeneralName() .. "_suit_value"][card:getSuitString()] or 0) > 0 then return end
		local peach = self:getCard("Peach")
		if peach then
			self:sort(self.friends, "hp")
			if not self:isWeak(self.friends[1]) then
				use.card = card
				return
			end
		end
	end
end

function SmartAI:useCardStink(card, use)
	local next_player = self.player:getNextAlive()
	if self:isFriend(next_player) then return end
	use.card = card
end
