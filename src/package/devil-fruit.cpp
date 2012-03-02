#include "general.h"
#include "standard.h"
#include "devil-fruit.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

class Orange: public TriggerSkill{
public:
    Orange():TriggerSkill("orange$"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash")){
            Room *room = zhurong->getRoom();
            QList<ServerPlayer *>targets;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(zhurong))
                if(tmp->inMyAttackRange(damage.to) && tmp->getKingdom() == zhurong->getKingdom())
                    targets << tmp;
            if(!targets.isEmpty() && room->askForSkillInvoke(zhurong, objectName(), data)){
                ServerPlayer *target = room->askForPlayerChosen(zhurong, targets, objectName());
                QString prompt = QString("@orange:%1:%2")
                                 .arg(zhurong->objectName()).arg(damage.to->objectName());
                const Card *slash = room->askForCard(target, "slash", prompt);
                if(slash)
                    room->cardEffect(slash, target, damage.to);
            }
        }
        return false;
    }
};

DevilFruitPackage::DevilFruitPackage()
    :Package("devil_fruit")
{
    skills << new Orange;
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "uzumaki", "god", 5, true, true);
    new General(this, "haruno", "god", 5, false, true);

    addMetaObject<CheatCard>();
}

ADD_PACKAGE(DevilFruit)
ADD_PACKAGE(Test)
