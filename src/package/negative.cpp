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
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guilin");
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &) const{
        if(event == PhaseChange && player->getPhase() == Player::Judge && player->getJudgingArea().length()!=0){
            Room *room = player->getRoom();
            ServerPlayer *three = room->findPlayerBySkillName(objectName());
            if(!three->askForSkillInvoke(objectName()))
                return false;
  //          CardEffectStruct effect = data.value<CardEffectStruct>();
            QList<const Card *> three_cards = three->getJudgingArea();
            QList<const Card *> cards = player->getJudgingArea();
            foreach(const Card *card, cards){
               room->moveCardTo(card, three, Player::Judging);
            }
            foreach(const Card *card, three_cards){
               room->moveCardTo(card, player, Player::Judging);
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

    General *mizunashi = new General(this, "mizunashi", "yi", 4, false);
    mizunashi->addSkill(new Ruoshui);
}

ADD_PACKAGE(Negative)
