#include "negative.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"

class Zhenwu: public PhaseChangeSkill{
public:
    Zhenwu():PhaseChangeSkill("zhenwu"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *satoshi) const{
        Room *room = satoshi->getRoom();
        if(satoshi->getPhase() == Player::Draw || satoshi->getPhase() == Player::Play){
            if(room->getTag("Zhenwu").isNull() && satoshi->askForSkillInvoke(objectName())){
                QString choice = room->askForChoice(satoshi, objectName(), "slash+ndtrick");
                room->setTag("Zhenwu", QVariant::fromValue(choice));
                LogMessage log;
                log.type = "#Zhenwu";
                log.from = satoshi;
                log.arg = choice;
                log.arg2 = objectName();
                room->sendLog(log);
                return true;
            }
        }
        else if(satoshi->getPhase() == Player::Start)
            room->removeTag("Zhenwu");
        return false;
    }
};

class ZhenwuEffect: public TriggerSkill{
public:
    ZhenwuEffect():TriggerSkill("#zhenwu_eft"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *satoshi = room->findPlayerBySkillName("zhenwu");
        if(!satoshi)
            return false;
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

class Ruoshui: public TriggerSkill{
public:
    Ruoshui():TriggerSkill("ruoshui"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *three = room->findPlayerBySkillName(objectName());
        if(!three || three == player)
            return false;
        if(player->getPhase() == Player::Judge && !player->getJudgingArea().isEmpty()
                && three->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
            QList<const Card *> three_cards = three->getJudgingArea();
            QList<const Card *> cards = player->getJudgingArea();
            foreach(const Card *card, cards)
               room->moveCardTo(card, three, Player::Judging);
            foreach(const Card *card, three_cards)
               room->moveCardTo(card, player, Player::Judging);
         }
        return false;
    }
};

class Wodi: public TriggerSkill{
public:
    Wodi():TriggerSkill("wodi"){
        events << PhaseChange;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::RoundStart && player->getRole() == "renegade")
            player->setRole("loyalist");
        return false;
    }
};

class Suji: public TriggerSkill{
public:
    Suji():TriggerSkill("suji"){
        events << PhaseChange << CardUsed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed){
            if(player->getPhase() == Player::Play && !player->hasFlag("Sujied")){
                CardUseStruct use = data.value<CardUseStruct>();
                if(!use.card->isVirtualCard()){
                    QVariantList cards = player->tag["Suji"].toList();
                    cards << use.card->getEffectiveId();
                    player->tag["Suji"] = cards;
                    player->setFlags("Sujied");
                }
            }
        }
        else{
            if(player->getPhase() == Player::Start){
                QVariantList cards = player->tag["Suji"].toList();
                if(cards.length() == 3){
                    Room *room = player->getRoom();
                    foreach(QVariant card_id, cards){
                        const Card *card = Sanguosha->getCard(card_id.toInt());
                        room->moveCardTo(card, player, Player::Hand);
                    }
                    player->setFlags("Sujied");
                }
            }
        }
        return false;
    }
};

NegativePackage::NegativePackage()
    :Package("negative")
{
    General *maedasatoshi = new General(this, "maedasatoshi", "woo");
    maedasatoshi->addSkill(new Zhenwu);
    maedasatoshi->addSkill(new ZhenwuEffect);
    related_skills.insertMulti("zhenwu", "#zhenwu_eft");

    General *mizunashireina = new General(this, "mizunashireina", "yi", "3/4", General::Female, General::Hidden);
    mizunashireina->addSkill(new Ruoshui);
    mizunashireina->addSkill(new Wodi);

    General *hanedashuukichi = new General(this, "hanedashuukichi", "zhen");
    hanedashuukichi->addSkill(new Suji);
}

ADD_PACKAGE(Negative)
