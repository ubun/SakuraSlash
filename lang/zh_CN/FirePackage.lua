-- translation for FirePackage

return {
	["fire"] = "侵略如火",

	["amurotooru"] = "安室透",
	["cv:amurotooru"] = "古谷彻",
	["ientou"] = "渗透",
	[":ientou"] = "单发技，你可弃掉差值数的手牌，互换除主公外两名角色的体力值。\
★互换体力时，若遇到上限溢出问题，以上限为界流失体力，比如3血3上限和4血4上限交换后，变成3血3上限和3血4上限。",
	["$ientou"] = "原来还有个年轻的女侦探啊。",

	["matsunakayuriko"] = "松中百合子",
	["biguan"] = "闭馆",
	[":biguan"] = "锁定技，摸牌阶段，你摸X张牌，X为你当前手牌数，最多为4.",
	["fengsheng"] = "逢生",
	[":fengsheng"] = "锁定技，当你解除濒死状态时，手牌上限+1.",
	["#Fengsheng"] = "%from 的锁定技【%arg】被触发，增加了1点手牌上限",
	["@door"] = "门",

	["kugorou"] = "毛利小五郎",
	["yiben"] = "一本",
	[":yiben"] = "回合外，当你失去一张手牌时，可视为对任一其他角色使用一张【杀】。",
	["xiebi"] = "卸臂",
	[":xiebi"] = "当你对武将牌背面朝上的角色造成伤害后，可令该角色将其武将牌翻面。",

	["kinoshitafusae"] = "木之下芙纱绘",
	["akibareiko"] = "秋庭怜子",
	["chuyin"] = "初音",
	[":chuyin"] = "当你受到伤害时，可增加1点体力上限，若此时你的体力值为全场最少，其他角色依次进行二选一：失去一点体力或弃置两张牌。",

	["satomiwako"] = "佐藤美和子",
	["cv:satomiwako"] = "湯屋敦子",
	["designer:satomiwako"] = "烨子",
	["jiaoxie"] = "缴械",
	[":jiaoxie"] = "其他角色的武器牌进入弃牌堆时，你可以获得之。",
	["xianv"] = "侠女",
	[":xianv"] = "任意角色受到无属性伤害时，你可以弃置一张装备牌，令该伤害-1.",
	["@xianv"] = "你可以弃置一张装备牌发动【侠女】，令 %src 受到的伤害 -1",
	["$Xianv"] = "%from 发动了【侠女】，弃置了 %card，令 %to 受到的伤害 -1",
	["$jiaoxie1"] = "我来想办法。",
	["$jiaoxie2"] = "放下枪，我是警察！", -- 缺
	["$xianv1"] = "一定不会让你逃走！",
	["$xianv2"] = "一定要亲手抓住你！",
	["~satomiwako"] = "让我忘掉他吧，笨蛋……",

	["takagiwataru"] = "高木涉",
	["cv:takagiwataru"] = "高木涉",
	["mune"] = "木讷",
	[":mune"] = "在你的判定牌生效后，你可以立即获得它。若此时为判定阶段，你同时获得此延时锦囊。",
	["gengzhi"] = "耿直",
	[":gengzhi"] = "当你受到伤害时，翻开牌堆顶两张牌，若有且仅有一张红桃牌，你回复一点体力并获得此牌，否则随机获得其中一张牌，另一张牌放回牌堆顶。",
	["$mune"] = "我真的要生气了！",
	["$gengzhi"] = "你不是这样说的吗？",

	["koizumiakako"] = "小泉红子",
	["designer:koizumiakako"] = "烨子",
	["cv:koizumiakako"] = "沢城みゆき",
	["fangxin"] = "芳心",
	[":fangxin"] = "当你需要使用或打出一张【杀】时，你可以进行一次判定：若结果不为♥，则视为你使用或打出一张【杀】，否则你不能使用或打出【杀】，直到回合结束。",
	["mogua"] = "魔卦",
	[":mogua"] = "出牌阶段，你可以将任意数量的手牌以任意顺序置于牌堆顶。",
	["$fangxin1"] = "这次我一定要让你成为我的俘虏。",
	["$fangxin2"] = "哦呵呵呵呵~~~~",
	["$mogua"] = "魔法球，快告诉我，这世上最美的人是谁？",

	["hakubasaguru"] = "白马探",
	["sherry"] = "雪莉",

	["miyanoagemi"] = "宫野明美",
	["shanliang"] = "善良",
	[":shanliang"] = "距离2以内的角色受到伤害时，你可以失去一点体力，防止该伤害，然后摸X张牌，X为该角色当前体力值。",
	["qingshang"] = "情殇",
	[":qingshang"] = "锁定技，回合外，若你的体力为1，非属性伤害对你无效。",
	["#QSProtect"] = "%to 的锁定技【%arg】被触发，%from 对 %to 造成的伤害无效",

	["hondouhidemi"] = "本堂瑛海",
	["shendie"] = "神谍",
	[":shendie"] = "单发技，你可以和任一非主公角色更换身份牌，其他任何角色都不知道你对谁发动了神谍技能，包括被神谍的角色本身。任何角色都无法查看神谍后的身份牌，包括你自己。当有角色死亡后，你须增加1点体力槽，失去技能【神谍】直到游戏结束",
	["gangxie"] = "钢楔",
	[":gangxie"] = "锁定技，对你使用【杀】，需要再打出一张【杀】才能生效；和你【决斗】的角色必须先出【杀】",
	["#Gangxie"] = "$from 的锁定技【%arg】生效，%to 需再打出一张【杀】",
	["@gangxie"] = "%src 的锁定技【钢楔】被触发，请再打出一张【杀】",

	["chibaisshin"] = "千叶一伸",
	["miikenaeko"] = "三池苗子",

}

