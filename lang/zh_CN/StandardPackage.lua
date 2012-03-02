-- translation for StandardPackage

local t = {
	["standard_cards"] = "标准卡牌包",
	["standard_ex_cards"] = "标准EX卡牌包",

	["slash"] = "杀",
	[":slash"] = "出牌时机：出牌阶段\
使用目标：除你外，你攻击范围内的一名角色\
作用效果：【杀】对目标角色造成1点伤害",
	["slash-jink"] = "%src 使用了【杀】，请打出一张【闪】",

	["jink"] = "闪",
	[":jink"] = "出牌时机：以你为目标的【杀】开始结算时\
使用目标：以你为目标的【杀】\
作用效果：抵消目标【杀】的效果",
	["#Slash"] = "%from 对 %to 使用了【杀】",
	["#Jink"] = "%from 使用了【闪】",

	["peach"] = "肉",
	[":peach"] = "出牌时机：1、出牌阶段。2、有角色处于濒死状态时\
使用目标：1、你。2、处于濒死状态的一名角色\
作用效果：目标角色回复1点体力",

	["crossbow"] = "机关枪",
	[":crossbow"] = "攻击范围：１\
武器特效：出牌阶段，你可以使用任意张【杀】",

	["hammer"] = "锤（5吨）",
	[":hammer"] = "攻击范围：１\
武器特效：当你使用杀指定一名角色为目标时，可进行一次判定，若为红色该伤害+1，否则该伤害-1。",

	["double_sword"] = "人妖话筒",
	[":double_sword"] = "攻击范围：２\
武器特效：当你使用杀指定一名不同性别的角色为目标时，可以摸一张牌。",

	["qinggang_sword"] = "十手",
	[":qinggang_sword"] = "攻击范围：２\
武器特效：<b>锁定技</b>，当你使用【杀】指定一名角色为目标后，无视其防具",

	["blade"] = "狗枪",
	[":blade"] = "攻击范围：３\
武器特效：当你使用的【杀】被【闪】抵消时，你可以立即对相同的目标再使用一张【杀】直到【杀】生效或你不愿意出了为止",
	["blade-slash"] = "您可以再打出一张【杀】发动狗枪的追杀效果",

	["spear"] = "铁云剑",
	[":spear"] = "攻击范围：３\
武器特效：你可以将两张手牌当【杀】使用或打出",

	["axe"] = "黑刀",
	[":axe"] = "攻击范围：３\
武器特效：当你使用的【杀】被【闪】抵消时，你可以弃置两张牌，则此【杀】依然造成伤害",
	["@axe"] = "你可再弃置两张牌（包括装备）使此杀强制命中",
	["#AxeSkill"] = "%from 使用了【%arg】的技能，弃置了2张牌以对 %to 强制命中",

	["halberd"] = "天候棒",
	[":halberd"] = "攻击范围：４\
武器特效：在任意角色的气象判定牌生效前，你可以打出一张牌代替之。",
	["@halberd"] = "你可以使用【天候棒】来修改 %src 的 %arg 判定",

	["kylin_bow"] = "燃烧炮",
	[":kylin_bow"] = "攻击范围：５\
武器特效：当你使用【杀】对目标角色造成伤害时，你可以弃置其装备区里的一张坐骑牌",
	["kylin_bow:yes"] = "弃置对方的坐骑",

	["guding_blade"] = "象剑",
	[":guding_blade"] = "攻击范围：２\
武器特效：<b>锁定技</b>，当你使用【杀】对目标角色造成伤害时，若其没有手牌，此伤害+1",
	["#GudingBladeEffect"] = "%from 装备的象剑生效，无手牌对象 %to 的伤害从 %arg 增加到 %arg2",

	["eight_diagram"] = "超人披风",
	[":eight_diagram"] = "防具效果：每当你需要使用（或打出）一张【闪】时，你可以进行一次判定：若结果为红色，则视为你使用（或打出）了一张【闪】；若为黑色，则你仍可从手牌里使用（或打出）。\
★由披风使用或打出的【闪】，并非从你的手牌中使用或打出",
	["eight_diagram:yes"] = "进行一次判定，若判定结果为红色，则视为你打出了一张【闪】",

	["night_diagram"] = "海军披风",
	[":night_diagram"] = "防具效果：每当你需要使用（或打出）一张【闪】时，你可以进行一次判定：若结果为黑色，则视为你使用（或打出）了一张【闪】；若为红色，则你仍可从手牌里使用（或打出）。\
★由披风使用或打出的【闪】，并非从你的手牌中使用或打出",
	["night_diagram:yes"] = "进行一次判定，若判定结果为红色，则视为你打出了一张【闪】",

	["mask"] = "天龙人面具",
	[":mask"] = "防具效果：与你距离为1以内的角色才能对你出杀。",

	["horse"] = "配件",
	["defensive_horse"] = "坐骑",
	[":+1 horse"] = "其他角色计算与你的距离时，始终+1。（你可以理解为一种防御上的优势）不同名称的+1坐骑，其效果是相同的",
	["jueying"] = "飞鱼",
	["dilu"] = "威霸",
	["zhuahuangfeidian"] = "毛河马",

	["offensive_horse"] = "饰品",
	[":-1 horse"] = "你计算与其他角色的距离时，始终-1。（你可以理解为一种进攻上的优势）不同名称的-1饰品，其效果是相同的",
	["chitu"] = "云贝",
	["dayuan"] = "丛林腰带",
	["zixing"] = "狙击镜",

	["amazing_grace"] = "黄金乡",
	[":amazing_grace"] = "出牌时机：出牌阶段。\
使用目标：所有角色。\
作用效果：你从牌堆亮出等同于现存角色数量的牌，然后按行动顺序结算，每名目标角色选择并获得其中的一张",

	["god_salvation"] = "宴会",
	[":god_salvation"] = "出牌时机：出牌阶段。\
使用目标：所有角色。\
作用效果：按行动顺序结算，目标角色回复1点体力",

	["savage_assault"] = "屠魔令",
	[":savage_assault"] = "出牌时机：出牌阶段。\
使用目标：除你以外的所有角色。\
作用效果：按行动顺序结算，除非目标角色打出1张【杀】，否则该角色受到【屠魔令】对其造成的1点伤害",
	["savage-assault-slash"] = "%src 使用了【屠魔令】，请打出一张【杀】来响应",

	["archery_attack"] = "海啸",
	[":archery_attack"] = "出牌时机：出牌阶段。\
使用目标：除你以外的所有角色。\
作用效果：按行动顺序结算，除非目标角色打出1张【闪】，否则该角色受到【海啸】对其造成的1点伤害",
	["archery-attack-jink"] = "%src 使用了【海啸】，请打出一张【闪】以响应",

	["collateral"] = "挑拨",
	[":collateral"] = "出牌时机：出牌阶段。\
使用目标：装备区里有武器牌的一名其他角色A。（你需要在此阶段选择另一个A使用【杀】能攻击到的角色B）。\
作用效果：A需对B使用一张【杀】，否则A必须将其装备区的武器牌交给你",
	["collateral-slash"] = "%src 使用了【挑拨】，目标是 %dest，请打出一张【杀】以响应",

	["duel"] = "决斗",
	[":duel"] = "出牌时机：出牌阶段\
使用目标：一名其他角色\
作用效果：由该角色开始，你与其轮流打出一张【杀】，首先不出【杀】的一方受到另一方造成的1点伤害",
	["duel-slash"] = "%src 向您决斗，您需要打出一张【杀】",

	["ex_nihilo"] = "木质宝箱",
	[":ex_nihilo"] = "出牌时机：出牌阶段。\
使用目标：你。\
作用效果：摸两张牌",

	["ex_nihilp"] = "金质宝箱",
	[":ex_nihilp"] = "出牌时机：出牌阶段。\
使用目标：你。\
作用效果：摸三张牌",

	["snatch"] = "盗取",
	[":snatch"] = "出牌时机：出牌阶段。\
使用目标：与你距离1以内的一名其他角色。\
作用效果：你选择并获得其区域里的一张牌",

	["dismantlement"] = "船工分解",
	[":dismantlement"] = "出牌时机：出牌阶段。\
使用目标：一名其他角色。\
作用效果：你选择并弃置其区域里的一张牌",

	["nullification"] = "霸气",
	[":nullification"] = "出牌时机：目标技能牌对目标角色生效前。\
使用目标：目标技能牌（除霸王色霸气）。\
作用效果：抵消目标技能牌对一名角色产生的效果，或抵消另一张【霸气】产生的效果",

	["nullificatiom"] = "霸王色霸气",
	[":nullificatiom"] = "出牌时机：目标技能牌对目标角色生效前。\
使用目标：目标技能牌。\
作用效果：抵消目标技能牌对一名角色产生的效果，或抵消另一张【霸气】或【霸王色霸气】产生的效果",

	["indulgence"] = "消极之魂",
	[":indulgence"] = "出牌时机：出牌阶段。\
使用目标：一名其他角色。\
作用效果：将【消极之魂】放置于目标角色判定区里，目标角色回合判定阶段，进行判定；若判定结果不为红桃，则跳过目标角色的出牌阶段，将【消极之魂】置入弃牌堆",

	["lightning"] = "雷电",
	[":lightning"] = "出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【雷电】放置于你判定区里，目标角色回合判定阶段，进行判定；若判定结果为黑桃2-9之间（包括黑桃2和9），则【雷电】对目标角色造成3点伤害，将闪电置入弃牌堆。若判定结果不为黑桃2-9之间（包括黑桃2和9），将【雷电】移动到当前目标角色下家（【雷电】的目标变为该角色）的判定区",

}

local ohorses = {"chitu", "dayuan", "zixing"}
local dhorses = {"zhuahuangfeidian", "dilu", "jueying", "hualiu"}

for _, horse in ipairs(ohorses) do
	t[":" .. horse] = t[":-1 horse"]
end

for _, horse in ipairs(dhorses) do
	t[":" .. horse] = t[":+1 horse"]
end

return t