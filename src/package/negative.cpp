#include "negative.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"

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
    General *mizunashi = new General(this, "mizunashi", "yi", 4, false);
    mizunashi->addSkill(new Ruoshui);
}

ADD_PACKAGE(Negative)
