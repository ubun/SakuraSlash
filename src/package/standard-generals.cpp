#include "general.h"
#include "standard.h"
#include "standard-generals.h"
#include "standard-skillcards.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"

class XJQ: public TriggerSkill{
public:
    XJQ():TriggerSkill("xjq"){
        events << CardFinished;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *kudou, QVariant &data) const{
        Room *room = kudou->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash") && room->askForSkillInvoke(kudou, objectName(), data)){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = kudou;

            room->judge(judge);
            if(judge.isGood())
                room->askForUseCard(kudou, "slash", "@xjq");
        }
        return false;
    }
};

class JYT: public TriggerSkill{
public:
    JYT():TriggerSkill("jyt"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){
            Room *room = damage.to->getRoom();

            LogMessage log;
            log.type = "#JYTEffect";
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            room->sendLog(log);

            damage.damage = 0;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class WQ: public OneCardViewAsSkill{
public:
    WQ():OneCardViewAsSkill("wq"){
        frequency = Compulsory;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Diamond;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->isWeak() && pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("wq");

        return ncard;
    }
};

class FNF: public PhaseChangeSkill{
public:
    FNF():PhaseChangeSkill("fnf"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Draw && room->askForSkillInvoke(mouri, objectName())){
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = mouri;
            use.to << room->askForPlayerChosen(mouri, room->getAlivePlayers(), objectName());

            room->useCard(use);
            return true;
        }
        return false;
    }
};

class SQSJViewAsSkill: public OneCardViewAsSkill{
public:
    SQSJViewAsSkill():OneCardViewAsSkill("sqsj"){
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SQSJCard *card = new SQSJCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sqsj";
    }
};

class SQSJ:public TriggerSkill{
public:
    SQSJ():TriggerSkill("sqsj"){
        events << CardUsed;
        view_as_skill = new SQSJViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yueying, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        if(yueying->isWeak() && card->inherits("Slash") && use.to.length() == 1){
            Room *room = yueying->getRoom();
            use.to.first()->setFlags("Sq1");
            if(room->askForUseCard(yueying, "@@sqsj", "@sqsj")){
                room->playSkillEffect(objectName());
                foreach(ServerPlayer *tmp, room->getOtherPlayers(use.to.first())){
                    if(tmp->hasFlag("Sqsj")){
                        tmp->setFlags("-Sqsj");
                        use.to << tmp;
                    }
                }
                data = QVariant::fromValue(use);
            }
            use.to.first()->setFlags("-Sq1");
        }
        return false;
    }
};

class ZZ:public TriggerSkill{
public:
    ZZ():TriggerSkill("zz"){
        events << DamageComplete;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *mori, QVariant &data) const{
        Room *room = mori->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from)
            return false;
        if(room->askForSkillInvoke(mori, objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = mori;

            room->judge(judge);
            if(judge.isGood())
                damage.from->setPhase(Player::Discard);
        }
        return false;
    }
};

class TM:public MasochismSkill{
public:
    TM():MasochismSkill("tm"){
    }

    virtual void onDamaged(ServerPlayer *mori, const DamageStruct &damage) const{
        Room *room = mori->getRoom();
        if(!mori->isWeak())
            return;
        if(room->askForSkillInvoke(mori, objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = mori;

            room->judge(judge);
            if(judge.isGood()){
                foreach(ServerPlayer *tmp, room->getOtherPlayers(mori)){
                    if(mori->distanceTo(tmp) <= 1)
                        room->loseHp(tmp);
                }
            }
        }
    }
};

class SS: public TriggerSkill{
public:
    SS():TriggerSkill("ss"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isFemale();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xiang = room->findPlayerBySkillName(objectName());
        if(!xiang || !xiang->askForSkillInvoke(objectName()))
            return false;
        if(!room->askForDiscard(xiang, objectName(), 2, false, true))
            room->loseHp(xiang);
        return true;
    }
};

class EMFJ: public TriggerSkill{
public:
    EMFJ():TriggerSkill("emfj"){
        events << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getSuit() != Card::Club)
            return false;
        Room *room = lubu->getRoom();
        room->playSkillEffect(objectName());

        QString slasher = lubu->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher);

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
        }

        room->slashResult(effect, jink);
        return true;
    }
};

//dc skill card

class XZM: public OneCardViewAsSkill{
public:
    XZM():OneCardViewAsSkill("xzm"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class JM:public TriggerSkill{
public:
    JM():TriggerSkill("jm"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isWeak();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard") && card->isBlack()){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

//hq like buqu
//cs

class JT: public OneCardViewAsSkill{
public:
    JT():OneCardViewAsSkill("jt"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

//lb

//hk

//ts

class YS: public OneCardViewAsSkill{
public:
    YS():OneCardViewAsSkill("ys"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach");
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

//wny

class CN: public OneCardViewAsSkill{
public:
    CN():OneCardViewAsSkill("cn"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ExNihilo *ex = new ExNihilo(first->getSuit(), first->getNumber());
        ex->addSubcard(first->getId());
        ex->setSkillName(objectName());
        return ex;
    }
};

class JDXJ: public ProhibitSkill{
public:
    JDXJ():ProhibitSkill("jdxj"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Indulgence");
    }
};

//slx

class Tianzhen: public TriggerSkill{
public:
    Tianzhen():TriggerSkill("tianzhen"){
        events << Predamaged << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ayumi, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("TrickCard")){
            LogMessage log;
            log.type = "#TianzhenPrevent";
            log.from = ayumi;
            log.to << damage.to;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            ayumi->getRoom()->sendLog(log);
            return true;
        }
        return false;
    }
};

class Lanman: public OneCardViewAsSkill{
public:
    Lanman():OneCardViewAsSkill("lanman"){
    }

    virtual bool isEnabledAtPlay(const Player *yoshida) const{
        return yoshida->getHandcardNum() < 4;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ExNihilo *exnihilo = new ExNihilo(first->getSuit(), first->getNumber());
        exnihilo->addSubcard(first->getId());
        exnihilo->setSkillName(objectName());
        return exnihilo;
    }
};

class Dontcry: public PhaseChangeSkill{
public:
    Dontcry():PhaseChangeSkill("dontcry"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("cry") < 4
                && target->isAlive();
    }

    virtual bool onPhaseChange(ServerPlayer *ayumi) const{
        Room *room = ayumi->getRoom();
        if(ayumi->getPhase() == Player::Start){
            ayumi->addMark("cry");
        }else if(ayumi->getPhase() == Player::Discard){
            if(ayumi->getMark("cry") == 1)
                //room->playSkillEffect(objectName());
                return true;
        }else if(ayumi->getPhase() == Player::Finish){
            if(ayumi->getMark("cry") == 2 && ayumi->isAlive()){
                room->setPlayerProperty(ayumi, "maxhp", ayumi->getMaxHP()+1);
                room->broadcastInvoke("animate", "lightbox:$dontcry");
                room->setPlayerProperty(ayumi, "hp", ayumi->getMaxHP());

                LogMessage log;
                log.type = "#BukuWake";
                log.from = ayumi;
                log.arg = objectName();
                room->sendLog(log);
            }
        }
        return false;
    }
};

class DuorenViewAsSkill:public OneCardViewAsSkill{
public:
    DuorenViewAsSkill():OneCardViewAsSkill("duoren"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed() && !to_select->isEquipped();
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

class Duoren: public TriggerSkill{
public:
    Duoren():TriggerSkill("duoren"){
        events << CardResponsed;
        view_as_skill = new DuorenViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *mouriran, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        Room *room = mouriran->getRoom();
        ServerPlayer *source = room->getCurrent();
        QVariant tgt = QVariant::fromValue(source);
        if(source->getWeapon() && room->askForSkillInvoke(mouriran, "duoren", tgt))
            mouriran->obtainCard(source->getWeapon());
        return false;
    }
};

class Shouhou: public PhaseChangeSkill{
public:
    Shouhou():PhaseChangeSkill("shouhou"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Start){
            QList<ServerPlayer *> hurts;
            foreach(ServerPlayer *player, room->getAlivePlayers())
                if(player->isWounded())
                    hurts << player;
            if(!hurts.isEmpty() && mouri->askForSkillInvoke(objectName())){
                ServerPlayer *target = room->askForPlayerChosen(mouri, hurts, "shouhou");
                if(target){
                    RecoverStruct recover;
                    recover.card = NULL;
                    recover.who = mouri;
                    room->recover(target, recover);

                    mouri->skip(Player::Judge);
                    mouri->skip(Player::Draw);
                }
            }
        }
        return false;
    }
};

class Heqi:public TriggerSkill{
public:
    Heqi():TriggerSkill("heqi"){
        events << CardEffected;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *kazuha, QVariant &data) const{
        Room *room = kazuha->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Slash"))
            return false;
        if(kazuha->getHandcardNum() > effect.from->getCardCount(true))
            return false;
        if(effect.from->getJudgingArea().isEmpty() && effect.from->isKongcheng())
            return false;
        if(room->askForSkillInvoke(kazuha, objectName(), data)){
            int card_id = room->askForCardChosen(kazuha, effect.from, "hj", objectName());
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), kazuha, Player::Hand, false);
            else
                room->obtainCard(kazuha, card_id);
            //room->playSkillEffect(objectName());
        }
        return false;
    }
};

ShouqiuCard::ShouqiuCard(){
    target_fixed = true;
}

void ShouqiuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

class ShouqiuViewAsSkill:public OneCardViewAsSkill{
public:
    ShouqiuViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@shouqiu";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ShouqiuCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Shouqiu: public TriggerSkill{
public:
    Shouqiu():TriggerSkill("shouqiu"){
        view_as_skill = new ShouqiuViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@shouqiu-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@shouqiu", prompt, false);

        if(card){
            // the only difference for Guicai & Guidao
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class Shenyong: public TriggerSkill{
public:
    Shenyong():TriggerSkill("shenyong"){
        events << Damage << DamageComplete;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *kyo, QVariant &data) const{
        if(kyo->getPhase() != Player::Play)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->inherits("Slash")){
            return false;
        }
        Room *room = kyo->getRoom();
        if(event == DamageComplete){
            foreach(ServerPlayer *tmp, room->getAllPlayers())
                tmp->setFlags("-shenyong");
            return false;
        }
        damage.to->setFlags("shenyong");
        QList<ServerPlayer *>players = room->getOtherPlayers(damage.to);

        QMutableListIterator<ServerPlayer *> itor(players);
        while(itor.hasNext()){
            itor.next();
            if(!damage.to->inMyAttackRange(itor.value()) || itor.value()->hasFlag("shenyong"))
                itor.remove();
        }
        if(players.isEmpty() || !room->askForSkillInvoke(kyo, objectName(), data))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(kyo, players, objectName());
        if(target){
            CardEffectStruct effect;
            Slash *slash = new Slash(damage.card->getSuit(), damage.card->getNumber());
            slash->setSkillName("shenyong_slash");
            effect.card = slash;
            effect.from = kyo;
            effect.to = target;
            room->cardEffect(effect);
        }
        return false;
    }
};

class ShentouViewAsSkill: public OneCardViewAsSkill{
public:
    ShentouViewAsSkill():OneCardViewAsSkill("shentou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class Shentou: public PhaseChangeSkill{
public:
    Shentou():PhaseChangeSkill("shentou"){
        view_as_skill = new ShentouViewAsSkill;
    }
    virtual int getPriority() const{
        return 2;
    }
    virtual bool onPhaseChange(ServerPlayer *kid) const{
        if(kid->getPhase() == Player::Discard){
            if(kid->usedTimes("Snatch") > 2 && kid->askForSkillInvoke(objectName()))
                return true;
        }
        return false;
    }
};

BaiyiCard::BaiyiCard(){
    once = true;
}

bool BaiyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void BaiyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->showAllCards(source, targets.first());
}

class Baiyi: public OneCardViewAsSkill{
public:
    Baiyi():OneCardViewAsSkill("baiyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("BaiyiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        BaiyiCard *card = new BaiyiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Feixing: public DistanceSkill{
public:
    Feixing():DistanceSkill("feixing"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()) && to->getEquips().length() == 0)
            return +1;
        else
            return 0;
    }
};

class Yirong:public TriggerSkill{
public:
    Yirong():TriggerSkill("yirong"){
        events << AskForPeachesDone;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@yaiba") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *sharon, QVariant &) const{
        Room *room = sharon->getRoom();
        if(sharon->getHp() <= 0 && sharon->askForSkillInvoke(objectName())){
            sharon->loseMark("@yaiba");
            room->broadcastInvoke("animate", "lightbox:$yirong");

            QStringList genlist = Sanguosha->getLimitedGeneralNames();
            foreach(ServerPlayer *player, room->getAllPlayers()){
                genlist.removeOne(player->getGeneralName());
            }

            QString general = room->askForGeneral(sharon, genlist);
            room->transfigure(sharon, general, false, false);
            //room->acquireSkill(sharon, "yirong", false);
            sharon->getRoom()->setPlayerProperty(sharon, "hp", 3);
            return true;
        }
        return false;
    }
};

class Wuyu: public TriggerSkill{
public:
    Wuyu():TriggerSkill("wuyu"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *s, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = s;
            log.arg = objectName();
            s->getRoom()->sendLog(log);

            s->drawCards(2);
        }
        return false;
    }
};

class Quzheng: public TriggerSkill{
public:
    Quzheng():TriggerSkill("quzheng"){
        events << CardDiscarded;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("quzheng");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *megure = room->findPlayerBySkillName(objectName());
        if(!megure)
            return false;

        if(player->getPhase() == Player::Discard){
            CardStar card = data.value<CardStar>();
            QList<int> card_ids;
            foreach(int tmp, card->getSubcards())
                if(!Sanguosha->getCard(tmp)->inherits("Weapon"))
                    card_ids << tmp;

            if(card_ids.isEmpty() || !room->askForSkillInvoke(megure, objectName(), data))
                return false;
            if(megure->hasLordSkill("ranglu")){
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(megure)){
                    if(tmp != player && tmp->getKingdom() == "zhen"){
                        players << tmp;
                    }
                }
                if(!players.isEmpty() && room->askForSkillInvoke(megure, "ranglu", data)){
                    megure = room->askForPlayerChosen(megure, players, "ranglu");
                }
            }
            foreach(int card_id, card_ids){
                room->obtainCard(megure, card_id);
            }
        }
        return false;
    }
};

class QuzhengSkip: public PhaseChangeSkill{
public:
    QuzhengSkip():PhaseChangeSkill("#quzheng_skip"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Draw;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getHandcardNum() > player->getMaxHP())
            return true;
        return false;
    }
};

class Shangchi: public PhaseChangeSkill{
public:
    Shangchi():PhaseChangeSkill("shangchi"){
        frequency = Frequent;
        default_choice = "me";
    }

    virtual bool onPhaseChange(ServerPlayer *matsumoto) const{
        if(matsumoto->getPhase() == Player::Draw && matsumoto->isWounded() && matsumoto->askForSkillInvoke(objectName())){
            Room *room = matsumoto->getRoom();
            if(room->askForChoice(matsumoto, objectName(), "me+him") == "me")
                room->drawCards(matsumoto, matsumoto->getLostHp());
            else {
                ServerPlayer *target = room->askForPlayerChosen(matsumoto, room->getOtherPlayers(matsumoto), objectName());
                target->drawCards(matsumoto->getHp() - 1);
            }
        }

        return false;
    }
};

DiaobingCard::DiaobingCard(){
    once = true;
}

bool DiaobingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    return targets.isEmpty();
}

void DiaobingCard::use(Room *room, ServerPlayer *matsu, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges("jing", matsu);
    const Card *slash = NULL;
    foreach(ServerPlayer *liege, lieges){
        slash = room->askForCard(liege, ".At", "@diaobing-slash", QVariant::fromValue(matsu));
        if(slash && (!targets.first()->isKongcheng() || !slash->inherits("FireAttack"))){
            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = liege;
            card_use.to << targets.first();
            room->useCard(card_use);
        }
    }
}

class AttackPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("Slash") ||
                card->inherits("FireAttack") ||
                card->inherits("Duel");
    }
};

class Diaobing: public ZeroCardViewAsSkill{
public:
    Diaobing():ZeroCardViewAsSkill("diaobing$"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("diaobing") && !player->hasUsed("DiaobingCard");
    }

    virtual const Card *viewAs() const{
        return new DiaobingCard;
    }
};
/*
class Qinjian: public TriggerSkill{
public:
    Qinjian():TriggerSkill("qinjian"){
        events << SlashMissed;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash && player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;

            Room *room = player->getRoom();
            room->judge(judge);
            if(judge.isBad())
                return false;
            player->obtainCard(effect.slash);
            room->askForUseCard(player, "slash", "@qinjian-slash");
        }
        return false;
    }
};
*/
class Tishen: public TriggerSkill{
public:
    Tishen():TriggerSkill("tishen"){
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@fake") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *kaitou, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != kaitou)
            return false;

        Room *room = kaitou->getRoom();
        if(kaitou->askForSkillInvoke(objectName(), data)){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = kaitou;

            room->judge(judge);

            if(judge.isGood()){
                room->broadcastInvoke("animate", "lightbox:$tishen");
                kaitou->loseMark("@fake");
                room->setPlayerProperty(kaitou, "hp", 3);
                return true;
            }
        }

        return false;
    }
};

MoshuCard::MoshuCard(){
    target_fixed = true;
}

void MoshuCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &) const{
    room->moveCardTo(this, NULL, Player::DrawPile, true);
}

class MoshuViewAsSkill: public ViewAsSkill{
public:
    MoshuViewAsSkill():ViewAsSkill("moshu"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        MoshuCard *card = new MoshuCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@moshu!";
    }
};

class Moshu: public TriggerSkill{
public:
    Moshu():TriggerSkill("moshu"){
        events << PhaseChange;
        view_as_skill = new MoshuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *kaitou = room->findPlayerBySkillName(objectName());
        if(!kaitou || player == kaitou)
            return false;
        if(player->getPhase() == Player::Draw && !kaitou->hasFlag("MagicUsed")
           && kaitou->askForSkillInvoke(objectName(), QVariant::fromValue(player))){
            LogMessage log;
            log.type = "#Moshu";
            log.from = kaitou;
            log.arg2 = objectName();

            kaitou->drawCards(2);
            if(!room->askForUseCard(kaitou, "@@moshu!", "@moshu-card")){
                room->moveCardTo(kaitou->getHandcards().last(), NULL, Player::DrawPile, true);
                room->moveCardTo(kaitou->getHandcards().last(), NULL, Player::DrawPile, true);
            }

            log.arg = QString::number(2);

            room->sendLog(log);
            room->setPlayerFlag(kaitou, "MagicUsed");
        }
        else if(player == kaitou && player->getPhase() == Player::Start){
            room->setPlayerFlag(kaitou, "-MagicUsed");
        }
        return false;
    }
};

RenxingCard::RenxingCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool RenxingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->getHp() < Self->getHp())
        return false;
    if(to_select->isKongcheng())
        return false;
    return true;
}

void RenxingCard::use(Room *room, ServerPlayer *aoko, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    bool success = aoko->pindian(target, objectName(), this);
    if(success){
        if(target->isNude())
            return;
        int card_id = room->askForCardChosen(aoko, target, "he", "renxing");
        room->obtainCard(aoko, card_id);
    }else{
        room->loseHp(aoko);
    }
}

class Renxing: public OneCardViewAsSkill{
public:
    Renxing():OneCardViewAsSkill("renxing"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("RenxingCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new RenxingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Qingmei: public TriggerSkill{
public:
    Qingmei():TriggerSkill("qingmei"){
        events << Pindian;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *aoko = room->findPlayerBySkillName(objectName());
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->from != aoko && pindian->to != aoko)
            return false;
        Card *pdcd;
        if(pindian->from == aoko && pindian->to_card->inherits("BasicCard")){
            pdcd = Sanguosha->cloneCard(pindian->to_card->objectName(), pindian->to_card->getSuit(), 1);
            pdcd->addSubcard(pindian->to_card);
            pdcd->setSkillName(objectName());
            pindian->to_card = pdcd;
        }
        else if(pindian->to == aoko && pindian->from_card->inherits("BasicCard")){
            pdcd = Sanguosha->cloneCard(pindian->from_card->objectName(), pindian->from_card->getSuit(), 1);
            pdcd->addSubcard(pindian->from_card);
            pdcd->setSkillName(objectName());
            pindian->from_card = pdcd;
        }

        LogMessage log;
        log.type = "#Qingmeieff";
        log.from = aoko;
        log.arg = objectName();
        room->sendLog(log);

        data = QVariant::fromValue(pindian);
        return false;
    }
};

AnshaCard::AnshaCard(){
    once = true;
    target_fixed = true;
}

void AnshaCard::use(Room *room, ServerPlayer *gin, const QList<ServerPlayer *> &) const{
    ServerPlayer *target = room->askForPlayerChosen(gin, room->getOtherPlayers(gin), "ansha");
    if(target && gin->getMark("@ansha") > 0){
        target->getRoom()->setPlayerMark(target, "anshamark", 1);
        room->throwCard(this);
        gin->loseMark("@ansha");
    }
}

class AnshaViewAsSkill: public ViewAsSkill{
public:
    AnshaViewAsSkill():ViewAsSkill("ansha"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@ansha") > 0;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{

        if(selected.length() >= 4)
            return false;
        if(to_select->isEquipped())
            return false;
        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getSuit() == item->getFilteredCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 4){
            AnshaCard *card = new AnshaCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Ansha: public PhaseChangeSkill{
public:
    Ansha():PhaseChangeSkill("ansha"){
        view_as_skill = new AnshaViewAsSkill;
        frequency = Limited;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("anshamark") > 0;
    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish){
            room->setPlayerMark(player, "anshamark", 0);
            ServerPlayer *gin = room->findPlayerBySkillName(objectName());
            if(gin){
                LogMessage log;
                log.type = "#Ansha";
                log.from = gin;
                log.to << player;
                log.arg = objectName();
                room->sendLog(log);

                DamageStruct damage;
                damage.from = gin;
                damage.to = player;
                damage.damage = 3;
                room->setEmotion(gin, "good");
                room->broadcastInvoke("animate", "lightbox:$ansha");
                room->setEmotion(player, "bad");
                room->loseMaxHp(gin);
                room->damage(damage);
            }
        }
        return false;
    }
};

class Juelu: public TriggerSkill{
public:
    Juelu():TriggerSkill("juelu"){
        events << Damage;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gin, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()){
            Room *room = gin->getRoom();
            const Card *card = room->askForCard(gin, "slash", "juelu-slash");
            if(card){
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = gin;
                log.arg = objectName();
                room->sendLog(log);
                if(gin->hasFlag("drank"))
                    room->setPlayerFlag(gin, "-drank");
                room->cardEffect(card, gin, damage.to);
            }
        }
        return false;
    }
};

class Heiyi: public PhaseChangeSkill{
public:
    Heiyi():PhaseChangeSkill("heiyi$"){
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "hei";
    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Draw)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *gin = room->getLord();
        if(gin->hasLordSkill(objectName())){
            if(player != gin && room->askForSkillInvoke(player, "heiyi", QVariant::fromValue(gin))){
                gin->gainMark("@heiyi");
                return true;
            }
            else if(player == gin){
                for(; gin->getMark("@heiyi") > 0; gin->loseMark("@heiyi")){
                    gin->drawCards(2);
                }
            }
        }
        return false;
    }
};

MaixiongCard::MaixiongCard(){
    will_throw = false;
}

bool MaixiongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self || to_select->getMark("daoh") != 0)
        return false;

    return true;
}

void MaixiongCard::use(Room *room, ServerPlayer *vodka, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Hand, false);
    room->setPlayerMark(target, "daoh", 1);

    int old_value = vodka->getMark("maixiong");
    int new_value = old_value + subcards.length();
    vodka->setMark("maixiong", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = vodka;
        room->recover(vodka, recover);
    }
}

class MaixiongViewAsSkill:public OneCardViewAsSkill{
public:
    MaixiongViewAsSkill():OneCardViewAsSkill("maixiong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MaixiongCard *card = new MaixiongCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Maixiong: public PhaseChangeSkill{
public:
    Maixiong():PhaseChangeSkill("maixiong"){
        view_as_skill = new MaixiongViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start)
            target->setMark("maixiong", 0);
        else if(target->getPhase() == Player::NotActive)
            target->getRoom()->setPlayerMark(target, "daoh", 0);
        return false;
    }
};

class Dashou: public TriggerSkill{
public:
    Dashou():TriggerSkill("dashou"){
        events << PhaseChange;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish && player->askForSkillInvoke(objectName(), data)){
            foreach(ServerPlayer *other, room->getOtherPlayers(player)){
                const Card *card = room->askForCard(other, ".", "@dashou-get:" + player->objectName(), QVariant::fromValue(player));
                if(card){
                    player->obtainCard(card);
                    player->addMark("dashou");
                }
            }
            if(player->getMark("dashou")>2)
                player->turnOver();
        }
        player->setMark("dashou",0);
        return false;
    }
};

class Jushen:public TriggerSkill{
public:
    Jushen():TriggerSkill("jushen"){
        frequency = Compulsory;
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *akai, QVariant &data) const{
        Room *room = akai->getRoom();
        const Weapon *weapon = akai->getWeapon();
        if(weapon && weapon->getRange() > 2){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            LogMessage log;
            log.arg = objectName();
            log.type = "#Jushenslash";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);

            effect.from->getRoom()->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class Xunzhi: public TriggerSkill{
public:
    Xunzhi():TriggerSkill("xunzhi"){
        events << Death << Damaged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shuichi, QVariant &data) const{
        if(event == Damaged){
            if(!shuichi->hasSkill("chevyCK"))
                return false;
            Room *room = shuichi->getRoom();
            DamageStruct damage = data.value<DamageStruct>();
            if(room->askForSkillInvoke(shuichi, "chevyCK", data)){
                DamageStruct damage2 = damage;
                room->killPlayer(shuichi, &damage2);
            }
            return false;
        }
        QList<ServerPlayer *> targets;
        ServerPlayer *target;
        Room *room = shuichi->getRoom();

        DamageStar damage = data.value<DamageStar>();
        if(damage && damage->from && damage->to == shuichi && damage->from != shuichi) //not zisha
            target = damage->from;
        else
            target = room->askForPlayerChosen(shuichi, room->getAlivePlayers(), objectName());
        targets << target;
        if(!room->getOtherPlayers(target).isEmpty())
            targets << room->askForPlayerChosen(shuichi, room->getOtherPlayers(target), objectName());

        foreach(target, targets){
            target->gainMark("@aka",1);
            target->acquireSkill("xunzhiresult");
        }
        return false;
    }
};

class Xunzhiresult:public TriggerSkill{
public:
    Xunzhiresult():TriggerSkill("#xunzhiresult"){
        events << DrawNCards;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@aka")>0;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        LogMessage log;
        log.type = "#Xunzhieffect";
        log.from = player;
        log.arg = "xunzhi";
        data = data.toInt() - 1;
        player->getRoom()->sendLog(log);
        return false;
    }
};

GaizaoCard::GaizaoCard(){
}

bool GaizaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    return targets.isEmpty() && to_select->hasEquip();
}

void GaizaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    room->throwCard(room->askForCardChosen(effect.from, effect.to, "e", "gaizao"));
    effect.from->drawCards(1);
    effect.to->drawCards(1);
}

class GaizaoViewAsSkill: public ZeroCardViewAsSkill{
public:
    GaizaoViewAsSkill():ZeroCardViewAsSkill("gaizao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@gaizao";
    }

    virtual const Card *viewAs() const{
        return new GaizaoCard;
    }
};

class Gaizao:public TriggerSkill{
public:
    Gaizao():TriggerSkill("gaizao"){
        events << PhaseChange;
        view_as_skill = new GaizaoViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *hakasi, QVariant &data) const{
        if(hakasi->getPhase() != Player::Draw)
            return false;

        Room *room = hakasi->getRoom();
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->hasEquip()){
                room->askForUseCard(hakasi, "@@gaizao", "@gaizao");
                break;
            }
        }
        /*
        QList<ServerPlayer *> targets;
        if(player->hasEquip())
            targets << player;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(tmp->hasEquip()){
                targets << tmp;
            }
        }
        if(!targets.isEmpty() && room->askForSkillInvoke(player, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());

        }*/
        return false;
    }
};

class Suyuan: public TriggerSkill{
public:
    Suyuan():TriggerSkill("suyuan"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *agasa = room->findPlayerBySkillName(objectName());
        if(!agasa)
            return false;
        if((damage.from == agasa || damage.to == agasa) && room->askForSkillInvoke(agasa, objectName(), data)){
            damage.from = room->askForPlayerChosen(agasa, room->getOtherPlayers(damage.from), objectName());
            LogMessage log;
            log.type = "#SuyuanChange";
            log.from = agasa;
            log.to << damage.from;
            room->sendLog(log);
            if(agasa->hasSkill("beetle") && room->askForSkillInvoke(agasa, "beetle", data)){
                damage.to = room->askForPlayerChosen(agasa, room->getOtherPlayers(damage.to), "beetle");

                LogMessage log;
                log.type = "#BeetleChange";
                log.from = agasa;
                log.to << damage.to;
                room->sendLog(log);
                room->damage(damage);
                return true;
            }
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Baomu: public TriggerSkill{
public:
    Baomu():TriggerSkill("baomu$"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();

        if(dying_data.who->getKingdom() != "shao")
            return false;
        Room *room = player->getRoom();
        ServerPlayer *cancer = room->findPlayerBySkillName(objectName());

        if(cancer && room->askForSkillInvoke(cancer, objectName(), QVariant::fromValue(dying_data.who))){
            const Card *recovcd = room->askForCard(cancer, ".S", "@baomu:" + dying_data.who->objectName());
            if(!recovcd)
                return false;
            RecoverStruct recover;
            recover.who = cancer;
            recover.card = recovcd;
            room->recover(dying_data.who, recover);
        }
        return false;
    }
};

YuandingCard::YuandingCard(){
    will_throw = false;
}

bool YuandingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && !to_select->hasFlag("gardener");
}

void YuandingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->showCard(effect.from, effect.card->getId(), effect.to);
    const Card *cart = room->askForCard(effect.to, ".", "@yuandingask:" + effect.from->objectName(), false);
    if(cart){
        effect.from->obtainCard(cart);
        effect.to->obtainCard(effect.card);
        room->setPlayerFlag(effect.to, "gardener");
    }
}

class YuandingViewAsSkill:public OneCardViewAsSkill{
public:
    YuandingViewAsSkill():OneCardViewAsSkill("yuanding"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YuandingCard *card = new YuandingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Yuanding: public PhaseChangeSkill{
public:
    Yuanding():PhaseChangeSkill("yuanding"){
        view_as_skill = new YuandingViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *sumiko) const{
        if(sumiko->getPhase() == Player::NotActive){
            foreach(ServerPlayer *tmp, sumiko->getRoom()->getAllPlayers())
                if(tmp->hasFlag("gardener"))
                    sumiko->getRoom()->setPlayerFlag(tmp, "-gardener");
        }
        return false;
    }
};

class Qiniao: public PhaseChangeSkill{
public:
    Qiniao():PhaseChangeSkill("qiniao"){
    }

    virtual bool onPhaseChange(ServerPlayer *sumiko) const{
        if(sumiko->getPhase() == Player::Discard)
           sumiko->setMark("qiniao", sumiko->getHandcardNum());
        if(sumiko->getPhase() == Player::Finish){
           if(sumiko->getMark("qiniao") - sumiko->getHandcardNum() < 2 && sumiko->askForSkillInvoke(objectName())){
               Room *room = sumiko->getRoom();
               ServerPlayer *target = room->askForPlayerChosen(sumiko, room->getAlivePlayers(), objectName());
               target->gainMark("@bird");
           }
        }
        return false;
    }
};

class QiniaoSkip: public TriggerSkill{
public:
    QiniaoSkip():TriggerSkill("#qiniaoskip"){
        events << PhaseChange;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@bird")>0;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start){
            player->skip(Player::Judge);
            player->loseMark("@bird");
        }
        return false;
    }
};

class Biaoche: public DistanceSkill{
public:
    Biaoche():DistanceSkill("biaoche"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        //from->parent()->findChildren<const Player *>()
        if(from->hasSkill(objectName()) && to->getHp() > from->getHp())
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() < from->getHp())
            correct ++;
        return correct;
    }
};

JingshenCard::JingshenCard(){
    once = true;
    target_fixed = true;
}

bool JingshenCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() < 2;
}

void JingshenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const QList<int> &jipin = source->getPile("jipin");
    source->addToPile("jipin", this->getSubcards().first(), false);
    if(jipin.length() >= room->getAlivePlayers().length() - 1){
        ServerPlayer *target = room->askForPlayerChosen(source, room->getAlivePlayers(), "jingshen");
        if(!target) target = source;
        foreach(int card_id, source->getPile("jipin"))
            room->obtainCard(target, card_id);
/*        room->fillAG(jipin);
        QList<ServerPlayer *> players;
        players << source;
        players << room->getOtherPlayers(source);
        players = players.mid(0, room->getAlivePlayers().length());
        foreach(ServerPlayer *player, players){
           int card_id = room->askForAG(player, jipin, false, objectName());
           //source->removeCardFromPile("jipin", card_id);
           room->throwCard(card_id);
           room->takeAG(player, card_id);
        }
        room->broadcastInvoke("clearAG");
*/    }
}

class Jingshen: public OneCardViewAsSkill{
public:
    Jingshen():OneCardViewAsSkill("jingshen"){
    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JingshenCard");
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
    virtual const Card *viewAs(CardItem *card_item) const{
        JingshenCard *card = new JingshenCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Long: public TriggerSkill{
public:
    Long():TriggerSkill("long"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *aoyamagoushou, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = aoyamagoushou->getRoom();
        if(room->askForSkillInvoke(aoyamagoushou, objectName())){
            room->playSkillEffect(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):([2357JK])");
            judge.good = true;
            judge.reason = objectName();
            judge.who = aoyamagoushou;

            room->judge(judge);
            if(judge.isGood()){
                damage.to->drawCards(2);
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }else{
                judge.pattern = QRegExp("(.*):(.*):([A])");
                judge.good = true;
                judge.reason = objectName();
                judge.who = aoyamagoushou;
                if(judge.isGood()){
                  damage.damage +=2;
                  data = QVariant::fromValue(damage);
                } else {
                    aoyamagoushou->drawCards(1);
                    return true;
                }
            }
        }
        return false;
    }
};

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

void StandardPackage::addGenerals(){
    General *lufei = new General(this, "lufei", "zhen");
    lufei->addSkill(new XJQ);
    lufei->addSkill(new JYT);
    lufei->addSkill(new WQ);

    General *suolong = new General(this, "suolong", "zhen");
    suolong->addSkill(new FNF);
    suolong->addSkill(new SQSJ);

    General *weiwei = new General(this, "weiwei", "zhen");
    weiwei->addSkill(new ZZ);
    weiwei->addSkill(new TM);

    General *xiangji = new General(this, "xiangji", "zhen");
    xiangji->addSkill(new SS);
    xiangji->addSkill(new EMFJ);
    //xiangji->addSkill(new DC);

    General *namei = new General(this, "namei", "zhen");
    namei->addSkill(new XZM);
    namei->addSkill(new JM);

    //General *buluke = new General(this, "buluke", "zhen");
    //buluke->addSkill(new HQ);
    //buluke->addSkill(new CS);

    General *fulanqi = new General(this, "fulanqi", "zhen");
    fulanqi->addSkill(new JT);
    //fulanqi->addSkill(new LB);

    //General *luobin = new General(this, "luobin", "zhen");
    //luobin->addSkill(new HK);
    //luobin->addSkill(new TS);

    General *qiaoba = new General(this, "qiaoba", "zhen");
    qiaoba->addSkill(new YS);
    //qiaoba->addSkill(new WNY);

    General *wusuopu = new General(this, "wusuopu", "zhen");
    wusuopu->addSkill(new CN);
    wusuopu->addSkill(new JDXJ);
    //wusuopu->addSkill(new SLX);

    General *megurejyuuzou, *matsumotokiyonaka, *otagiritoshirou;
    megurejyuuzou = new General(this, "megurejyuuzou", "jing");
    megurejyuuzou->addSkill(new Quzheng);
    megurejyuuzou->addSkill(new QuzhengSkip);
    related_skills.insertMulti("quzheng", "#quzheng_skip");
    megurejyuuzou->addSkill(new Skill("ranglu$"));

    matsumotokiyonaka = new General(this, "matsumotokiyonaka$", "jing");
    matsumotokiyonaka->addSkill(new Shangchi);
    matsumotokiyonaka->addSkill(new Diaobing);

    otagiritoshirou = new General(this, "otagiritoshirou", "jing");
    otagiritoshirou->addSkill(new Skill("qinjian", Skill::Compulsory));

    General *kurobakaitou, *nakamoriaoko;
    kurobakaitou = new General(this, "kurobakaitou", "guai");
    kurobakaitou->addSkill(new Tishen);
    kurobakaitou->addSkill(new MarkAssignSkill("@fake", 1));
    related_skills.insertMulti("tishen", "#@fake-1");
    //kurobakaitou->addSkill(new MarkAssignSkill("magic", 1));
    kurobakaitou->addSkill(new Moshu);

    nakamoriaoko = new General(this, "nakamoriaoko", "guai", 4, false);
    nakamoriaoko->addSkill(new Renxing);
    nakamoriaoko->addSkill(new Qingmei);

    General *gin, *vodka, *akaishuichi;
    gin = new General(this, "gin$", "hei");
    gin->addSkill(new Ansha);
    gin->addSkill(new MarkAssignSkill("@ansha", 1));
    related_skills.insertMulti("ansha", "#@ansha-1");
    gin->addSkill(new Juelu);
    gin->addSkill(new Heiyi);

    vodka = new General(this, "vodka", "hei");
    vodka->addSkill(new Maixiong);
    vodka->addSkill(new Dashou);

    akaishuichi = new General(this, "akaishuichi", "te");
    akaishuichi->addSkill(new Jushen);
    akaishuichi->addSkill(new Xunzhi);
    akaishuichi->addSkill(new Xunzhiresult);
    related_skills.insertMulti("xunzhi", "#xunzhiresult");

    General *agasahiroshi, *kobayashisumiko, *yamamuramisae, *aoyamagoushou;

    agasahiroshi = new General(this, "agasahiroshi$", "za");
    agasahiroshi->addSkill(new Gaizao);
    agasahiroshi->addSkill(new Suyuan);
    agasahiroshi->addSkill(new Baomu);

    kobayashisumiko = new General(this, "kobayashisumiko", "za", 4, false);
    kobayashisumiko->addSkill(new Yuanding);
    kobayashisumiko->addSkill(new Qiniao);
    kobayashisumiko->addSkill(new QiniaoSkip);
    related_skills.insertMulti("qiniao", "#qiniaoskip");

    yamamuramisae = new General(this, "yamamuramisae", "za", 3, false);
    yamamuramisae->addSkill(new Biaoche);
    yamamuramisae->addSkill(new Jingshen);

    aoyamagoushou = new General(this, "aoyamagoushou", "god");
    aoyamagoushou->addSkill(new Long);

    // for skill cards
    addMetaObject<SQSJCard>();
    addMetaObject<ShouqiuCard>();
    addMetaObject<BaiyiCard>();
    addMetaObject<DiaobingCard>();
    addMetaObject<MoshuCard>();
    addMetaObject<RenxingCard>();
    addMetaObject<AnshaCard>();
    addMetaObject<MaixiongCard>();
    addMetaObject<GaizaoCard>();
    addMetaObject<YuandingCard>();
    addMetaObject<JingshenCard>();

    patterns[".At"] = new AttackPattern;

    addMetaObject<CheatCard>();
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "uzumaki", "god", 5, true, true);
    new General(this, "haruno", "god", 5, false, true);
}

ADD_PACKAGE(Test)
