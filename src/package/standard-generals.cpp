#include "general.h"
#include "standard.h"
#include "standard-generals.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"

ZhenxiangCard::ZhenxiangCard(){
    once = true;
}

bool ZhenxiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void ZhenxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    room->setPlayerFlag(effect.to, "dongchaee");
    room->setTag("Dongchaee", effect.to->objectName());
    room->setTag("Dongchaer", effect.from->objectName());

    room->showAllCards(effect.to, effect.from);

    LogMessage log;
    log.type = "$Zhenxiangissingle";
    log.from = effect.from;
    log.to << effect.to;
    room->sendLog(log);
}

class ZhenxiangViewAsSkill: public ZeroCardViewAsSkill{
public:
    ZhenxiangViewAsSkill():ZeroCardViewAsSkill("zhenxiangV"){
    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhenxiangCard");
    }
    virtual const Card *viewAs() const{
        return new ZhenxiangCard;
    }
};

class Zhenxiang: public TriggerSkill{
public:
    Zhenxiang():TriggerSkill("zhenxiang"){
        events << CardEffected << PhaseChange;
        view_as_skill = new ZhenxiangViewAsSkill;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *kudou, QVariant &data) const{
        Room *room = kudou->getRoom();
        if(event == PhaseChange && kudou->getPhase() == Player::Finish){
            QString dongchaee_name = room->getTag("Dongchaee").toString();
            if(!dongchaee_name.isEmpty()){
                ServerPlayer *dongchaee = room->findChild<ServerPlayer *>(dongchaee_name);
                room->setPlayerFlag(dongchaee, "-dongchaee");

                room->setTag("Dongchaee", QVariant());
                room->setTag("Dongchaer", QVariant());
            }
        }
        else if(event == CardEffected && kudou->getPhase() == Player::NotActive){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.from != kudou && effect.card->isNDTrick()
                && !effect.card->inherits("AmazingGrace")
                && room->askForSkillInvoke(kudou, objectName(), data)){
                room->showAllCards(effect.from, effect.to);
            }
        }
        return false;
    }
};

class Wuwei:public MasochismSkill{
public:
    Wuwei():MasochismSkill("wuwei$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("wuwei");
    }

    virtual void onDamaged(ServerPlayer *kudou, const DamageStruct &damage) const{
        Room *room = kudou->getRoom();
        QVariant data = QVariant::fromValue(damage);
        if(room->getCurrent() != kudou && damage.from && room->askForSkillInvoke(kudou, objectName(), data)){
            foreach(ServerPlayer *player, room->getOtherPlayers(kudou)){
                const Card *slash = room->askForCard(player, "slash", "@wuwei-slash", data);
                if(slash){
                    CardUseStruct card_use;
                    card_use.card = slash;
                    card_use.from = kudou;
                    card_use.to << damage.from;
                    room->useCard(card_use);
                }
            }
        }
    }
};

class Rexue:public MasochismSkill{
public:
    Rexue():MasochismSkill("rexue"){
    }

    virtual void onDamaged(ServerPlayer *hatto, const DamageStruct &damage) const{
        Room *room = hatto->getRoom();
        if(hatto->askForSkillInvoke(objectName())){
            int x = damage.damage, i;
            for(i=0; i<x; i++){
                ServerPlayer *target = room->askForPlayerChosen(hatto, room->getOtherPlayers(hatto), "rexue");
                int card_id = room->drawCard();
                target->addToPile("rexue", card_id);
                room->acquireSkill(target, "rexue_effect", false);
            }
        }
    }
};

class RexueEffect:public TriggerSkill{
public:
    RexueEffect():TriggerSkill("rexue_effect"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DyingStruct dying_data = data.value<DyingStruct>();
        if(player == dying_data.who &&
           !dying_data.who->getPile("rexue").isEmpty() &&
           dying_data.who->askForSkillInvoke("rexue_get", data)){
            int peach = player->getPile("rexue").first();
            room->throwCard(peach);
            RecoverStruct recover;
            recover.card = Sanguosha->getCard(peach);
            ServerPlayer *hatto = room->findPlayerBySkillName("rexue");
            if(hatto)
                recover.who = hatto;

            room->recover(player, recover);
            return false;
        }
        return false;
    }
};

JiaojinCard::JiaojinCard(){
    once = true;
    will_throw = false;
}

bool JiaojinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    return true;
}

void JiaojinCard::use(Room *room, ServerPlayer *heiji, const QList<ServerPlayer *> &targets) const{

    ServerPlayer *target = targets.first();
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    room->showCard(heiji, card->getEffectiveId());
    QString suit_str = card->getSuitString();
    QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
    QString prompt = QString("@jiaojinask:%1::%2").arg(heiji->getGeneralName()).arg(suit_str);
    if(!room->askForCard(target, pattern, prompt)){
        if(target->getCardCount(true) <= 3)
            target->throwAllCards();
        else
            room->askForDiscard(target, "jiaojin", 3, false, true);
    }
}

class Jiaojin:public OneCardViewAsSkill{
public:
    Jiaojin():OneCardViewAsSkill("jiaojin"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JiaojinCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JiaojinCard *card = new JiaojinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Chenshui: public PhaseChangeSkill{
public:
    Chenshui():PhaseChangeSkill("chenshui"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Start && room->askForSkillInvoke(mouri, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(mouri, room->getAlivePlayers(), "chenshuiprotect");
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                if (player == target)
                    continue;
                if (player == mouri)
                    continue;
                const Card *slash = room->askForCard(player, "slash", "chenshui-slash:" + mouri->getGeneralName());
                if(!slash){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.to = player;
                    damage.from = mouri;
                    room->damage(damage);
                }
            }
            mouri->skip(Player::Judge);
            mouri->skip(Player::Draw);
            mouri->skip(Player::Play);
            //return true;
        }
        return false;
    }
};

class Jingxing:public MasochismSkill{
public:
    Jingxing():MasochismSkill("jingxing"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *mori, const DamageStruct &damage) const{
        Room *room = mori->getRoom();
        if(!room->askForSkillInvoke(mori, objectName()))
            return;

        for(int i = damage.damage; i>0; i--){
            ServerPlayer *target = room->askForPlayerChosen(mori, room->getAlivePlayers(), objectName());
            if(target == mori)
                target->drawCards(2);
            else{
                if(!target->isNude())
                    room->askForDiscard(target, objectName(), 1, false, true);
            }
        }
    }
};

class Zhinang: public TriggerSkill{
public:
    Zhinang():TriggerSkill("zhinang"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *conan, QVariant &data) const{
        Room *room = conan->getRoom();
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Hand && conan && room->askForSkillInvoke(conan, objectName())){
            //room->playSkillEffect(objectName());
             conan->drawCards(1);
        }
        return false;
    }
};

MazuiCard::MazuiCard(){
    once = true;
}

void MazuiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = true;
    judge.reason = objectName();
    judge.who = source;

    room->judge(judge);
    if(judge.isGood()){
        target->turnOver();
    }
}

class Mazui: public ZeroCardViewAsSkill{
public:
    Mazui():ZeroCardViewAsSkill("mazui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MazuiCard");
    }

    virtual const Card *viewAs() const{
        return new MazuiCard;
    }
};

class Fuyuan: public TriggerSkill{
public:
    Fuyuan():TriggerSkill("fuyuan$"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("fuyuan");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard){
            if(player->getMark("aptx") > 0){
                room->detachSkillFromPlayer(player, "zhenxiang");
                room->detachSkillFromPlayer(player, "wuwei");

                LogMessage log;
                log.type = "$Fuyuanrb";
                log.from = player;
                room->sendLog(log);
                player->setMark("aptx", 0);
            }

            QVariant num = player->getHandcardNum();
            player->tag["FC_S"] = num;
            return false;
        }
        if(player->getPhase() == Player::Finish){
            int num = player->tag.value("FC_S").toInt();
            if(num - player->getHandcardNum() >= 2 && player->askForSkillInvoke(objectName(), data)){
                player->setMark("aptx", 1);
                room->acquireSkill(player, "zhenxiang");
                if(player->isLord())
                    room->acquireSkill(player, "wuwei");
            }
        }
        return false;
    }
};

class Pantao: public TriggerSkill{
public:
    Pantao():TriggerSkill("pantao"){
        events << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *ai = room->findPlayerBySkillName(objectName());

        if(ai && ai->isWounded() && ai->faceUp() && ai->askForSkillInvoke(objectName())){
            ai->turnOver();

            PlayerStar zhenggong = room->getTag("Pantao").value<PlayerStar>();
            if(zhenggong == NULL){
                PlayerStar p = player;
                room->setTag("Pantao", QVariant::fromValue(p));
                player->gainMark("pantao");
            }

            room->setCurrent(ai);
            ai->play();

            return true;

        }else{
            PlayerStar p = room->getTag("Pantao").value<PlayerStar>();
            if(p){
                p->loseMark("pantao");
                room->setCurrent(p);
                room->setTag("Pantao", QVariant());
            }
        }

        return false;
    }
};

class Qianfu: public TriggerSkill{
public:
    Qianfu():TriggerSkill("qianfu"){
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->faceUp();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isRed())
            return false;

        LogMessage log;
        log.type = "#SkillNullify";
        log.from = player;
        log.arg = objectName();
        log.arg2 = effect.slash->objectName();

        player->getRoom()->sendLog(log);

        return true;
    }
};

ShiyanCard::ShiyanCard(){
    once = true;
    target_fixed = true;
    will_throw = false;
}

void ShiyanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const Card *card = Sanguosha->getCard(this->getSubcards().first());

    LogMessage log;
    log.from = source;
    log.card_str = card->getEffectIdString();
    log.type = "$Shiyan";
    room->sendLog(log);

    Card::Suit suit = card->getSuit();
    JudgeStruct judge;
    judge.reason = objectName();
    judge.who = source;

    room->judge(judge);
    source->obtainCard(judge.card);

    if(judge.card->getSuit()==suit){
        log.type = "$Shiyansuc";
        room->sendLog(log);

        room->drawCards(source,2);
    }
    else{
        log.type = "$Shiyanfail";
        room->sendLog(log);
    }
}

class Shiyan:public OneCardViewAsSkill{
public:
    Shiyan():OneCardViewAsSkill("shiyan"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShiyanCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShiyanCard *card = new ShiyanCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

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
                //room->playSkillEffect(objectName());
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
    source->obtainCard(this);
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
            room->broadcastInvoke("animate", "lightbox:$yirong");
            sharon->loseMark("@yaiba");

            QStringList genlist = Sanguosha->getLimitedGeneralNames();
            foreach(ServerPlayer *player, room->getAllPlayers()){
                genlist.removeOne(player->getGeneralName());
            }

            QString general = room->askForGeneral(sharon, genlist);
            room->transfigure(sharon, general, false);
            //room->acquireSkill(sharon, "yirong", false);
            sharon->getRoom()->setPlayerProperty(sharon, "hp", 3);
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
            if(card->getSubcards().isEmpty() || !room->askForSkillInvoke(megure, objectName(), data))
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
            foreach(int card_id, card->getSubcards()){
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
        if(player->getHandcardNum() > player->getHp())
            return true;
        return false;
    }
};

class Shangchi: public PhaseChangeSkill{
public:
    Shangchi():PhaseChangeSkill("shangchi"){
        default_choice = "me";
    }

    virtual bool onPhaseChange(ServerPlayer *matsumoto) const{
        if(matsumoto->getPhase() == Player::Draw && matsumoto->isWounded()){
            Room *room = matsumoto->getRoom();
            if(room->askForChoice(matsumoto, objectName(), "me+him") == "me")
                room->drawCards(matsumoto,matsumoto->getLostHp());
            else {
                ServerPlayer *target = room->askForPlayerChosen(matsumoto,room->getAlivePlayers(),objectName());
                target->drawCards(matsumoto->getLostHp()-1);
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

class Moshu: public TriggerSkill{
public:
    Moshu():TriggerSkill("moshu"){
        events << PhaseChange;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *kaitou = room->findPlayerBySkillName(objectName());
        if(!kaitou)
            return false;
        if(player != kaitou && player->getPhase() == Player::Draw && kaitou->getMark("magic") == 0){
            QString choice = room->askForChoice(kaitou, objectName(), "zero+one+two");
            const Card *card;

            LogMessage log;
            log.type = "#Moshu";
            log.from = kaitou;
            log.arg2 = objectName();

            if(choice=="zero")
                return false;
            else if(choice=="one"){
                kaitou->drawCards(1);
                card = room->askForCard(kaitou, ".", "moshu-only");
                if(!card)
                    card = kaitou->getHandcards().first();
                room->moveCardTo(card, NULL, Player::DrawPile, true);

                log.arg = QString::number(1);
            }
            else{
                kaitou->drawCards(2);
                card = room->askForCard(kaitou, ".", "moshu-first");
                if(!card)
                    card = kaitou->getHandcards().last();
                room->moveCardTo(card, NULL, Player::DrawPile, true);
                card = room->askForCard(kaitou, ".", "moshu-second");
                if(!card)
                    card = kaitou->getHandcards().first();
                room->moveCardTo(card, NULL, Player::DrawPile, true);

                log.arg = QString::number(2);
            }
            room->sendLog(log);
            kaitou->setMark("magic", 1);
         }
        else if(player==kaitou && player->getPhase() == Player::Start){
            kaitou->setMark("magic", 0);
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

    virtual bool trigger(TriggerEvent , ServerPlayer *gin, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()){
            Room *room = gin->getRoom();
            if(room->askForSkillInvoke(gin, objectName(), data)){
                const Card *card = room->askForCard(gin, "slash", "juelu-slash");
                if(card){
                    // if player is drank, unset his flag
                    if(gin->hasFlag("drank"))
                        room->setPlayerFlag(gin, "-drank");
                    room->cardEffect(card, gin, damage.to);
                }
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
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *shuichi, QVariant &data) const{
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

class Gaizao:public TriggerSkill{
public:
    Gaizao():TriggerSkill("gaizao"){
        events << PhaseChange;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Draw)
            return false;

        Room *room = player->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->hasEquip()){
                targets << tmp;
            }
        }
        if(targets.length()!=0 && room->askForSkillInvoke(player,objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player,targets,objectName());
            int card_id = room->askForCardChosen(player, target, "e", objectName());
            room->throwCard(card_id);
            player->drawCards(1);
            target->drawCards(1);
        }
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
            log.arg = objectName();
            room->sendLog(log);
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
            const Card *recovcd = room->askForCard(cancer, ".S", objectName());
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
    return targets.isEmpty() && !to_select->isKongcheng();
}

void YuandingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->showCard(effect.from, effect.card->getId(), effect.to);
    const Card *cart = room->askForCard(effect.to, ".", "@yuandingask:" + effect.from->objectName(), false);
    if(cart){
        effect.from->obtainCard(cart);
        effect.to->obtainCard(effect.card);
    }
}

class Yuanding:public OneCardViewAsSkill{
public:
    Yuanding():OneCardViewAsSkill("yuanding"){

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

class Qiniao: public TriggerSkill{
public:
    Qiniao():TriggerSkill("qiniao"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *sumiko, QVariant &data) const{
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
    General *kudoushinichi, *hattoriheiji, *mourikogorou;

    kudoushinichi = new General(this, "kudoushinichi$", "zhen");
    kudoushinichi->addSkill(new Zhenxiang);
    kudoushinichi->addSkill(new Wuwei);

    hattoriheiji = new General(this, "hattoriheiji", "zhen", 3);
    hattoriheiji->addSkill(new Rexue);
    hattoriheiji->addSkill(new Jiaojin);

    mourikogorou = new General(this, "mourikogorou", "zhen", 3);
    mourikogorou->addSkill(new Chenshui);
    mourikogorou->addSkill(new Jingxing);

    General *edogawaconan, *haibaraai, *yoshidaayumi;
    edogawaconan = new General(this, "edogawaconan$", "shao", 3);
    edogawaconan->addSkill(new Zhinang);
    edogawaconan->addSkill(new Mazui);
    edogawaconan->addSkill(new Fuyuan);

    haibaraai = new General(this, "haibaraai", "shao", 3, false);
    haibaraai->addSkill(new Pantao);
    haibaraai->addSkill(new Shiyan);
    haibaraai->addSkill(new Qianfu);

    yoshidaayumi = new General(this, "yoshidaayumi", "shao", 2, false);
    yoshidaayumi->addSkill(new Tianzhen);
    yoshidaayumi->addSkill(new Lanman);
    yoshidaayumi->addSkill(new Dontcry);

    General *mouriran, *touyamakazuha, *kyougokumakoto;
    mouriran = new General(this, "mouriran", "woo", 3, false);
    mouriran->addSkill(new Duoren);
    mouriran->addSkill(new Shouhou);

    touyamakazuha = new General(this, "touyamakazuha", "woo", 3, false);
    touyamakazuha->addSkill(new Heqi);
    touyamakazuha->addSkill(new Shouqiu);

    kyougokumakoto = new General(this, "kyougokumakoto", "woo");
    kyougokumakoto->addSkill(new Shenyong);

    General *kaitoukid, *sharon;
    kaitoukid = new General(this, "kaitoukid", "yi", 3);
    kaitoukid->addSkill(new Shentou);
    kaitoukid->addSkill(new Baiyi);
    kaitoukid->addSkill(new Feixing);

    sharon = new General(this, "sharon", "yi", 3, false);
    sharon->addSkill(new Yirong);
    sharon->addSkill(new MarkAssignSkill("@yaiba", 1));
    related_skills.insertMulti("yirong", "#@yaiba");
    sharon->addSkill(new Wuyu);

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
    otagiritoshirou->addSkill(new Qinjian);

    General *kurobakaitou, *nakamoriaoko;
    kurobakaitou = new General(this, "kurobakaitou", "guai");
    kurobakaitou->addSkill(new Tishen);
    kurobakaitou->addSkill(new MarkAssignSkill("@fake", 1));
    related_skills.insertMulti("tishen", "#@fake");
    //kurobakaitou->addSkill(new MarkAssignSkill("magic", 1));
    kurobakaitou->addSkill(new Moshu);

    nakamoriaoko = new General(this, "nakamoriaoko", "guai", 4, false);
    nakamoriaoko->addSkill(new Renxing);
    nakamoriaoko->addSkill(new Qingmei);

    General *gin, *vodka, *akaishuichi;
    gin = new General(this, "gin$", "hei");
    gin->addSkill(new Ansha);
    gin->addSkill(new MarkAssignSkill("@ansha", 1));
    related_skills.insertMulti("ansha", "#@ansha");
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
    addMetaObject<ZhenxiangCard>();
    addMetaObject<JiaojinCard>();
    addMetaObject<MazuiCard>();
    addMetaObject<ShiyanCard>();
    addMetaObject<ShouqiuCard>();
    addMetaObject<BaiyiCard>();
    addMetaObject<DiaobingCard>();
    addMetaObject<RenxingCard>();
    addMetaObject<AnshaCard>();
    addMetaObject<MaixiongCard>();
    addMetaObject<YuandingCard>();
    addMetaObject<JingshenCard>();

    skills << new RexueEffect;
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
