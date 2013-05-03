#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "thunder.h"

class MoonSpearSkill: public WeaponSkill{
public:
    MoonSpearSkill():WeaponSkill("moon_spear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card == NULL || !card->isBlack())
            return false;

        Room *room = player->getRoom();
        room->askForUseCard(player, "slash", "@moon-spear-slash");

        return false;
    }
};

class MoonSpear: public Weapon{
public:
    MoonSpear(Suit suit = Card::Diamond, int number = 12)
        :Weapon(suit, number, 3){
        setObjectName("moon_spear");
        skill = new MoonSpearSkill;
    }
};

class SPMoonSpearSkill: public WeaponSkill{
public:
    SPMoonSpearSkill():WeaponSkill("sp_moonspear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(!card || !card->isBlack())
            return false;

        Room *room = player->getRoom();
        if(!room->askForSkillInvoke(player, objectName(), data))
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(player->inMyAttackRange(tmp))
                targets << tmp;
        }
        if(targets.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        if(!room->askForCard(target, "jink", "@moon-spear-jink")){
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

class SPMoonSpear: public Weapon{
public:
    SPMoonSpear(Suit suit = Card::Heart, int number = 12)
        :Weapon(suit, number, 3){
        setObjectName("sp_moonspear");
        skill = new SPMoonSpearSkill;
    }
};

ThunderPackage::ThunderPackage()
    :Package("thunder")
{
    type = CardPack;

    (new MoonSpear)->setParent(this);
    (new SPMoonSpear)->setParent(this);
}

ADD_PACKAGE(Thunder)
