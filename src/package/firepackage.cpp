#include "firepackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"

IentouCard::IentouCard(){
    once = true;
}

bool IentouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->isLord())
        return false;
    if(!targets.isEmpty()){
        if(qAbs(to_select->getHp() - targets.first()->getHp()) > getSubcards().length())
            return false;
    }
    return true;
}

bool IentouCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() != 2)
        return false;
    if(targets.first()->getHp() == targets.last()->getHp())
        return false;
    return qAbs(targets.first()->getHp() - targets.last()->getHp()) <= getSubcards().length();
}

void IentouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    int x = targets.first()->getHp();
    room->setPlayerProperty(targets.first(), "hp", qMin(targets.first()->getMaxHP(), targets.last()->getHp()));
    room->setPlayerProperty(targets.last(), "hp", qMin(targets.last()->getMaxHP(), x));
}

class Ientou: public ViewAsSkill{
public:
    Ientou():ViewAsSkill("ientou"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("IentouCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        IentouCard *card = new IentouCard;
        card->addSubcards(cards);
        return card;
    }
};

class Biguan:public DrawCardsSkill{
public:
    Biguan():DrawCardsSkill("biguan"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *matru, int n) const{
        Room *room = matru->getRoom();
        int todraw = qMin(matru->getHandcardNum(), 4);
        if(todraw > 2)
            room->playSkillEffect(objectName(), qrand() % 2 + 1);
        else
            room->playSkillEffect(objectName(), qrand() % 2 + 3);

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = matru;
        log.arg = objectName();
        room->sendLog(log);

        return todraw;
    }
};

class Fengsheng: public TriggerSkill{
public:
    Fengsheng():TriggerSkill("fengsheng"){
        events << AskForPeachesDone;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *yuriko, QVariant &data) const{
        if(yuriko->getHp() > 0){
            yuriko->playSkillEffect(objectName());
            yuriko->gainMark("@door");
        }
        return false;
    }
};

class Yiben: public TriggerSkill{
public:
    Yiben():TriggerSkill("yiben"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *mori, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place != Player::Hand)
            return false;

        Room *room = mori->getRoom();
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *player, room->getOtherPlayers(mori)){
            if(mori->canSlash(player, false))
                players << player;
        }

        ServerPlayer *target = mori;
        if(!players.isEmpty() && mori->askForSkillInvoke(objectName()))
            target = room->askForPlayerChosen(mori, players, objectName());

        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct use;
        use.card = slash;
        use.from = mori;
        use.to << target;

        room->useCard(use);
        return false;
    }
};

class Xiebi: public TriggerSkill{
public:
    Xiebi():TriggerSkill("xiebi"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *rou, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to && !damage.to->faceUp() && rou->askForSkillInvoke(objectName(), data)){
            rou->playSkillEffect(objectName(), 1);
            damage.to->turnOver();
        }
        return false;
    }
};

/*
class Mengjin: public TriggerSkill{
public:
    Mengjin():TriggerSkill("mengjin"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *pangde, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude()){
            Room *room = pangde->getRoom();
            if(pangde->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                room->throwCard(to_throw);
            }
        }

        return false;
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
        chain->addSubcard(card);
        chain->setSkillName(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill{
public:
    Niepan():TriggerSkill("niepan"){
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pangtong, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != pangtong)
            return false;

        Room *room = pangtong->getRoom();
        if(pangtong->askForSkillInvoke(objectName(), data)){
            room->broadcastInvoke("animate", "lightbox:$niepan");
            room->playSkillEffect(objectName());

            pangtong->loseMark("@nirvana");

            room->setPlayerProperty(pangtong, "hp", qMin(3, pangtong->getMaxHP()));
            pangtong->throwAllCards();
            pangtong->drawCards(3);

            if(pangtong->isChained()){
                    room->setPlayerProperty(pangtong, "chained", false);
            }
            if(!pangtong->faceUp())
                pangtong->turnOver();
        }

        return false;
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
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Bazhen: public TriggerSkill{
public:
    Bazhen():TriggerSkill("bazhen"){
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) con->getMark("qinggang") == 0 && target->getMark("wuqian") == 0;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *wolong, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        Room *room = wolong->getRoom();
        if(wolong->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wolong;

            room->judge(judge);

            if(judge.isGood()){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(wolong, "good");
                return true;
            }else
                room->setEmotion(wolong, "bad");
        }

        return false;
    }
};
*/
class Jiaoxie: public TriggerSkill{
public:
    Jiaoxie():TriggerSkill("jiaoxie"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jiuwenlong = room->findPlayerBySkillName(objectName());
        if(!jiuwenlong || player == jiuwenlong)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::DiscardedPile){
            const Card *weapon = Sanguosha->getCard(move->card_id);
            if(weapon->inherits("Weapon") &&
               jiuwenlong->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                jiuwenlong->obtainCard(weapon);
            }
        }
        return false;
    }
};

class Xianv: public TriggerSkill{
public:
    Xianv():TriggerSkill("xianv"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jiuwenlong = room->findPlayerBySkillName(objectName());
        if(!jiuwenlong || jiuwenlong->isNude())
            return false;
        DamageStruct damage = data.value<DamageStruct>();

        bool caninvoke = jiuwenlong->hasSkill("mazdaRX7") ? true : damage.nature == DamageStruct::Normal;
        if(caninvoke && damage.to->isAlive() && damage.damage > 0){
            caninvoke = false;
            foreach(const Card *cd, jiuwenlong->getCards("he")){
                if(cd->getTypeId() == Card::Equip){
                    caninvoke = true;
                    break;
                }
            }
            if(caninvoke){
                const Card *card = room->askForCard(jiuwenlong, "EquipCard", "@xianv:" + damage.to->objectName(), data);
                if(card){
                    LogMessage log;
                    log.type = "$Xianv";
                    log.from = jiuwenlong;
                    log.to << damage.to;
                    log.card_str = card->getEffectIdString();
                    room->sendLog(log);
                    room->playSkillEffect(objectName());

                    damage.damage --;
                }
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Mune: public TriggerSkill{
public:
    Mune():TriggerSkill("mune"){
        frequency = Frequent;
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *wataru, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        if(wataru->askForSkillInvoke(objectName(), data_card)){
            wataru->obtainCard(judge->card);
            wataru->playSkillEffect(objectName());
            //if(judge->delayedtrick)
            //    wataru->obtainCard(judge->delayedtrick);
            return true;
        }
        return false;
    }
};

class Gengzhi: public MasochismSkill{
public:
    Gengzhi():MasochismSkill("gengzhi"){
    }

    virtual void onDamaged(ServerPlayer *takagi, const DamageStruct &) const{
        Room *room = takagi->getRoom();
        const Card *first = room->peek();
        room->moveCardTo(first, takagi, Player::Special);
        const Card *second = room->peek();
        room->moveCardTo(second, takagi, Player::Special);

        const Card *heart = NULL;
        if(first->getSuit() == Card::Heart && second->getSuit() != Card::Heart)
            heart = first;
        else if(first->getSuit() != Card::Heart && second->getSuit() == Card::Heart)
            heart = second;

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = takagi;
        log.arg = objectName();
        room->sendLog(log);
        room->playSkillEffect(objectName());
        if(heart){
            RecoverStruct rec;
            rec.card = heart;
            room->recover(takagi, rec, true);
            room->moveCardTo(heart, takagi, Player::Hand);
            //takagi->obtainCard(heart);
        }
        else{
            if(qrand() % 2 == 0){
                heart = first;
                room->moveCardTo(heart, takagi, Player::Hand);
                //takagi->obtainCard(heart);
            }
        }
        if(heart == first)
            room->moveCardTo(second, NULL, Player::DrawPile);
        else
            room->moveCardTo(first, NULL, Player::DrawPile);
    }
};

FangxinCard::FangxinCard(){
    mute = true;
}

bool FangxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void FangxinCard::use(Room *room, ServerPlayer *gaolian, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.first();

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = false;
    judge.reason = objectName();
    judge.who = gaolian;

    room->judge(judge);

    if(judge.isGood()){
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("fangxin");

        CardUseStruct use;
        use.from = gaolian;
        use.to << to;
        use.card = slash;
        room->useCard(use);
    }else
        room->setPlayerFlag(gaolian, "Fangxin");
}

class FangxinViewAsSkill:public ZeroCardViewAsSkill{
public:
    FangxinViewAsSkill():ZeroCardViewAsSkill("fangxin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new FangxinCard;
    }
};

class Fangxin: public TriggerSkill{
public:
    Fangxin():TriggerSkill("fangxin"){
        events << CardAsked;
        view_as_skill = new FangxinViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *gaolian, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        if(gaolian->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = gaolian;

            Room *room = gaolian->getRoom();
            room->playSkillEffect(objectName());
            room->judge(judge);

            if(judge.isGood()){
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                room->provide(slash);
                return true;
            }
            else
                room->setPlayerFlag(gaolian, "Fangxin");
        }
        return false;
    }
};

MoguaCard::MoguaCard(){
    target_fixed = true;
}

void MoguaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    int num = getSubcards().length();
    room->moveCardTo(this, NULL, Player::DrawPile);
    QList<int> fog = room->getNCards(num, false);
    room->doGuanxing(source, fog, true);
}

class Mogua:public ViewAsSkill{
public:
    Mogua():ViewAsSkill("mogua"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        MoguaCard *mogua_card = new MoguaCard;
        mogua_card->addSubcards(cards);
        return mogua_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }
};

class Shanliang: public TriggerSkill{
public:
    Shanliang():TriggerSkill("shanliang"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        QList<ServerPlayer *> ducks = room->findPlayersBySkillName(objectName());
        if(ducks.isEmpty())
            return false;
        foreach(ServerPlayer *duck, ducks){
            if(duck != player && damage.damage > 0 && duck->distanceTo(player) <= 2 &&
               duck->askForSkillInvoke(objectName())){
                room->loseHp(duck);
                if(duck->isAlive())
                    duck->drawCards(player->getHp());
                return true;
            }
        }
        return false;
    }

    virtual int getPriority() const{
        return 2;
    }
};

class Qingshang: public TriggerSkill{
public:
    Qingshang():TriggerSkill("qingshang"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Normal && player->getHp() == 1){
            Room *room = player->getRoom();

            LogMessage log;
            log.type = "#QSProtect";
            log.from = damage.from;
            log.to << player;
            log.arg = objectName();
            room->sendLog(log);

            return true;
        }else
            return false;
    }
};

FirePackage::FirePackage()
    :Package("fire")
{
    General *amurotooru = new General(this, "amurotooru", "zhen");
    amurotooru->addSkill(new Ientou);

    General *matsunakayuriko = new General(this, "matsunakayuriko", "shao", 2, false);
    matsunakayuriko->addSkill(new Biguan);
    matsunakayuriko->addSkill(new Fengsheng);

    General *kugorou = new General(this, "kugorou", "woo");
    kugorou->addSkill(new Yiben);
    kugorou->addSkill(new Xiebi);

    General *satomiwako = new General(this, "satomiwako", "jing", 3, false);
    satomiwako->addSkill(new Jiaoxie);
    satomiwako->addSkill(new Xianv);

    General *takagiwataru = new General(this, "takagiwataru", "jing", 3);
    takagiwataru->addSkill(new Mune);
    takagiwataru->addSkill(new Gengzhi);

    General *koizumiakako = new General(this, "koizumiakako", "guai", 3, false);
    koizumiakako->addSkill(new Fangxin);
    koizumiakako->addSkill(new Mogua);

    General *miyanoagemi = new General(this, "miyanoagemi", "te", 3, false, true);
    miyanoagemi->addSkill(new Shanliang);
    miyanoagemi->addSkill(new Qingshang);

    addMetaObject<IentouCard>();
    addMetaObject<FangxinCard>();
    addMetaObject<MoguaCard>();
}

ADD_PACKAGE(Fire);
