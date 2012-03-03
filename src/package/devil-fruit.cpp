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

class Mango: public TriggerSkill{
public:
    Mango():TriggerSkill("mango$"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(!killer || !killer->hasLordSkill(objectName()))
            return false;
        if(killer->isAlive())
            killer->drawCards(2);
        return false;
    }
};

class Starfruit: public TriggerSkill{
public:
    Starfruit():TriggerSkill("starfruit$"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.to->getKingdom() != effect.from->getKingdom() && player->askForSkillInvoke(objectName()))
            player->drawCards(1);
        return false;
    }
};

HoneymelonCard::HoneymelonCard(){

}

bool HoneymelonCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void HoneymelonCard::use(Room *room, ServerPlayer *liubei, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges(liubei->getKingdom(), liubei);
    const Card *slash = NULL;

    QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
    foreach(ServerPlayer *liege, lieges){
        slash = room->askForCard(liege, "slash", "@honeymelon-slash:" + liubei->objectName(), tohelp);
        if(slash){
            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = liubei;
            card_use.to << targets.first();

            room->useCard(card_use);
            return;
        }
    }
}

class HoneymelonViewAsSkill:public ZeroCardViewAsSkill{
public:
    HoneymelonViewAsSkill():ZeroCardViewAsSkill("honeymelon$"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("honeymelon") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new HoneymelonCard;
    }
};

class Honeymelon: public TriggerSkill{
public:
    Honeymelon():TriggerSkill("honeymelon$"){
        events << CardAsked;
        view_as_skill = new HoneymelonViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("honeymelon");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = liubei->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges(liubei->getKingdom(), liubei);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(liubei, objectName()))
            return false;

        QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@honeymelon-slash:" + liubei->objectName(), tohelp);
            if(slash){
                room->provide(slash);
                return true;
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

class Durian: public TriggerSkill{
public:
    Durian():TriggerSkill("durian$"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from && damage.damage > 1 &&
           damage.from->getKingdom() != player->getKingdom()){
            Room *room = damage.to->getRoom();

            LogMessage log;
            log.type = "#DurianProtect";
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            room->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Apple: public TriggerSkill{
public:
    Apple():TriggerSkill("apple$"){
        events << AskForPeachesDone << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("apple");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunquan, QVariant &data) const{
        Room *room = sunquan->getRoom();
        switch(event){
        case CardEffected: {
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(effect.card->inherits("Peach") && effect.from->getKingdom() == sunquan->getKingdom()
                   && sunquan != effect.from && sunquan->hasFlag("dying")){
                    sunquan->setFlags("apple");

                    LogMessage log;
                    log.type = "#AppleExtraRecover";
                    log.from = sunquan;
                    log.to << effect.from;
                    room->sendLog(log);

                    RecoverStruct recover;
                    recover.who = effect.from;
                    room->recover(sunquan, recover);

                    room->getThread()->delay(2000);
                }
                break;
            }
        case AskForPeachesDone:{
                sunquan->setFlags("-apple");
                break;
            }
        default:
            break;
        }
        return false;
    }
};

//defense-fruit
class Pineapple:public TriggerSkill{
public:
    Pineapple():TriggerSkill("pineapple$"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("pineapple");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *caocao, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        Room *room = caocao->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges(caocao->getKingdom(), caocao);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->playSkillEffect(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)caocao);
        foreach(ServerPlayer *liege, lieges){
            const Card *jink = room->askForCard(liege, "jink", "@pineapple-jink:" + caocao->objectName(), tohelp);
            if(jink){
                room->provide(jink);
                return true;
            }
        }
        return false;
    }
};

class Lemon:public MasochismSkill{
public:
    Lemon():MasochismSkill("lemon$"){
    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        if(!damage.from || damage.from->getKingdom() == xiahou->getKingdom())
            return;
        if(xiahou->askForSkillInvoke(objectName()))
            xiahou->drawCards(1);
    }
};
HuangtianCard::HuangtianCard(){
    once = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasLordSkill("huangtian")){
        zhangjiao->obtainCard(this);
        room->setEmotion(zhangjiao, "good");
    }
}

bool HuangtianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian") && to_select != Self;
}
class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("HuangtianCard") && player->getKingdom() == "qun";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huangtian: public GameStartSkill{
public:
    Huangtian():GameStartSkill("huangtian$"){

    }

    virtual void onGameStart(ServerPlayer *zhangjiao) const{
        Room *room = zhangjiao->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "huangtianv");
        }
    }
};

DevilFruitPackage::DevilFruitPackage()
    :Package("devil_fruit")
{
    type = CardPack;
    skills
            << new Orange << new Mango << new Starfruit << new Honeymelon
            << new Papaya << new Durian << new Apple
            << new Skill("grape$") << new Pineapple << new Lemon
            ;
    addMetaObject<HoneymelonCard>();
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "uzumaki", "god", 5, true, true);
    new General(this, "haruno", "god", 5, false, true);

    addMetaObject<CheatCard>();
    addMetaObject<HuangtianCard>();
}

ADD_PACKAGE(DevilFruit)
ADD_PACKAGE(Test)
