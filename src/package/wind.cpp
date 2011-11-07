#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Bianhu: public TriggerSkill{
public:
    Bianhu():TriggerSkill("bianhu"){
        events << CardUsed;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
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
            if(room->askForCard(eri, pattern, prompt)){
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
            if(drawnum > 0 && player->askForSkillInvoke(objectName(), data)){
                ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
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
            room->getThread()->delay();
            if(Sanguosha->getCard(card_id)->getSuit() == Card::Spade){
                RecoverStruct recover;
                recover.card = Sanguosha->getCard(card_id);
                room->recover(genta, recover);
            }
            else{
                ServerPlayer *target = room->askForPlayerChosen(genta, room->getOtherPlayers(genta), objectName());
                room->obtainCard(target, card_id);
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
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@nijian:%1::%2").arg(damage.from->getGeneralName()).arg(suit_str);
        if(room->askForCard(heiji, pattern, prompt)){
            DamageStruct damage2 = damage;
            damage2.to = damage.from;
            room->damage(damage2);
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
        if(use.card->inherits("Slash") && zhenwutag == "slash")
            return true;
        if(use.card->isNDTrick() && zhenwutag == "ndtrick")
            return true;
        return false;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    General *kisakieri, *kujoureiko, *kojimagenta, *heiji, *okidasouji,
            *suzukisonoko, *okinoyouko, *hattoriheizou, *touyamaginshirou,
            *nakamoriginzou, *vermouth, *jodie, *araidetomoaki, *tomesan;

    kisakieri = new General(this, "kisakieri", "zhen", 3, false);
    kisakieri->addSkill(new Bianhu);
    kisakieri->addSkill(new Fenju);

    kujoureiko = new General(this, "kujoureiko", "zhen", 3, false);
    kujoureiko->addSkill(new Fating);
    kujoureiko->addSkill(new Rougu);
    kujoureiko->addSkill(new RouguSkip);
    related_skills.insertMulti("rougu", "#rougu-skip");

    kojimagenta = new General(this, "kojimagenta$", "shao", 3);
    kojimagenta->addSkill(new Manyu);
    kojimagenta->addSkill(new Bantu);
    patterns[".unbasic"] = new UnBasicPattern;
    kojimagenta->addSkill(new Tuanzhang);
    skills << new TuanzhangViewAsSkill;

    heiji = new General(this, "heiji", "woo");
    heiji->addSkill(new Nijian);

    okidasouji = new General(this, "okidasouji", "woo");
    okidasouji->addSkill(new Zhenwu);
    okidasouji->addSkill(new ZhenwuEffect);
    related_skills.insertMulti("zhenwu", "#zhenwu_eft");

    suzukisonoko = new General(this, "suzukisonoko", "yi");
    okinoyouko = new General(this, "okinoyouko", "yi");
    hattoriheizou = new General(this, "hattoriheizou", "jing");
    touyamaginshirou = new General(this, "touyamaginshirou", "jing");
    nakamoriginzou = new General(this, "nakamoriginzou", "guai");
    vermouth = new General(this, "vermouth", "hei");
    jodie = new General(this, "jodie", "te");
    araidetomoaki = new General(this, "araidetomoaki", "za");
    tomesan = new General(this, "tomesan", "za");

    addMetaObject<FatingCard>();
    addMetaObject<TuanzhangCard>();
}

ADD_PACKAGE(Wind)


