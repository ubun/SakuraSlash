#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "maneuvering.h"

class Bianhu: public TriggerSkill{
public:
    Bianhu():TriggerSkill("bianhu"){
        events << CardUsed;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        Room *room = player->getRoom();
        ServerPlayer *eri = room->findPlayerBySkillName(objectName());
        if(!eri)
            return false;
        if(use.card->inherits("TrickCard") && !use.card->inherits("Nullification")){
            QString suit_str = use.card->getSuitString();
            QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
            QString prompt = QString("@bianhu:%1::%2").arg(use.from->getGeneralName()).arg(suit_str);
            if(eri->askForSkillInvoke(objectName(), data) && room->askForCard(eri, pattern, prompt)){
                ServerPlayer *target = room->askForPlayerChosen(eri, room->getAllPlayers(), objectName());
                use.to.clear();
                use.to << target;
                data = QVariant::fromValue(use);
            }
        }
        return false;
    }
};

class Fenju: public TriggerSkill{
public:
    Fenju():TriggerSkill("fenju"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard){
            player->tag["cardnum"] = player->getHandcardNum();
        }
        else if(player->getPhase() == Player::Finish){
            int drawnum = player->tag.value("cardnum", 0).toInt() - player->getHandcardNum();
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                if(tmp->getGeneral()->isMale())
                    players << tmp;
            }
            if(drawnum > 0 && !players.isEmpty() && player->askForSkillInvoke(objectName(), data)){
                ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());
                if(target)
                    target->drawCards(drawnum);
            }
        }
        else if(player->getPhase() == Player::NotActive)
            player->tag["cardnum"] = 0;

        return false;
    }
};

FatingCard::FatingCard(){
    target_fixed = true;
    will_throw = false;
}

void FatingCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{

}


class FatingViewAsSkill:public OneCardViewAsSkill{
public:
    FatingViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@fating";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FatingCard *card = new FatingCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Fating: public TriggerSkill{
public:
    Fating():TriggerSkill("fating"){
        view_as_skill = new FatingViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;

        return !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@fating-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@fating", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            player->obtainCard(judge->card);

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

class Rougu: public ProhibitSkill{
public:
    Rougu():ProhibitSkill("rougu"){

    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card) const{
        if(to->getHandcardNum() < to->getMaxHP())
            return card->inherits("Snatch") || card->inherits("Dismantlement");
        else if(to->getHandcardNum() > to->getMaxHP())
            return card->inherits("SupplyShortage") || card->inherits("Indulgence");
        return false;
    }
};

class RouguSkip: public PhaseChangeSkill{
public:
    RouguSkip():PhaseChangeSkill("#rougu-skip"){
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *reiko) const{
        if(reiko->getPhase() == Player::Discard)
            return reiko->getHandcardNum() <= reiko->getMaxHP();
        return false;
    }
};

class Manyu: public PhaseChangeSkill{
public:
    Manyu():PhaseChangeSkill("manyu"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *genta) const{
        if(genta->getPhase() == Player::Start && genta->isWounded()
            && genta->askForSkillInvoke(objectName())){
            Room *room = genta->getRoom();

            int card_id = room->drawCard();
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
            LogMessage log;
            log.from = genta;
            log.type = "$Manyu";
            log.card_str = QString::number(card_id);
            room->sendLog(log);
            room->getThread()->delay();

            if(Sanguosha->getCard(card_id)->getSuit() == Card::Spade){
                RecoverStruct recover;
                recover.card = Sanguosha->getCard(card_id);
                room->recover(genta, recover);
                room->throwCard(card_id);
            }
            else{
                ServerPlayer *target = room->askForPlayerChosen(genta, room->getOtherPlayers(genta), objectName());
                log.type = "$ManyuTo";
                log.to << target;
                room->obtainCard(target, card_id);
                room->sendLog(log);
            }
        }
        return false;
    }
};

class UnBasicPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->getTypeId() != Card::Basic;
    }
};

class Bantu: public TriggerSkill{
public:
    Bantu():TriggerSkill("bantu"){
        events << SlashProceed;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        LogMessage log;
        log.type = "#BantuEffect";
        log.from = effect.from;
        log.to << effect.to;
        log.arg = objectName();
        room->sendLog(log);

        const Card *jink = room->askForCard(effect.to, ".unbasic", "bantu-jink:" + effect.from->objectName());
        room->slashResult(effect, jink);

        return true;
    }
};

TuanzhangCard::TuanzhangCard(){
}

bool TuanzhangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getKingdom() == "shao" && to_select->isWounded();
}

void TuanzhangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *kojima = room->getLord();
    if(!kojima->hasLordSkill("tuanzhang"))
        return;
    const Card *peach = Sanguosha->getCard(this->getSubcards().first());
    CardUseStruct use;
    use.card = peach;
    use.from = source;
    use.to << targets.first();
    room->useCard(use);
}

class TuanzhangViewAsSkill: public OneCardViewAsSkill{
public:
    TuanzhangViewAsSkill():OneCardViewAsSkill("tuanzhangv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getKingdom() == "shao";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->inherits("Peach");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TuanzhangCard *card = new TuanzhangCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Tuanzhang: public GameStartSkill{
public:
    Tuanzhang():GameStartSkill("tuanzhang$"){
    }

    virtual void onGameStart(ServerPlayer *kojima) const{
        if(!kojima->isLord())
            return;
        Room *room = kojima->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "tuanzhangv");
        }
    }
};

class Nijian: public TriggerSkill{
public:
    Nijian():TriggerSkill("nijian"){
        events << Predamaged;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *heiji, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        const Card *card = damage.card;
        if(!card->inherits("Slash") && !card->inherits("Duel"))
            return false;
        if(!heiji->askForSkillInvoke(objectName()))
            return false;
        Room *room = heiji->getRoom();
        QString suit_str = card->getSuitString();
        QString pattern = QString("..%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@nijian:%1::%2").arg(damage.from->getGeneralName()).arg(suit_str);
        if(room->askForCard(heiji, pattern, prompt, data)){
            damage.to = damage.from;
            room->damage(damage);
            return true;
        }
        return false;
    }
};

class Zhenwu: public PhaseChangeSkill{
public:
    Zhenwu():PhaseChangeSkill("zhenwu"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *okida) const{
        Room *room = okida->getRoom();
        if(okida->getPhase() == Player::Draw || okida->getPhase() == Player::Play){
            if(room->getTag("Zhenwu").isNull() && okida->askForSkillInvoke(objectName())){
                QString choice = room->askForChoice(okida, objectName(), "slash+ndtrick");
                room->setTag("Zhenwu", QVariant::fromValue(choice));
                LogMessage log;
                log.type = "#Zhenwu";
                log.from = okida;
                log.arg = choice;
                log.arg2 = objectName();
                room->sendLog(log);
                return true;
            }
        }
        else if(okida->getPhase() == Player::Start)
            room->removeTag("Zhenwu");
        return false;
    }
};

class ZhenwuEffect: public TriggerSkill{
public:
    ZhenwuEffect():TriggerSkill("#zhenwu_eft"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        ServerPlayer *okida = target->getRoom()->findPlayerBySkillName("zhenwu");
        return okida;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QString zhenwutag = room->getTag("Zhenwu").toString();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->isRed())
            return false;
        if((use.card->inherits("Slash") && zhenwutag == "slash") ||
           (use.card->isNDTrick() && zhenwutag == "ndtrick")){
            LogMessage log;
            log.type = "#ZhenwuEffect";
            log.from = player;
            log.arg = "zhenwu";
            log.arg2 = use.card->objectName();
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

HuachiCard::HuachiCard(){

}

bool HuachiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale();
}

void HuachiCard::onEffect(const CardEffectStruct &effect) const{
    DummyCard *cards = effect.from->wholeHandCards();
    Room *room = effect.from->getRoom();
    if(cards){
        room->moveCardTo(cards, effect.to, Player::Hand, false);
        delete cards;
        effect.to->gainMark("@flower");
        effect.from->tag["Kyo"] = QVariant::fromValue(effect.to);
    }
}

class HuachiViewAsSkill: public ZeroCardViewAsSkill{
public:
    HuachiViewAsSkill():ZeroCardViewAsSkill("huachi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@huachi";
    }

    virtual const Card *viewAs() const{
        return new HuachiCard;
    }
};

class Huachi: public PhaseChangeSkill{
public:
    Huachi():PhaseChangeSkill("huachi"){
        view_as_skill = new HuachiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) || target->getMark("@flower") > 0;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *sonoko) const{
        Room *room = sonoko->getRoom();
        if(sonoko->getMark("@flower") > 0){
            if(sonoko->getPhase() == Player::NotActive){
                sonoko->loseAllMarks("@flower");
            }
            return false;
        }
        if(sonoko->getPhase() == Player::Discard && sonoko->getHandcardNum() >= 2)
            room->askForUseCard(sonoko, "@@huachi", "@huachi");
        return false;
    }
};

class Huhua: public TriggerSkill{
public:
    Huhua():TriggerSkill("huhua"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *kyo = player->tag["Kyo"].value<PlayerStar>();
        if(!kyo || kyo->getMark("@flower") == 0)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        damage.to = kyo;

        LogMessage log;
        log.type = "#Huhua";
        log.from = player;
        log.to << kyo;
        log.arg = QString::number(damage.damage);
        if(damage.nature == DamageStruct::Normal)
            log.arg2 = "normal_nature";
        else if(damage.nature == DamageStruct::Fire)
            log.arg2 = "fire_nature";
        else
            log.arg2 = "thunder_nature",
        room->sendLog(log);

        room->damage(damage);
        return true;
    }
};

class Ouxiang: public PhaseChangeSkill{
public:
    Ouxiang():PhaseChangeSkill("ouxiang"){
    }

    virtual bool onPhaseChange(ServerPlayer *youko) const{
        if(youko->tag.value("Grandma", false).toBool() || youko->getPhase() != Player::Start)
            return false;
        if(youko->askForSkillInvoke(objectName())){
            youko->drawCards(youko->getHp());
            youko->skip();
        }
        return false;
    }
};

class Qingchun: public PhaseChangeSkill{
public:
    Qingchun():PhaseChangeSkill("qingchun"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Finish
                && target->hasSkill("qingchun")
                && target->isAlive()
                && !target->tag.value("Grandma", false).toBool();
    }

    virtual bool onPhaseChange(ServerPlayer *youko) const{
        youko->gainMark("@qingchun");
        if(youko->getMark("@qingchun") >= 5){
            LogMessage log;
            log.type = "#Qingchun";
            log.from = youko;
            log.arg = QString::number(youko->getMark("@qingchun"));
            log.arg2 = objectName();
            youko->getRoom()->sendLog(log);

            youko->tag["Grandma"] = true;
        }
        return false;
    }
};


class YunchouClear: public PhaseChangeSkill{
public:
    YunchouClear():PhaseChangeSkill("#yunchou_clear"){

    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->tag.value("Yunchou", "").toString() != "")
                    player->tag.remove("Yunchou");
            }
        }
        return false;
    }
};

YunchouCard::YunchouCard(){
    once = true;
    will_throw = false;
}

void YunchouCard::onEffect(const CardEffectStruct &effect) const{
    QString choice = Sanguosha->getCard(this->getSubcards().first())->getType();
    effect.to->tag["Yunchou"] = choice;

    LogMessage log;
    log.type = "#Yunchou";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = choice;
    log.arg2 = "yunchou";
    effect.to->obtainCard(this);
    effect.from->getRoom()->sendLog(log);
}

class Yunchou: public OneCardViewAsSkill{
public:
    Yunchou():OneCardViewAsSkill("yunchou"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YunchouCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YunchouCard *card = new YunchouCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class YunchouEffect: public TriggerSkill{
public:
    YunchouEffect():TriggerSkill("#yunchou_eft"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->tag.value("Yunchou", "").toString() != "";
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QString type = player->tag.value("Yunchou", "").toString();
        if(use.card->getType() == type){
            LogMessage log;
            log.type = "#YunchouEffect";
            log.from = player;
            log.arg2 = "yunchou";
            player->getRoom()->sendLog(log);

            player->getRoom()->throwCard(use.card->getId());
            return true;
        }
        return false;
    }
};

class Weiwo: public TriggerSkill{
public:
    Weiwo():TriggerSkill("weiwo"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        Room *room = player->getRoom();
        if((move->from_place == Player::Hand)
            && move->to != player
            && player->askForSkillInvoke(objectName(), data)){
            JudgeStruct judge;
            judge.reason = objectName();
            judge.who = player;
            room->judge(judge);
            if(judge.card->getType() == Sanguosha->getCard(move->card_id)->getType())
                player->obtainCard(judge.card);
        }
        return false;
    }
};

class LingjiaPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return player->getMark("lingjia") == card->getNumber();
    }
};

class Lingjia: public TriggerSkill{
public:
    Lingjia():TriggerSkill("lingjia$"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->getLord()->hasLordSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *heizou = room->getLord();
        if(!heizou->hasLordSkill(objectName()))
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach") && (use.to.isEmpty() || use.to.first()->getHp() > 0)){
            QList<ServerPlayer *> zhenjing;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(heizou)){
                if(tmp == use.from || (!use.to.isEmpty() && use.to.contains(tmp)))
                    continue;
                if(tmp->getKingdom() == "zhen" || tmp->getKingdom() == "jing")
                    zhenjing << tmp;
            }
            QString prompt = QString("@lingjia:%1::%2").arg(use.from->getGeneralName()).arg(use.card->getNumberString());
            if(zhenjing.isEmpty())
                return false;
            room->setPlayerMark(heizou, "lingjia", use.card->getNumber());
            if(room->askForCard(heizou, ".Lj", prompt, data)){
                ServerPlayer *target = room->askForPlayerChosen(heizou, zhenjing, objectName());
                target->obtainCard(use.card);
                return true;
            }
        }
        return false;
    }
};

class EquipPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("EquipCard");
    }
};

class Yinsi: public TriggerSkill{
public:
    Yinsi():TriggerSkill("yinsi"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *toyama = room->findPlayerBySkillName(objectName());
        if(!toyama || toyama->isNude())
            return false;
        int peach = 0, equip = 0;
        foreach(const Card *tmp, toyama->getHandcards()){
            if(tmp->inherits("Peach"))
                peach ++;
            if(tmp->inherits("EquipCard"))
                equip ++;
        }
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isDead())
            return false;
        QString choice = "cancel";
        toyama->tag["YinsTarget"] = QVariant::fromValue(damage.to);
        if(peach > 0 && (toyama->inMyAttackRange(damage.to) || equip > 0)){
            choice = room->askForChoice(toyama, objectName(), "friend+enemy+cancel");
        }
        else if(peach > 0){
            choice = room->askForChoice(toyama, objectName(), "friend+cancel");
        }
        if(choice == "cancel")
            return false;
        if(choice == "friend"){
            const Card *peach = room->askForCard(toyama, "peach", "@yinsi-friend:" + damage.to->objectName(), data);
            if(peach){
                CardUseStruct use;
                use.card = peach;
                use.from = toyama;
                use.to << damage.to;
                room->useCard(use);
            }
        }
        else{
            const Card *equip = room->askForCard(toyama, ".Ep", "@yinsi-enemy:" + damage.to->objectName(), data);
            if(equip){
                FireSlash *slash = new FireSlash(equip->getSuit(), equip->getNumber());
                slash->setSkillName(objectName());
                CardUseStruct use;
                use.card = slash;
                use.from = toyama;
                use.to << damage.to;
                room->useCard(use);
            }
        }
        return false;
    }
};

WeijiaoCard::WeijiaoCard(){
}

bool WeijiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    if(targets.length() == 1)
        return !to_select->isKongcheng() && to_select->getGender() != targets.first()->getGender();
    else
        return !to_select->isKongcheng();
}

bool WeijiaoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void WeijiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *from = targets.at(0);
    ServerPlayer *to = targets.at(1);

    bool success = from->pindian(to, "weijiao", room->askForCardShow(from, source, "@weijiao-ask:" + to->objectName()));
    DamageStruct damage;
    damage.card = NULL;
    if(success){
        damage.from = from;
        damage.to = to;
    }
    else{
        damage.from = to;
        damage.to = from;
    }
    room->damage(damage);
}

class WeijiaoViewAsSkill: public ZeroCardViewAsSkill{
public:
    WeijiaoViewAsSkill():ZeroCardViewAsSkill("weijiao"){
    }

    virtual const Card *viewAs() const{
        return new WeijiaoCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@weijiao";
    }
};

class Weijiao:public PhaseChangeSkill{
public:
    Weijiao():PhaseChangeSkill("weijiao"){
        view_as_skill = new WeijiaoViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Judge){
            Room *room = player->getRoom();
            int count = 0;
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                if(!player->isKongcheng()){
                    count ++;
                }
                if(count == 2)
                    break;
            }
            if(count == 2 && room->askForUseCard(player, "@@weijiao", "@weijiao-card"))
                return true;
        }
        return false;
    }
};

class Shiyi: public TriggerSkill{
public:
    Shiyi():TriggerSkill("shiyi"){
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *nakamori = room->findPlayerBySkillName(objectName());
        if(!nakamori)
            return false;
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason == "weijiao" && nakamori->askForSkillInvoke(objectName())){
            nakamori->skip(Player::Draw);
            nakamori->obtainCard(pindian->from_card);
            nakamori->obtainCard(pindian->to_card);
        }
        return false;
    }
};

WeixiaoCard::WeixiaoCard(){
    once = true;
    will_throw = false;
}

bool WeixiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale();
}

void WeixiaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->obtainCard(this);
    LogMessage log;
    log.from = effect.to;
    log.to << effect.from;
    log.type = "#Weixiao";
    room->sendLog(log);

    effect.to->gainMark("@smile");
}

class WeixiaoViewAsSkill: public OneCardViewAsSkill{
public:
    WeixiaoViewAsSkill():OneCardViewAsSkill("weixiao"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("WeixiaoCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        WeixiaoCard *card = new WeixiaoCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Weixiao: public TriggerSkill{
public:
    Weixiao():TriggerSkill("weixiao"){
        events << Predamage << PhaseChange;
        view_as_skill = new WeixiaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@smile") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange){
            if(player->getPhase() == Player::Finish)
                player->loseAllMarks("@smile");
        }
        else{
            DamageStruct damage = data.value<DamageStruct>();
            if(!damage.card)
                return false;

            if(damage.card->inherits("Slash") || damage.card->inherits("Duel")){
                LogMessage log;
                log.type = "#WeixiaoBuff";
                log.from = damage.from;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                room->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Qianmian: public TriggerSkill{
public:
    Qianmian():TriggerSkill("qianmian"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        Room *room = target->getRoom();
        return !target->isLord() && target->hasSkill(objectName()) && room->getMode() != "06_3v3";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        player->tag["Qmmv"] = player->getRole();
        QString role = room->askForChoice(player, objectName(), "renegade+rebel+loyalist+cancel");
        if(role != "cancel"){
            LogMessage log;
            log.type = "#Qianmian";
            log.from = player;
            log.arg = role;
            log.arg2 = objectName();
            room->sendLog(log);

            player->tag["Qmmv"] = role;
        }
        return false;
    }
};

class Kuai: public TriggerSkill{
public:
    Kuai():TriggerSkill("kuai$"){
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "hei";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *ver = room->getLord();
        if(!ver || !ver->hasLordSkill(objectName()))
            return false;
        if(ver->askForSkillInvoke(objectName())){
            DamageStruct damage = data.value<DamageStruct>();
            ver->drawCards(damage.damage);
        }
        return false;
    }
};

class Dianwan: public DrawCardsSkill{
public:
    Dianwan():DrawCardsSkill("dianwan"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *jodie, int n) const{
        Room *room = jodie->getRoom();
        if(jodie->isWounded() && room->askForSkillInvoke(jodie, objectName()))
            return n + 1;
        else
            return n;
    }
};

ShuangyuCard::ShuangyuCard(){

}

bool ShuangyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    return true;
}

bool ShuangyuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void ShuangyuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QString myrole = source->getRole();
    QList<ServerPlayer *> lucky_players, cup_players;
    foreach(ServerPlayer *player, targets){
        QString result = room->askForChoice(player, "shuangyu", "lord+loyalist+rebel+renegade");
        if(result == myrole)
            lucky_players << player;
        else{
            cup_players << player;
        }
    }
    source->loseMark("@two");
    foreach(ServerPlayer *player, lucky_players){
        LogMessage log;
        log.type = "#ShuangyuAdd";
        log.from = player;
        log.to << source;
        log.arg = QString::number(1);
        room->sendLog(log);

        room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
    }
    foreach(ServerPlayer *player, cup_players){
        LogMessage log;
        log.type = "#ShuangyuReduce";
        log.from = player;
        log.to << source;
        log.arg = QString::number(3);
        room->sendLog(log);

        room->setPlayerProperty(player, "hp", player->getHp() - 3);
        room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(-3));
        if(player->getHp() <= 0)
            room->enterDying(player, NULL);
    }
}

class Shuangyu: public ZeroCardViewAsSkill{
public:
    Shuangyu():ZeroCardViewAsSkill("shuangyu"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@two") > 0;
    }

    virtual const Card *viewAs() const{
        return new ShuangyuCard;
    }
};

class Juanxiu: public ProhibitSkill{
public:
    Juanxiu():ProhibitSkill("juanxiu"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(to->getPhase() == Player::NotActive && !from->inMyAttackRange(to))
            return card->inherits("TrickCard");
        return false;
    }
};

class Qingdi: public TriggerSkill{
public:
    Qingdi():TriggerSkill("qingdi"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip && player && room->askForSkillInvoke(player, objectName())){
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = player;
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
            use.to << target;

            room->useCard(use, false);
        }
        return false;
    }
};

ZhiyuCard::ZhiyuCard(){
    once = true;
}

bool ZhiyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool ZhiyuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void ZhiyuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void ZhiyuCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

class ZhiyuViewAsSkill: public OneCardViewAsSkill{
public:
    ZhiyuViewAsSkill():OneCardViewAsSkill("zhiyu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhiyuCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhiyuCard *card = new ZhiyuCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Zhiyu: public PhaseChangeSkill{
public:
    Zhiyu():PhaseChangeSkill("zhiyu"){
        view_as_skill = new ZhiyuViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::NotActive &&
           player->isWounded() &&
           player->getSlashCount() == 0 &&
           player->askForSkillInvoke(objectName())){
            RecoverStruct rec;
            rec.who = player;
            rec.card = NULL;
            player->getRoom()->recover(player, rec);
        }
        return false;
    }
};

class Yanshi: public TriggerSkill{
public:
    Yanshi():TriggerSkill("yanshi"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isNude())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *tomi = room->findPlayerBySkillName(objectName());
        if(tomi && tomi->isAlive() && room->askForSkillInvoke(tomi, objectName(), data)){
            tomi->obtainCard(player->getWeapon());
            tomi->obtainCard(player->getArmor());
            tomi->obtainCard(player->getDefensiveCar());
            tomi->obtainCard(player->getOffensiveCar());

            DummyCard *all_cards = player->wholeHandCards();
            if(all_cards){
                room->moveCardTo(all_cards, tomi, Player::Hand, false);
                delete all_cards;
            }
        }
        return false;
    }
};

class Dushu: public TriggerSkill{
public:
    Dushu():TriggerSkill("dushu"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        ServerPlayer *git = dying_data.who;
        Room *room = player->getRoom();
        ServerPlayer *tomy = room->findPlayerBySkillName(objectName());
        if(tomy && tomy->inMyAttackRange(git)){
            const Card *poison = room->askForCard(tomy, "peach", "@dushu:" + git->objectName(), QVariant::fromValue(git));
            if(poison){
                LogMessage log;
                log.type = "#Dushu";
                log.from = tomy;
                log.arg = objectName();
                log.to << git;
                room->sendLog(log);

                DamageStruct damage;
                damage.from = tomy;
                damage.to = git;
                room->killPlayer(git, &damage);
                return true;
            }
        }
        return false;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{

    General *kisakieri = new General(this, "kisakieri", "zhen", 3, false);
    kisakieri->addSkill(new Bianhu);
    kisakieri->addSkill(new Fenju);

    General *kujoureiko = new General(this, "kujoureiko", "zhen", 3, false);
    kujoureiko->addSkill(new Fating);
    kujoureiko->addSkill(new Rougu);
    kujoureiko->addSkill(new RouguSkip);
    related_skills.insertMulti("rougu", "#rougu-skip");

    General *kojimagenta = new General(this, "kojimagenta$", "shao", 3);
    kojimagenta->addSkill(new Manyu);
    kojimagenta->addSkill(new Bantu);
    patterns[".unbasic"] = new UnBasicPattern;
    kojimagenta->addSkill(new Tuanzhang);
    skills << new TuanzhangViewAsSkill;

    General *heiji = new General(this, "heiji", "woo");
    heiji->addSkill(new Nijian);

    General *okidasouji = new General(this, "okidasouji", "woo");
    okidasouji->addSkill(new Zhenwu);
    okidasouji->addSkill(new ZhenwuEffect);
    related_skills.insertMulti("zhenwu", "#zhenwu_eft");

    General *suzukisonoko = new General(this, "suzukisonoko", "yi", 3, false);
    suzukisonoko->addSkill(new Huachi);
    suzukisonoko->addSkill(new Huhua);

    General *okinoyouko = new General(this, "okinoyouko", "yi", 3, false);
    okinoyouko->addSkill(new Ouxiang);
    okinoyouko->addSkill(new Qingchun);

    General *hattoriheizou = new General(this, "hattoriheizou$", "jing", 3);
    hattoriheizou->addSkill(new Yunchou);
    hattoriheizou->addSkill(new YunchouEffect);
    related_skills.insertMulti("yunchou", "#yunchou_eft");
    hattoriheizou->addSkill(new YunchouClear);
    related_skills.insertMulti("yunchou", "#yunchou_clear");
    hattoriheizou->addSkill(new Weiwo);
    hattoriheizou->addSkill(new Lingjia);
    patterns[".Lj"] = new LingjiaPattern;

    General *touyamaginshirou = new General(this, "touyamaginshirou", "jing");
    touyamaginshirou->addSkill(new Yinsi);
    patterns[".Ep"] = new EquipPattern;

    General *nakamoriginzou = new General(this, "nakamoriginzou", "guai");
    nakamoriginzou->addSkill(new Weijiao);
    nakamoriginzou->addSkill(new Shiyi);

    General *vermouth = new General(this, "vermouth$", "hei", 4, false);
    vermouth->addSkill(new Weixiao);
    vermouth->addSkill(new Qianmian);
    vermouth->addSkill(new Kuai);

    General *jodie = new General(this, "jodie", "te", 3, false);
    jodie->addSkill(new Dianwan);
    jodie->addSkill(new Shuangyu);
    jodie->addSkill(new MarkAssignSkill("@two", 1));
    related_skills.insertMulti("shuangyu", "#@two-1");
    jodie->addSkill(new Juanxiu);

    General *araidetomoaki = new General(this, "araidetomoaki", "za", 3);
    araidetomoaki->addSkill(new Qingdi);
    araidetomoaki->addSkill(new Zhiyu);

    General *tomesan = new General(this, "tomesan", "za");
    tomesan->addSkill(new Yanshi);
    tomesan->addSkill(new Dushu);

    addMetaObject<FatingCard>();
    addMetaObject<TuanzhangCard>();
    addMetaObject<HuachiCard>();
    addMetaObject<YunchouCard>();
    addMetaObject<WeijiaoCard>();
    addMetaObject<WeixiaoCard>();
    addMetaObject<ShuangyuCard>();
    addMetaObject<ZhiyuCard>();
}

ADD_PACKAGE(Wind)


