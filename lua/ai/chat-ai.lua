function speak(to,type)
	local i =math.random(1,#sgs.ai_chat[type])
	to:speak(sgs.ai_chat[type][i])
end

function speakTrigger(card,from,to,event)
	if (event=="death") and from:hasSkill("ganglie") then
		speak(from,"ganglie_death")
	end
	if from:hasSkill("yongsi") then
		speak(from,"yongsi")
	end

	if not card then return end
	
	if card:inherits("Indulgence") and (to:getHandcardNum()>to:getHp()) then
		speak(to,"indulgence")
	elseif card:inherits("Bathroom") then
		speak(to,"bathroom")
	elseif card:inherits("Renew") then
		speak(from,"renew")
	elseif card:inherits("LeijiCard") then
		speak(from,"leiji_jink")
	elseif card:inherits("TuxiCard") then
		speak(to,"tuxi")
	elseif card:inherits("QuhuCard") then
		speak(from,"quhu")
	elseif card:inherits("NonoCard") and from:hasSkill("nono3") then
		speak(from,"nono")
	elseif card:inherits("Slash") and to:hasSkill("yiji") and (to:getHp()<=1) then
		speak(to,"guojia_weak")
	elseif card:inherits("SavageAssault") and (to:hasSkill("tieji") or to:hasSkill("jizhi") or to:hasSkill("huoji")) then
		speak(to,"daxiang")
	elseif to:hasSkill("guangrong") then
		speak(to,"guangrong")
	end
end


function SmartAI:speak(type, isFemale) 
    local i =math.random(1,#sgs.ai_chat[type])
	if isFemale then 
		type = type .. "_female" 
	end 
    self.player:speak(sgs.ai_chat[type][i])
end

sgs.ai_chat={}

sgs.ai_chat.tuxi=
{
"这，就是传说中的天地一屎？",
"回头弄不死你！"
}

sgs.ai_chat.yiji=
{
"有胆就来打我吧!",
"狼虎灭却·金城铁壁!"
}

sgs.ai_chat.hostile_female= 
{ 
"啧啧啧，来帮你解决点手牌吧", 
"叫你欺负人!" ,
"手牌什么的最讨厌了"
}

sgs.ai_chat.hostile={
"你喜欢雷尼么？我还是萌花火~",
"果然还是看你不爽",
"我看你霸气外露，不可不防啊",
"还是爱丽丝最可爱:)"
}

sgs.ai_chat.resbond_hostile={
"擦，小心菊花不保",
"内牛满面了", "哎哟我去" 
}

sgs.ai_chat.friendly=
{ "……" }

sgs.ai_chat.resbond_friendly= 
{ "谢了……" }

sgs.ai_chat.duel_female= 
{ 
"哼哼哼，怕了吧" 
}

sgs.ai_chat.duel=
{
"来！一决胜负吧！"
}

sgs.ai_chat.lucky=
{
"哎哟运气好", 
"哈哈哈哈哈" 
}

sgs.ai_chat.collateral_female= 
{ 
"别以为这样就算赢了！" 
}

sgs.ai_chat.collateral= 
{ 
"你妹啊，我的刀！" 
}

sgs.ai_chat.jijiang_female= 
{ 
"队长你现在和我不是一拨……爱莫能助啦" 
}

sgs.ai_chat.jijiang=
{
"队长稳住，我来帮你！"
}

--huanggai
sgs.ai_chat.kurou=
{
"有桃么!有桃么？",
"我，我想要桃……",
"桃桃桃我的桃呢",
"求桃求小机器人各种求"
}

--indulgence
sgs.ai_chat.indulgence=
{
"我去，机器又出故障了！",
"擦，敢弄我爱机？",
"诶诶诶被摆了一道……"
}

--leiji
sgs.ai_chat.leiji_jink=
{
"我有闪我会到处乱说么？",
"你觉得我有木有闪啊",
"哈我有闪"
}

--quhu
sgs.ai_chat.quhu=
{
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}

--coquelicot
sgs.ai_chat.nono=
{
"精彩的魔术，现在开始了！",
"喜欢我送的装备么？嘻嘻~~",
"下次可要好好谢我哦~"
}

--drawcard
sgs.ai_chat.guangrong=
{
"嘻嘻，旧的不去，新的不来~",
"我的手牌是无穷无尽的！"
}

--salvageassault
sgs.ai_chat.daxiang=
{
"帝国华击团又出动了，赶紧避难！",
"擦，还好三笠号没有出来……",
"内牛满面啊死伤无数啊……"
}

sgs.ai_chat.guojia_weak=
{
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}

--bathroom
sgs.ai_chat.bathroom=
{
"哇……色狼啊……",
"谁、谁在偷窥我……",
"555……被看光了……"
}				  

--renew
sgs.ai_chat.renew=
{
"爆发吧！",
"赐予我力量吧……我是超人！",
"哇哈哈~~"
}				  

--xuchu
sgs.ai_chat.luoyi=
{
"知道我在德国是怎么活下来的么？受死吧！"
}

--yongsi
sgs.ai_chat.yongsi=
{
"我可爱的拿破仑又拿了一堆破轮子~~"
}
