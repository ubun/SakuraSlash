-- translation for MountainPackage

return {
	["mountain"] = "不动如山",

	["yamatokansuke"] = "大和敢助",
	["bamian"] = "疤面",
	[":bamian"] = "锁定技，你只能装备一辆车，你装备的车具有×2效果。",
	["bansha"] = "半杀",
	[":bansha"] = "出牌阶段，当你使用【杀】被【闪】抵消时，若目标角色的手牌数不小于你的手牌数，则你可以获得其一张手牌。",

	["morofushitakaaki"] = "诸伏高明",
	["kongcheng"] = "空城",
	[":kongcheng"] = "锁定技，当你的手牌数小于体力值时，不能成为【杀】、【决斗】或【偷袭】的目标。",
	["kanpo"] = "看破",
	[":kanpo"] = "你可以将黑色非锦囊牌当【无懈可击】使用。",

	["kawaguchisatoshi"] = "川口聪",
	["foyuan"] = "佛缘",
	[":foyuan"] = "你可以将黑色基本牌当【佛跳墙】使用且无距离限制。",

	["sawadahiroki"] = "泽田弘树",
	["jihun"] = "寄魂",
	[":jihun"] = "当任意角色受到伤害时，你可以打出一张手牌，将造成伤害的牌改成这张牌。",
	["chengshi"] = "程式",
	[":chengshi"] = "你每受到【杀】造成的1点伤害，可进行一次判定，若为红色，令任一已受伤角色回复一点体力。",
	["yimeng"] = "遗梦",
	[":yimeng"] = "当你死亡时，你可和任一其他角色交换武将牌。",
	["@jihun"] = "你可以发动【寄魂】更改 %src 对 %dest 造成伤害的牌",
	["$Jihun"] = "%from 发动了【寄魂】，将对 %to 造成伤害的牌改为了 %card",

	["kojimagenji"] = "小岛元次",
	["zhibao"] = "制暴",
	[":zhibao"] = "其他角色受到【杀】造成的伤害后，你可以弃一张基本牌，视为对伤害来源使用了一张【决斗】。",
	["@zhibao"] = "%src 被杀伤了，你可以发动【制暴】，弃一张基本牌教训 %dest",

	["kamenyaibaa"] = "假面超人",
	["feiti"] = "飞踢",
	[":feiti"] = "当你造成一次伤害时，可以避免此伤害，获得X个标记，X为造成伤害的点数；回合结束阶段开始时，你可以弃掉1个标记，对攻击范围外的一名角色造成1点伤害。（此伤害不纳入飞踢的发动条件）",
	["@yaiba"] = "假面",
	["@feiti"] = "你可以对一名攻击范围外的角色使用【飞踢】",

	["datewataru"] = "伊达航",
	["shehang"] = "涉航",
	[":shehang"] = "你可以将其他角色装备区里的牌回合内当杀、回合外当闪使用或打出。",
	["shehang:slash"] = "你可以发动【涉航】，将其他角色装备区里的牌当【杀】打出",
	["shehang:jink"] = "你可以发动【涉航】，将其他角色装备区里的牌当【闪】打出",

	["yokomizo"] = "横沟兄弟",
	["canwu"] = "参悟",
	[":canwu"] = "单发技，你可以和一名已受伤的男性角色拼点，若你赢，你和其各回复一点体力。",
	["chongwu"] = "重悟",
	[":chongwu"] = "回合外，任一其他角色回复体力时，你可以扣减一点体力，阻止其体力回复。",

	["kurobatouichi"] = "黑羽盗一",
	["guifu"] = "鬼斧",
	[":guifu"] = "锁定技，游戏开始时，你获得2个“鬼斧”标记。你每受到或造成的伤害均视为扣减或获得相同数量的“鬼斧”标记，体力流失和扣减体力上限对你无效。你的手牌上限+2。当你没有“鬼斧”标记时，须立即做一次判定，若为【桃】或红色锦囊，你获得2个“鬼斧”标记，否则将体力设为0并进入濒死阶段。",
	["shengong"] = "神工",
	[":shengong"] = "主公技，怪势力角色可以在自己的回合内选择失去一点体力，为你增加一个“鬼斧”标记。",
	["@hatchet"] = "鬼斧",
	["shengongv"] = "工神",
	[":shengongv"] = "单发技，怪势力角色可以失去一点体力，为主公黑羽盗一增加一个“鬼斧”标记。",

	["rye"] = "黑麦威士忌",

	["kir"] = "基尔",
	["wulian"] = "无怜",
	[":wulian"] = "锁定技，在以下情况下，你的【杀】不可闪避：你的体力值等于对方的体力值；你的体力值等于对方的当前手牌数。",
	["#Wulian"] = "%from 的技能【%arg】被触发，对 %to 的杀不可被闪避",

	["andrewcamel"] = "安德烈•卡迈尔",
	["cheji"] = "车技",
	[":cheji"] = "锁定技，若你没装备防具，视为你装备了一辆-1车。",
	["luosha"] = "裸杀",
	[":luosha"] = "当对方装备区里没牌时，你的【杀】造成的伤害可以+1.",

	["enomotoazusa"] = "榎本梓",
	["designer:enomotoazusa"] = "烨子",
	["zhaodai"] = "招待",
	[":zhaodai"] = "单发技，你可以将一张基本牌交给任一其他角色，然后选择一项：摸一张牌，或令其摸一张牌。",
	["kaxiang"] = "咖香",
	[":kaxiang"] = "当你受到其他角色造成的伤害时，你可以和该角色拼点。若你赢，则防止该伤害。",
	["zhaodai:tian"] = "对方摸1张牌",
	["zhaodai:zi"] = "自己摸1张牌",
	["#Kaxiang"] = "%from 对 %to 造成的 %arg 点伤害被抵消",

	["yamamuramisae"] = "山村美砂绘",
	["biaoche"] = "飙车",
	[":biaoche"] = "锁定技，当你计算与体力比你多的角色的距离时，始终-1；当体力比你多的角色计算与你的距离时，始终+1。",
	["jingshen"] = "敬神",
	[":jingshen"] = "单发技，你可以将一张手牌置于你的武将牌上，称为“祭品”，当祭品的数量和全场角色的数量相等时，你须将全部祭品交给一名其他角色。",
	["jipin"] = "祭品",

}
