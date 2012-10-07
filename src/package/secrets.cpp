#include "secrets.h"
#include "engine.h"
#include "skill.h"
#include "standard.h"
#include "carditem.h"
#include "client.h"
#include "standard-skillcards.h"
#include "maneuvering.h"

QStringList Tipslist(int index){
    QMap<int, QStringList> tipsmap;

    tipsmap[13] << "woo" << "wusheng" << "guai" << "kongcheng" << "kuanggu";
    tipsmap[12] << "te" << "qixi" << "shao" << "feiying" << "quhu";
    tipsmap[11] << "zhen" << "kanpo" << "yi" << "biyue" << "beige";
    tipsmap[10] << "hei" << "jiuchi" << "jing" << "keji" << "leiji";
    tipsmap[9] << "yi" << "guose" << "guai" << "paoxiao" << "lijian";
    tipsmap[8] << "te" << "huoji" << "zhen" << "tiandu" << "mingce";
    tipsmap[7] << "yi" << "longdan" << "hei" << "weimu" << "fanjian";
    tipsmap[6] << "zhen" << "jijiu" << "yi" << "qicai" << "xuanhuo";
    tipsmap[5] << "shao" << "qingguo" << "zhen" << "yingzi" << "tuxi";
    tipsmap[4] << "jing" << "yiyong" << "shao" << "jushou" << "guanxing";
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

// view_as_skills
class Wusheng:public OneCardViewAsSkill{
public:
    Wusheng():OneCardViewAsSkill("wusheng"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(!card->isRed())
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class Qingguo:public OneCardViewAsSkill{
public:
    Qingguo():OneCardViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Jink *jink = new Jink(card->getSuit(), card->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(card->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink";
    }
};

class Guose: public OneCardViewAsSkill{
public:
    Guose():OneCardViewAsSkill("guose"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class Jijiu: public OneCardViewAsSkill{
public:
    Jijiu():OneCardViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("peach") && player->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

class Duanliang: public OneCardViewAsSkill{
public:
    Duanliang():OneCardViewAsSkill("duanliang"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isBlack() && !card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();

        SupplyShortage *shortage = new SupplyShortage(card->getSuit(), card->getNumber());
        shortage->setSkillName(objectName());
        shortage->addSubcard(card);

        return shortage;
    }
};

class Jiuchi: public OneCardViewAsSkill{
public:
    Jiuchi():OneCardViewAsSkill("jiuchi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());

        return analeptic;
    }
};

class Huoji: public OneCardViewAsSkill{
public:
    Huoji():OneCardViewAsSkill("huoji"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Kanpo: public OneCardViewAsSkill{
public:
    Kanpo():OneCardViewAsSkill("kanpo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack() && !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("kanpo");

        return ncard;
    }
};

class Lianhuan: public OneCardViewAsSkill{
public:
    Lianhuan():OneCardViewAsSkill("lianhuan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
        chain->addSubcard(card);
        chain->setSkillName(objectName());
        return chain;
    }
};

class Luanji:public ViewAsSkill{
public:
    Luanji():ViewAsSkill("luanji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            ArcheryAttack *aa = new ArcheryAttack(first->getSuit(), 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        }else
            return NULL;
    }
};

class Yiyong: public OneCardViewAsSkill{
public:
    Yiyong():OneCardViewAsSkill("yiyong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Sacrifice *acrifice = new Sacrifice(card->getSuit(), card->getNumber());
        acrifice->addSubcard(card->getId());
        acrifice->setSkillName(objectName());
        return acrifice;
    }
};

//upgrade_skills
class Yingzi:public DrawCardsSkill{
public:
    Yingzi():DrawCardsSkill("yingzi"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        if(room->askForSkillInvoke(zhouyu, objectName())){
            room->playSkillEffect(objectName());
            return n + 1;
        }else
            return n;
    }
};

class Biyue: public PhaseChangeSkill{
public:
    Biyue():PhaseChangeSkill("biyue"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        if(diaochan->getPhase() == Player::Finish){
            Room *room = diaochan->getRoom();
            if(room->askForSkillInvoke(diaochan, objectName())){
                room->playSkillEffect(objectName());
                diaochan->drawCards(1);
            }
        }

        return false;
    }
};

class Mashu: public DistanceSkill{
public:
    Mashu():DistanceSkill("mashu")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

class Feiying: public DistanceSkill{
public:
    Feiying():DistanceSkill("feiying"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()))
            return +1;
        else
            return 0;
    }
};

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Indulgence");
    }
};

class Kongcheng: public ProhibitSkill{
public:
    Kongcheng():ProhibitSkill("kongcheng"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Slash") || card->inherits("Duel"))
            return to->isKongcheng();
        else
            return false;
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << CardResponsed << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(event == PhaseChange){
            if(lumeng->getPhase() == Player::Start){
                lumeng->setFlags("-keji_use_slash");
            }else if(lumeng->getPhase() == Player::Discard){
                if(!lumeng->hasFlag("keji_use_slash") &&
                   lumeng->getSlashCount() == 0 &&
                   lumeng->askForSkillInvoke("keji")){
                    lumeng->playSkillEffect("keji");
                    return true;
                }
            }
            return false;
        }
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("Slash"))
            lumeng->setFlags("keji_use_slash");
        return false;
    }
};

class Tiandu:public TriggerSkill{
public:
    Tiandu():TriggerSkill("tiandu"){
        frequency = Frequent;
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *guojia, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        Room *room = guojia->getRoom();
        if(guojia->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(guojia, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }
            guojia->obtainCard(judge->card);
            room->playSkillEffect(objectName());

            return true;
        }
        return false;
    }
};

class Jiang: public TriggerSkill{
public:
    Jiang():TriggerSkill("jiang"){
        events << CardUsed << CardEffected;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunce, QVariant &data) const{
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            card = effect.card;
        }
        if(card == NULL)
            return false;
        if(card->inherits("Duel") || (card->inherits("Slash") && card->isRed())){
            if(sunce->askForSkillInvoke(objectName(), data))
                sunce->drawCards(1);
        }
        return false;
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(3);
                target->turnOver();

                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

class Weimu: public ProhibitSkill{
public:
    Weimu():ProhibitSkill("weimu"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("TrickCard") && card->isBlack() && !card->inherits("Collateral");
    }
};

// card_skills
class TuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi";
    }
};

class Tuxi:public PhaseChangeSkill{
public:
    Tuxi():PhaseChangeSkill("tuxi"){
        view_as_skill = new TuxiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }
            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi", "@tuxi-card"))
                return true;
        }
        return false;
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect(objectName());

            int n = qMin(5, room->alivePlayerCount());
            room->doGuanxing(zhuge, room->getNCards(n, false), false);
        }

        return false;
    }
};

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());
        return lijian_card;
    }
};

class Quhu: public OneCardViewAsSkill{
public:
    Quhu():OneCardViewAsSkill("quhu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QuhuCard *card = new QuhuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponsed;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;
        Room *room = zhangjiao->getRoom();
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");
        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Damage << DamageDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(event == DamageDone && damage.from && damage.from->hasSkill("kuanggu") && damage.from->isAlive()){
            ServerPlayer *weiyan = damage.from;
            weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;
        }else if(event == Damage && player->hasSkill("kuanggu") && player->isAlive()){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            if(invoke){
                Room *room = player->getRoom();

                room->playSkillEffect(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class Tiaoxin: public ZeroCardViewAsSkill{
public:
    Tiaoxin():ZeroCardViewAsSkill("tiaoxin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("TiaoxinCard");
    }

    virtual const Card *viewAs() const{
        return new TiaoxinCard;
    }
};

class Mingce: public OneCardViewAsSkill{
public:
    Mingce():OneCardViewAsSkill("mingce"){
        default_choice = "draw";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MingceCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *c = to_select->getCard();
        return c->getTypeId() == Card::Equip || c->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MingceCard *card = new MingceCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Xinzhan: public ZeroCardViewAsSkill{
public:
    Xinzhan():ZeroCardViewAsSkill("xinzhan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XinzhanCard") && player->getHandcardNum() > player->getMaxHP();
    }

    virtual const Card *viewAs() const{
        return new XinzhanCard;
    }
};

class Xuanhuo: public OneCardViewAsSkill{
public:
    Xuanhuo():OneCardViewAsSkill("xuanhuo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XuanhuoCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XuanhuoCard *card = new XuanhuoCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Ganlu: public ZeroCardViewAsSkill{
public:
    Ganlu():ZeroCardViewAsSkill("ganlu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GanluCard");
    }

    virtual const Card *viewAs() const{
        return new GanluCard;
    }
};

class Beige: public TriggerSkill{
public:
    Beige():TriggerSkill("beige"){
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card == NULL || !damage.card->inherits("Slash") || damage.to->isDead())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *caiwenji = room->findPlayerBySkillName(objectName());
        if(caiwenji && !caiwenji->isNude() && caiwenji->askForSkillInvoke(objectName(), data)){
            room->askForDiscard(caiwenji, "beige", 1, false, true);

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.good = true;
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);

            switch(judge.card->getSuit()){
            case Card::Heart:{
                    RecoverStruct recover;
                    recover.who = caiwenji;
                    room->recover(player, recover);

                    break;
                }

            case Card::Diamond:{
                    player->drawCards(2);

                    break;
                }

            case Card::Club:{
                    if(damage.from && damage.from->isAlive()){
                        int to_discard = qMin(2, damage.from->getCardCount(true));
                        if(to_discard != 0)
                            room->askForDiscard(damage.from, "beige", to_discard, false, true);
                    }

                    break;
                }

            case Card::Spade:{
                    if(damage.from && damage.from->isAlive())
                        damage.from->turnOver();

                    break;
                }

            default:
                break;
            }
        }

        return false;
    }
};

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
            << new Tips(Card::Spade, 1)

            << new Analeptic(Card::NoSuit, 0);

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

    skills
            << new Wusheng << new Longdan << new Qixi
            << new Qingguo << new Yiyong << new Guose
            << new Duanliang << new Jijiu << new Jiuchi
            << new Huoji << new Kanpo << new Lianhuan << new Luanji
            ;
    skills
            << new Yingzi << new Biyue << new Qianxun
            << new Mashu << new Feiying << new Kongcheng
            << new Keji << new Tiandu << new Jiang
            << new Jushou << new Weimu << new Skill("qicai")
            ;
    skills
            << new Tuxi << new Guanxing << new Ganlu
            << new Fanjian << new Lijian << new Quhu
            << new Tiaoxin << new Leiji << new Xuanhuo
            << new Mingce << new Xinzhan << new Kuanggu << new Beige
            ;

    addMetaObject<TuxiCard>();
    addMetaObject<GanluCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<LijianCard>();
    addMetaObject<QuhuCard>();
    addMetaObject<TiaoxinCard>();
    addMetaObject<LeijiCard>();
    addMetaObject<XuanhuoCard>();
    addMetaObject<MingceCard>();
    addMetaObject<XinzhanCard>();
}

ADD_PACKAGE(Secrets);

