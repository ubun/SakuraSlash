#include "fire.h"
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
        int todraw = qMin(matru->getHandcardNum() + matru->getHp(), 4);
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
            yuriko->drawCards(3);

            LogMessage log;
            log.type = "#Fengsheng";
            log.from = yuriko;
            log.arg = objectName();
            yuriko->getRoom()->sendLog(log);

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
        if(mori->getPhase() != Player::NotActive || move->from_place != Player::Hand)
            return false;
        if(!Sanguosha->getCard(move->card_id)->isBlack())
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
        if(damage.to->isAlive() && !damage.to->faceUp() && rou->askForSkillInvoke(objectName(), data)){
            rou->playSkillEffect(objectName(), 1);
            damage.to->turnOver();
        }
        return false;
    }
};

class Yinxing: public TriggerSkill{
public:
    Yinxing():TriggerSkill("yinxing"){
        events << CardLost << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *s, QVariant &data) const{
        Room *room = s->getRoom();
        if(event == CardLost){
            if(!s->hasSkill(objectName()) || s->getPhase() != Player::NotActive)
                return false;
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Equip){
                LogMessage log;
                log.type = "#Yinxing";
                log.from = s;
                log.to << room->getCurrent();
                log.arg = objectName();
                room->sendLog(log);

                s->setFlags("Yinxing");
            }
        }
        else{
            if(s->getPhase() == Player::NotActive){
                foreach(ServerPlayer *fusae, room->getOtherPlayers(s)){
                    if(!fusae->hasFlag("Yinxing"))
                        continue;
                    fusae->setFlags("-Yinxing");
                    if(fusae->askForSkillInvoke(objectName()))
                        fusae->gainAnExtraTurn(s);
                }
            }
        }
        return false;
    }
};

class Chuyin: public MasochismSkill{
public:
    Chuyin():MasochismSkill("chuyin"){
    }

    virtual void onDamaged(ServerPlayer *miku, const DamageStruct &) const{
        Room *room = miku->getRoom();
        if(miku->isAlive() && miku->askForSkillInvoke(objectName())){
            room->setPlayerProperty(miku, "maxhp", miku->getMaxHp() + 1);
            bool flag = true;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(miku)){
                if(tmp->getHp() < miku->getHp()){
                    flag = false;
                    break;
                }
            }
            if(flag){
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miku)){
                    if(!room->askForDiscard(tmp, objectName(), 2, true, true))
                        room->loseHp(tmp);
                }
            }
        }
    }
};

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

YingyanCard::YingyanCard(){
    once = true;
}

bool YingyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->inMyAttackRange(to_select) && !to_select->isKongcheng();
}

void YingyanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if(effect.to->isKongcheng())
        return;
    room->setTag("Yingyan", QVariant::fromValue(effect));
    QList<int> all = effect.to->handCards();
    room->fillAG(all, effect.from);
    int mitan = room->askForAG(effect.from, all, effect.from->isNude(), "yingyan");
    if(mitan < 0)
        mitan = all.first();
    effect.from->invoke("clearAG");
    room->removeTag("Yingyan");
    if(!effect.from->isNude()){
        int c = room->askForCardChosen(effect.from, effect.from, "he", "yingyan");
        room->obtainCard(effect.from, mitan, false);
        room->obtainCard(effect.to, c, room->getCardPlace(c) != Player::Hand);
    }

    //effect.from->getRoom()->doGongxin(effect.from, effect.to);
}

class Yingyan: public ZeroCardViewAsSkill{
public:
    Yingyan():ZeroCardViewAsSkill("yingyan"){
    }

    virtual const Card *viewAs() const{
        return new YingyanCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YingyanCard");
    }
};

class EquiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return player->hasEquip(card);
    }
    virtual bool willThrow() const{
        return false;
    }
};

class Shoushi: public TriggerSkill{
public:
    Shoushi():TriggerSkill("shoushi"){
        events << CardLost << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange){
            if(!player->hasSkill(objectName()))
                return false;
            if(player->getPhase() == Player::Finish && player->askForSkillInvoke(objectName())){
                int final = 5;
                QStringList choices;
                choices << "5" << "back" << "next";
                QString sina = QString();
                while(final == final){
                    sina = room->askForChoice(player, objectName(), choices.join("+"));
                    if(sina != "next" && sina != "back")
                        break;
                    if(sina == "back")
                        final --;
                    else if(sina == "next")
                        final ++;
                    choices.replace(0, QString::number(final));
                }
                final = sina.toInt();
                LogMessage log;
                log.from = player;
                log.type = "#Shoushi";
                log.arg = QString::number(final);
                room->sendLog(log);
                room->setPlayerMark(player, "Shoushi", final);
            }
            else if(player->getPhase() == Player::RoundStart)
                room->setPlayerMark(player, "Shoushi", 0);
            return false;
        }
        if(player->getPhase() == Player::NotActive)
            return false;
        ServerPlayer *horse = room->findPlayerBySkillName(objectName());
        if(!horse || horse->getMark("Shoushi") < 1)
            return false;
        int num = horse->getMark("Shoushi");
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::DiscardedPile){
            const Card *card = Sanguosha->getCard(move->card_id);
            if(card->getNumber() == num){
                LogMessage log;
                log.from = horse;
                log.to << player;
                log.type = "#ShoushiGet";
                log.arg = objectName();
                log.arg2 = QString::number(num);
                room->sendLog(log);
                const Card *card = !player->hasEquip() ? NULL :
                                   room->askForCard(player, ".Equi", "@shoushi:" + horse->objectName(), data);
                if(card)
                    horse->obtainCard(card);
                else
                    room->loseHp(player);
            }
        }
        return false;
    }
};

ChunbaiCard::ChunbaiCard(){
    target_fixed = true;
    once = true;
}

void ChunbaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(5), left;
    left = cards;

    room->fillAG(cards, source);

    while(left.length() > 3){
        int card_id = room->askForAG(source, cards, false, "chunbai");
        if(card_id < 0)
            card_id = cards.first();
        left.removeOne(card_id);
        source->obtainCard(Sanguosha->getCard(card_id));
        room->fillAG(cards, source);
    }

    source->invoke("clearAG");
    room->doGuanxing(source, left, true);
}

class ChunbaiViewAsSkill: public ZeroCardViewAsSkill{
public:
    ChunbaiViewAsSkill():ZeroCardViewAsSkill("chunbai"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@chunbai";
    }

    virtual const Card *viewAs() const{
        return new ChunbaiCard;
    }
};

class Chunbai: public PhaseChangeSkill{
public:
    Chunbai():PhaseChangeSkill("chunbai"){
        view_as_skill = new ChunbaiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *sherry) const{
        if(sherry->getPhase() != Player::Draw)
            return false;
        Room *room = sherry->getRoom();
        return room->askForUseCard(sherry, "@@chunbai", "@chunbai");
    }
};

class Suoxiao: public PhaseChangeSkill{
public:
    Suoxiao():PhaseChangeSkill("suoxiao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start
                && PhaseChangeSkill::triggerable(target)
                && target->isKongcheng()
                && !target->hasMark("suoxiao");
    }

    virtual bool onPhaseChange(ServerPlayer *ai) const{
        Room *room = ai->getRoom();
        if(room->findPlayer("haibaraai"))
            return false;

        LogMessage log;
        log.type = "#WakeUp";
        log.from = ai;
        log.arg = objectName();
        room->sendLog(log);
        room->loseMaxHp(ai);
        room->acquireSkill(ai, "pantao");
        room->acquireSkill(ai, "shiyan");
        room->setPlayerMark(ai, "suoxiao", 1);
        return false;
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
               duck->askForSkillInvoke(objectName(), data)){
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

ShendieCard::ShendieCard(){
    once = true;
    target_fixed = true;
}
/*
bool ShendieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return !to_select->isLord() && to_select != Self;
}
*/
void ShendieCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> tars;
    foreach(ServerPlayer *tmp, room->getOtherPlayers(source)){
        if(!tmp->isLord())
            tars << tmp;
    }
    if(!tars.isEmpty()){
        ServerPlayer *target = room->askForPlayerChosen(source, tars, skill_name);
        QString myrole = source->getRole();
        source->setRole(target->getRole());
        target->setRole(myrole);
    }
}

class ShendieViewAsSkill: public ZeroCardViewAsSkill{
public:
    ShendieViewAsSkill():ZeroCardViewAsSkill("shendie"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShendieCard") && player->getMark("die") < 1;
    }

    virtual const Card *viewAs() const{
        return new ShendieCard;
    }
};

class Shendie:public TriggerSkill{
public:
    Shendie():TriggerSkill("shendie"){
        events << PhaseChange;
        view_as_skill = new ShendieViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *hidemi, QVariant &data) const{
        if(hidemi->getPhase() == Player::Play && hidemi->getMark("die") < 1){
            Room *room = hidemi->getRoom();
            if(room->getAlivePlayers().count() != room->getPlayers().count()){
                room->addHpSlot(hidemi);
                room->setPlayerMark(hidemi, "die", 1);
                room->detachSkillFromPlayer(hidemi, "shendie");
            }
        }
        return false;
    }
};

class Gangxie: public TriggerSkill{
public:
    Gangxie():TriggerSkill("gangxie"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Gangxie";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            return !room->askForCard(effect.from, "slash", "@gangxie:" + effect.to->objectName(), data);
        }
        return false;
    }
};

ManmiCard::ManmiCard(){
    will_throw = false;
    target_fixed = true;
    mute = true;
}

void ManmiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect(skill_name, qrand() % 2 + 1);
    foreach(int table, getSubcards())
        source->addToPile("comic", table);
}

class ManmiViewAsSkill: public ViewAsSkill{
public:
    ManmiViewAsSkill():ViewAsSkill("manmi"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        ManmiCard *card = new ManmiCard;
        card->addSubcards(cards);
        return card;
    }
};

class Manmi: public DrawCardsSkill{
public:
    Manmi():DrawCardsSkill("manmi"){
        view_as_skill = new ManmiViewAsSkill;
    }

    virtual int getDrawNum(ServerPlayer *issen, int n) const{
        Room *room = issen->getRoom();
        if(!issen->getPile("comic").isEmpty() && room->askForSkillInvoke(issen, objectName())){
            QList<int> cards = issen->getPile("comic"), left;
            left = cards;

            room->fillAG(cards, issen);

            while(!left.isEmpty()){
                int card_id = room->askForAG(issen, cards, true, "manmi");
                if(card_id < 0)
                    break;
                left.removeOne(card_id);
                room->throwCard(card_id);
                issen->invoke("clearAG");
                room->fillAG(left, issen);
            }

            issen->invoke("clearAG");

            int x = cards.length() - left.length();
            if(x > 0){
                room->playSkillEffect(objectName(), qrand() % 2 + 3);
                return n + x;
            }
        }
        return n;
    }
};

class Teshe: public TriggerSkill{
public:
    Teshe():TriggerSkill("teshe"){
        events << HpLost << MaxHpLost;
    }

    virtual bool triggerable(const ServerPlayer *player) const{;
        return !player->hasSkill("teshe");
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *chiba = room->findPlayerBySkillName(objectName());
        int n = data.toInt();
        if(chiba && !player->isNude() && n > 0 && chiba->askForSkillInvoke(objectName())){
            for(; n > 0; n--){
                int card_id = room->askForCardChosen(chiba, player, "he", objectName());
                room->obtainCard(chiba, card_id, room->getCardPlace(card_id) != Player::Hand);
                if(player->isNude())
                    break;
            }
        }
        return false;
    }
};

class Chufa: public TriggerSkill{
public:
    Chufa():TriggerSkill("chufa"){
        events << DrawNCards;
    }

    virtual bool triggerable(const ServerPlayer *player) const{;
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *naeko = room->findPlayerBySkillName(objectName());
        if(!naeko || player == naeko)
            return false;
        bool invoke = false;
        foreach(const Card *card, naeko->getCards("he")){
            if(card->getSuit() == Card::Heart){
                invoke = true;
                break;
            }
        }
        if(!invoke)
            return false;
        QString prompt = QString("@chufa:%1::%2").arg(player->objectName()).arg(data.toString());
        return data.toInt() > 0 && room->askForCard(naeko, "..H", prompt, QVariant::fromValue((PlayerStar) player));
    }
};

MijiCard::MijiCard(){
}

bool MijiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return !to_select->isKongcheng() && to_select->getGenderString() == "male";
}

void MijiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    Card::Suit suit = room->askForSuit(effect.from, skill_name);
    const Card *card = effect.to->getRandomHandCard();
    room->obtainCard(effect.from, card);
    if(card->getSuit() == suit)
        room->loseHp(effect.to);
}

class MijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    MijiViewAsSkill():ZeroCardViewAsSkill("miji"){
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@miji";
    }

    virtual const Card *viewAs() const{
        return new MijiCard;
    }
};

class Miji: public PhaseChangeSkill{
public:
    Miji():PhaseChangeSkill("miji"){
        view_as_skill = new MijiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *milk) const{
        if(milk->getPhase() != Player::Finish)
            return false;
        Room *room = milk->getRoom();
        room->askForUseCard(milk, "@@miji", "@miji");
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

    General *kinoshitafusae = new General(this, "kinoshitafusae", "yi", 4, false);
    kinoshitafusae->addSkill(new Yinxing);

    General *akibareiko = new General(this, "akibareiko", "yi", 3, false);
    akibareiko->addSkill(new Chuyin);

    General *satomiwako = new General(this, "satomiwako", "jing", 3, false);
    satomiwako->addSkill(new Jiaoxie);
    satomiwako->addSkill(new Xianv);

    General *takagiwataru = new General(this, "takagiwataru", "jing", 3);
    takagiwataru->addSkill(new Mune);
    takagiwataru->addSkill(new Gengzhi);

    General *koizumiakako = new General(this, "koizumiakako", "guai", 3, false);
    koizumiakako->addSkill(new Fangxin);
    koizumiakako->addSkill(new Mogua);

    General *hakubasaguru = new General(this, "hakubasaguru", "guai", 3);
    hakubasaguru->addSkill(new Yingyan);
    hakubasaguru->addSkill(new Shoushi);
    patterns[".Equi"] = new EquiPattern;

    General *sherry = new General(this, "sherry", "hei", 4, false);
    sherry->addSkill(new Chunbai);
    sherry->addSkill(new Suoxiao);
    sherry->addSkill("#losthp");

    General *miyanoagemi = new General(this, "miyanoagemi", "te", 3, false);
    miyanoagemi->addSkill(new Shanliang);
    miyanoagemi->addSkill(new Qingshang);

    General *hondouhidemi = new General(this, "hondouhidemi", "te", 3, false);
    hondouhidemi->addSkill(new Shendie);
    hondouhidemi->addSkill(new Gangxie);

    General *chibaisshin = new General(this, "chibaisshin", "za", 3);
    chibaisshin->addSkill(new Manmi);
    chibaisshin->addSkill(new Teshe);

    General *miikenaeko = new General(this, "miikenaeko", "za", 3, false);
    miikenaeko->addSkill(new Chufa);
    miikenaeko->addSkill(new Miji);

    addMetaObject<IentouCard>();
    addMetaObject<FangxinCard>();
    addMetaObject<MoguaCard>();
    addMetaObject<ShendieCard>();
    addMetaObject<YingyanCard>();
    addMetaObject<ChunbaiCard>();
    addMetaObject<ManmiCard>();
    addMetaObject<MijiCard>();
}

ADD_PACKAGE(Fire);
