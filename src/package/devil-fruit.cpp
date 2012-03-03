#include "devil-fruit.h"
#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "settings.h"

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

//attack-fruit
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
            foreach(ServerPlayer *tmp, room->getLieges(zhurong->getKingdom(), zhurong))
                if(tmp->inMyAttackRange(damage.to))
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

//recovery-fruit
class Papaya: public PhaseChangeSkill{
public:
    Papaya():PhaseChangeSkill("papaya$"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Start && mouri->isWounded() &&
           room->askForSkillInvoke(mouri, objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = mouri;

            room->judge(judge);
            if(judge.isGood()){
                foreach(ServerPlayer *target, room->getLieges(mouri->getKingdom(), mouri)){
                    if(room->askForCard(target, "..", "@papaya:" + mouri->objectName())){
                        RecoverStruct r;
                        r.who = target;
                        room->recover(mouri, r);
                        break;
                    }
                }
            }
        }
        return false;
    }
};

//defense-fruit


DevilFruitPackage::DevilFruitPackage()
    :Package("devil_fruit")
{
    type = CardPack;
    skills
            << new Orange
            << new Papaya
            << new Skill("grape$")
            ;
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
