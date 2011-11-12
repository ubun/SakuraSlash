#include "secrets.h"
#include "engine.h"
#include "skill.h"
#include "standard.h"
#include "carditem.h"
#include "client.h"

QStringList Tipslist(int index){
    QMap<int, QStringList> tipsmap;

    tipsmap[13] << "woo" << "wusheng" << "guai" << "kongcheng" << "kuanggu";
    tipsmap[12] << "te" << "qixi" << "shao" << "feiying" << "quhu";
    tipsmap[11] << "zhen" << "kanpo" << "yi" << "biyue" << "beige"; //
    tipsmap[10] << "hei" << "jiuchi" << "jing" << "keji" << "leiji";
    tipsmap[9] << "yi" << "guose" << "guai" << "paoxiao" << "lijian";
    tipsmap[8] << "te" << "huoji" << "zhen" << "tiandu" << "mingce";
    tipsmap[7] << "yi" << "longdan" << "hei" << "weimu" << "fanjian";
    tipsmap[6] << "shao" << "qingguo" << "zhen" << "yingzi" << "tuxi";
    tipsmap[5] << "zhen" << "jijiu" << "yi" << "qicai" << "xuanhuo";
    tipsmap[4] << "jing" << "" << "shao" << "jushou" << "guanxing";
    tipsmap[3] << "jing" << "lianhuan" << "woo" << "jiang" << "xinzhan";
    tipsmap[2] << "guai" << "duanliang" << "woo" << "mashu" << "tiaoxin";
    tipsmap[1] << "hei" << "luanji" << "te" << "qianxun" << "ganlu";

    return tipsmap.value(index, QStringList());
}

QString SecretsCard::getType() const{
    return "secrets";
}

Card::CardType SecretsCard::getTypeId() const{
    return Secrets;
}

Tips::Tips(Suit suit, int number): SecretsCard(suit, number){
    target_fixed = true;
}

QString Tips::getSubtype() const{
    return "secret_card";
}

void Tips::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QStringList tiplist = Tipslist(this->getNumber());
    QString skill;
    QString kingdom = source->getKingdom();
    if(kingdom == tiplist.at(0))
        skill = tiplist.at(1);
    else if(kingdom == tiplist.at(2))
        skill = tiplist.at(3);
    else
        skill = tiplist.at(4);

    LogMessage log;
    log.type = "$SecretCard";
    log.from = source;
    log.card_str = this->getEffectIdString();

    room->sendLog(log);
    room->throwCard(this);
    QString oldskill = source->tag.value("Tip", "").toString();
    room->detachSkillFromPlayer(source, oldskill);
    room->acquireSkill(source, skill);
    source->tag["Tip"] = skill;
}

SecretsPackage::SecretsPackage()
    :Package("secretsp")
{
    QList<Card *> tips;
    tips
            << new Tips(Card::Spade, 13)
            << new Tips(Card::Diamond, 12)
            << new Tips(Card::Club, 11)
            << new Tips(Card::Heart, 10)
            << new Tips(Card::Spade, 9)
            << new Tips(Card::Diamond, 8)
            << new Tips(Card::Club, 7)
            << new Tips(Card::Heart, 6)
            << new Tips(Card::Spade, 5)
            << new Tips(Card::Diamond, 4)
            << new Tips(Card::Club, 3)
            << new Tips(Card::Heart, 2)
            << new Tips(Card::Spade, 1);

    tips.at(0)->setObjectName("woaini");
    tips.at(1)->setObjectName("jingxian");
    tips.at(2)->setObjectName("kouhong");
    tips.at(3)->setObjectName("kiss");
    tips.at(4)->setObjectName("nvyanyuan");
    tips.at(5)->setObjectName("dikaer");
    tips.at(6)->setObjectName("zuanshi");
    tips.at(7)->setObjectName("zifahuxi");
    tips.at(8)->setObjectName("jiban");
    tips.at(9)->setObjectName("shipinliaotian");
    tips.at(10)->setObjectName("dianti");
    tips.at(11)->setObjectName("shengri");
    tips.at(12)->setObjectName("pingtianyi");

    foreach(Card *card, tips)
        card->setParent(this);

    type = CardPack;
/*
    skills
            << new EyedropsSkill //liubei
            << new RedsunglassesSkill //guanyu
            << new SpeakerSkill //zhangfei
            << new BananaSkill //zhaoyun
            << new FengjieSkill //huangyueying
            << new DustbinSkill //luxun
            << new AnimalsSkill //lvbu
            << new RollingpinSkill //sunshuishui
            << new DeathriSkill //huatuo
            << new AmazonstonSkill //huangzhong
            << new GnatSkill //caoren
            << new TranqgunSkill //xuhuang
            << new TombstoneSkill //dongzhuo
            << new SwitchbdSkill //wolong
            << new GoldlockSkill //yuanshao
            << new AppleSkill //taishici
            << new WookonSkill //dengai
            ;

    addMetaObject<RollingpinCard>();
    addMetaObject<WookonCard>();*/
}

ADD_PACKAGE(Secrets);

