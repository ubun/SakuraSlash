-- this scripts contains the AI classes for generals of negative package

-- ruoshui
sgs.ai_skill_invoke["ruoshui"] = function(self, data)
	local player = data:toPlayer()
	if self:isFriend(player) and not self:isWeak() then
		return math.random(1, 2) == 1
	end
	return false
end
