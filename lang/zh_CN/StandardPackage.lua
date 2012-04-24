-- translation for StandardPackage

return {
	["standard_cards"] = "标准卡牌包",

	["slash"] = "杀",
	[":slash"] = "基本牌<br />出牌时机：出牌阶段<br />使用目标：除你外，你攻击范围内的一名角色<br />作用效果：【杀】对目标角色造成1点伤害",
	["slash-jink"] = "%src 使用了【杀】，请打出一张【闪】",

	["jink"] = "闪",
	[":jink"] = "基本牌<br />出牌时机：以你为目标的【杀】开始结算时<br />使用目标：以你为目标的【杀】<br />作用效果：抵消目标【杀】的效果",
	["#Slash"] = "%from 对 %to 使用了【杀】",
	["#Jink"] = "%from 使用了【闪】",

	["peach"] = "桃",
	[":peach"] = "基本牌<br />出牌时机：1、出牌阶段。2、有角色处于濒死状态时<br />使用目标：1、你。2、处于濒死状态的一名角色<br />作用效果：目标角色回复1点体力",

	["crossbow"] = "行凶匕首",
	[":crossbow"] = "装备牌·武器牌<br />攻击范围：１<br />武器特效：出牌阶段，你可以使用任意张【杀】",

	["watch"] = "麻醉手表",
	[":watch"] = "装备牌·武器牌<br />攻击范围：２<br />武器特效：当你使用【杀】对目标角色造成伤害时，可以和其进行一次拼点，若你赢，目标角色将自己的武将牌翻面，否则目标角色获得麻醉手表",

	["qinggang_sword"] = "斩铁剑",
	[":qinggang_sword"] = "装备牌·武器牌<br />攻击范围：２<br />武器特效：<b>锁定技</b>，当你使用【杀】指定一名角色为目标后，无视其防具",

	["blade"] = "追踪眼镜",
	[":blade"] = "装备牌·武器牌<br />攻击范围：３<br />武器特效：当你使用的【杀】被【闪】抵消时，你可以立即对相同的目标再使用一张【杀】直到【杀】生效或你不愿意出了为止",
	["blade-slash"] = "您可以再打出一张【杀】发动追踪眼镜的追杀效果",

	["spear"] = "足球射出阀",
	[":spear"] = "装备牌·武器牌<br />攻击范围：３<br />武器特效：你可以将两张牌当【杀】使用或打出",

	["axe"] = "增强脚力鞋",
	[":axe"] = "装备牌·武器牌<br />攻击范围：３<br />武器特效：当你使用的【杀】被【闪】抵消时，你可以弃置两张牌，则此【杀】依然造成伤害",
	["@axe"] = "你可再弃置两张牌（包括装备）使此杀强制命中",
	["#AxeSkill"] = "%from 使用了【%arg】的技能，弃置了2张牌以对 %to 强制命中",

	["psg"] = "PSG-1",
	[":psg"] = "装备牌·武器牌<br />攻击范围：４<br />武器特效：<b>锁定技</b>，当你使用的【杀】是你最后一张手牌时，此【杀】不可闪避",

	["kylin_bow"] = "AWP",
	[":kylin_bow"] = "装备牌·武器牌<br />攻击范围：５<br />武器特效：当你使用【杀】对目标角色造成伤害时，你可以弃置其装备区里的所有车",

	["eight_diagram"] = "护身符",
	[":eight_diagram"] = "装备牌·防具牌<br />防具效果：每当你需要使用（或打出）一张【闪】时，你可以进行一次判定：若结果为红色，则视为你使用（或打出）了一张【闪】；若为黑色，则你仍可从手牌里使用（或打出）。",
	["eight_diagram:yes"] = "进行一次判定，若判定结果为红色，则视为你打出了一张【闪】",

	["renwang_shield"] = "伸缩吊带",
	[":renwang_shield"] = "装备牌·防具牌<br />防具效果：<b>锁定技</b>，黑色的【杀】对你无效",

	["ice_sword"] = "扑克枪",
	[":ice_sword"] = "装备牌·武器牌<br />攻击范围：２<br />武器特效：当你使用【杀】对目标角色造成伤害时，若该角色有牌，你可以防止此伤害，改为依次弃置其两张牌（弃完第一张再弃第二张）",
	["ice_sword:yes"] = "您可以弃置其两张牌",

	["car"] = "车",
	["chevyCK"] = "雪佛兰CK",
	[":chevyCK"] = "装备牌·车[进攻]<br />你计算与其他角色的距离时，始终-1<br/>\
◆赤井秀一专用技：当你受到伤害时，可立即发动“殉职”",
	["benzCLK"] = "奔驰CLK",
	[":benzCLK"] = "装备牌·车[进攻]<br />你计算与其他角色的距离时，始终-1<br/>\
◆詹姆斯·布莱克专用技：每回合你可以发动两次“暗涌”",
	["skateboard"] = "涡轮滑板",
	[":skateboard"] = "装备牌·车？[进攻]<br />你计算与其他角色的距离时，始终-1<br/>\
◆柯南专用技：？？",

	["porsche365A"] = "保时捷365A",
	[":porsche365A"] = "装备牌·车[防守]<br />其他角色计算与你的距离时，始终+1<br/>\
◆琴酒专用技：回合内，你可以将黑色手牌当【杀】使用",
	["beetle"] = "大众甲壳虫",
	[":beetle"] = "装备牌·车[防守]<br />其他角色计算与你的距离时，始终+1<br/>\
◆阿笠博士专用技：发动“溯源”时，你可以同时更改伤害目标",
	["mazdaRX7"] = "马自达RX-7",
	[":mazdaRX7"] = "装备牌·车[防守]<br />其他角色计算与你的距离时，始终+1<br/>\
◆佐藤美和子专用技：任意角色受到属性伤害时，你仍然可以发动“侠女”",

	["amazing_grace"] = "雨后彩虹",
	[":amazing_grace"] = "出牌时机：出牌阶段。\
使用目标：所有角色。\
作用效果：你从牌堆亮出等同于现存角色数量的牌，然后按行动顺序结算，每名目标角色选择并获得其中的一张",

	["god_salvation"] = "落英缤纷",
	[":god_salvation"] = "出牌时机：出牌阶段。\
使用目标：所有角色。\
作用效果：按行动顺序结算，目标角色回复1点体力",

	["savage_assault"] = "倾巢出动",
	[":savage_assault"] = "出牌时机：出牌阶段。\
使用目标：除你以外的所有角色。\
作用效果：按行动顺序结算，除非目标角色打出1张【杀】，否则该角色受到【倾巢出动】对其造成的1点伤害",
	["savage-assault-slash"] = "%src 使用了【倾巢出动】，请打出一张【杀】来响应",

	["archery_attack"] = "枪林弹雨",
	[":archery_attack"] = "出牌时机：出牌阶段。\
使用目标：除你以外的所有角色。\
作用效果：按行动顺序结算，除非目标角色打出1张【闪】，否则该角色受到【枪林弹雨】对其造成的1点伤害",
	["archery-attack-jink"] = "%src 使用了【枪林弹雨】，请打出一张【闪】以响应",

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

	["ex_nihilo"] = "勇气",
	[":ex_nihilo"] = "出牌时机：出牌阶段。\
使用目标：你。\
作用效果：摸两张牌",

	["snatch"] = "顺手牵羊",
	[":snatch"] = "出牌时机：出牌阶段。\
使用目标：与你距离1以内的一名其他角色。\
作用效果：你选择并获得其区域里的一张牌",

	["dismantlement"] = "过河拆桥",
	[":dismantlement"] = "出牌时机：出牌阶段。\
使用目标：一名其他角色。\
作用效果：你选择并弃置其区域里的一张牌",

	["nullification"] = "无懈可击",
	[":nullification"] = "出牌时机：目标锦囊对目标角色生效前。\
使用目标：目标锦囊。\
作用效果：抵消目标锦囊牌对一名角色产生的效果，或抵消另一张【无懈可击】产生的效果",

	["indulgence"] = "鬼打墙",
	[":indulgence"] = "出牌时机：出牌阶段。\
使用目标：一名其他角色。\
作用效果：将【鬼打墙】放置于目标角色判定区里，目标角色回合判定阶段，进行判定；若判定结果不为红桃，则跳过目标角色的出牌阶段，将【鬼打墙】置入弃牌堆",

	["lightning"] = "定时炸弹",
	[":lightning"] = "出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【定时炸弹】放置于你判定区里，目标角色回合判定阶段，进行判定；若判定结果为黑桃2-9之间（包括黑桃2和9），则【定时炸弹】对目标角色造成3点伤害，将闪电置入弃牌堆。若判定结果不为黑桃2-9之间（包括黑桃2和9），将【定时炸弹】移动到当前目标角色下家（【定时炸弹】的目标变为该角色）的判定区",

--run a way
	["runaway_mode"] = "跑路模式",
	["#Runprex"] = "%from 翻开一张跑路牌，但由于其点数 %arg 不小于场上存活角色数，本次不得跑路",
	["#Runpre"] = "%from 翻开一张跑路牌，他需要跑 %arg 步路",
	["#Runaway2"] = "%arg 位 %from 和 %arg2 位 %to 交换了位置",
	["#Runaway"] = "%arg 位 %from 跑到了 %arg2 位 的位置",
	["runbycar"] = "爱车助我",
	["runbycar:fast"] = "踩油门（暴走）",
	["$runfast"] = "%from 装备有 %card ，多跑了一步路",
	["runbycar:slow"] = "踩刹车（龟速）",
	["$runslow"] = "%from 装备有 %card ，少跑了一步路",
	["runbycar:kao"] = "踩离合器（^o^）",
}
