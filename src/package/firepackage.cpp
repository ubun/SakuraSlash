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

/*
QiangxiCard::QiangxiCard(){
    once = true;
}

bool QiangxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(!subcards.isEmpty() && Self->getWeapon() == Sanguosha->getCard(subcards.first())
        && !Self->hasFlag("tianyi_success"))
        return Self->distanceTo(to_select) <= 1;

    return Self->inMyAttackRange(to_select);
}

void QiangxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(subcards.isEmpty())
        room->loseHp(effect.from);

    DamageStruct damage;
    damage.card = NULL;
    damage.from = effect.from;
    damage.to = effect.to;

    room->damage(damage);
}

class Qiangxi: public ViewAsSkill{
public:
    Qiangxi():ViewAsSkill("qiangxi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QiangxiCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return new QiangxiCard;
        else if(cards.length() == 1){
            QiangxiCard *card = new QiangxiCard;
            card->addSubcards(cards);

            return card;
        }else
            return NULL;
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

class ShuangxiongViewAsSkill: public OneCardViewAsSkill{
public:
    ShuangxiongViewAsSkill():OneCardViewAsSkill("shuangxiong"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("shuangxiong") != 0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        int value = Self->getMark("shuangxiong");
        if(value == 1)
            return to_select->getFilteredCard()->isBlack();
        else if(value == 2)
            return to_select->getFilteredCard()->isRed();

        return false;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Duel *duel = new Duel(card->getSuit(), card->getNumber());
        duel->addSubcard(card);
        duel->setSkillName(objectName());
        return duel;
    }
};

class Shuangxiong: public TriggerSkill{
public:
    Shuangxiong():TriggerSkill("shuangxiong"){
        view_as_skill = new ShuangxiongViewAsSkill;

        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shuangxiong, QVariant &data) const{
        Room *room = shuangxiong->getRoom();

        if(event == PhaseChange){
            if(shuangxiong->getPhase() == Player::Start){
                room->setPlayerMark(shuangxiong, "shuangxiong", 0);
            }else if(shuangxiong->getPhase() == Player::Draw){
                if(shuangxiong->askForSkillInvoke(objectName())){
                    shuangxiong->setFlags("shuangxiong");

                    JudgeStruct judge;
                    judge.pattern = QRegExp("(.*)");
                    judge.good = true;
                    judge.reason = objectName();
                    judge.who = shuangxiong;

                    room->judge(judge);

                    room->setPlayerMark(shuangxiong, "shuangxiong", judge.card->isRed() ? 1 : 2);
                    shuangxiong->setFlags("-shuangxiong");

                    return true;
                }
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "shuangxiong"){
                shuangxiong->obtainCard(judge->card);
                return true;
            }
        }

        return false;
    }
};

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
                int to_throw = room->askForCardChosen(pangde, effect.to, "he", objectName());
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
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
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
                if(dying_data.damage == NULL || dying_data.damage->nature == DamageStruct::Normal)
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
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
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

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getArmor() && target->getMark("qinggang") == 0 && target->getMark("wuqian") == 0;
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
                const Card *card = room->askForCard(jiuwenlong, "EquipCard", "@xianv", data);
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
};

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
        if(damage.from && player->getHp() == 1){
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
