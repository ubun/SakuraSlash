#include "evilplus.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "standard.h"
#include "maneuvering.h"

class Ganglie:public MasochismSkill{
public:
    Ganglie():MasochismSkill("ganglie"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if(judge.isGood()){
                if(!room->askForDiscard(from, objectName(), 2, true)){
                    DamageStruct damage;
                    damage.from = xiahou;
                    damage.to = from;

                    room->setEmotion(xiahou, "good");
                    room->damage(damage);
                }
            }else
                room->setEmotion(xiahou, "bad");
        }
    }
};

KurouCard::KurouCard(){
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if(source->isAlive())
        room->drawCards(source, 2);
}

class Kurou: public ZeroCardViewAsSkill{
public:
    Kurou():ZeroCardViewAsSkill("kurou"){
    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Zaiqi: public PhaseChangeSkill{
public:
    Zaiqi():PhaseChangeSkill("zaiqi"){

    }

    virtual bool onPhaseChange(ServerPlayer *menghuo) const{
        if(menghuo->getPhase() == Player::Draw && menghuo->isWounded()){
            Room *room = menghuo->getRoom();
            if(room->askForSkillInvoke(menghuo, objectName())){
                int x = menghuo->getLostHp(), i;

                room->playSkillEffect(objectName(), 1);
                bool has_heart = false;

                for(i=0; i<x; i++){
                    int card_id = room->drawCard();
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);

                    room->getThread()->delay();

                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->getSuit() == Card::Heart){
                        RecoverStruct recover;
                        recover.card = card;
                        recover.who = menghuo;
                        room->recover(menghuo, recover);
                        room->throwCard(card_id);
                        has_heart = true;
                    }else
                        room->obtainCard(menghuo, card_id);
                }

                if(has_heart)
                    room->playSkillEffect(objectName(), 2);
                else
                    room->playSkillEffect(objectName(), 3);

                return true;
            }
        }

        return false;
    }
};

class Qixing: public PhaseChangeSkill{
public:
    Qixing():PhaseChangeSkill("qixing"){
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getMark("@star") > 0;
    }

    static void Exchange(ServerPlayer *shenzhuge){
        QList<int> &stars = shenzhuge->getPile("stars");
        if(stars.isEmpty())
            return;

        Room *room = shenzhuge->getRoom();
        room->fillAG(stars, shenzhuge);

        int ai_delay = Config.AIDelay;
        Config.AIDelay = 0;

        QList<int> to_exchange;
        while(!stars.isEmpty()){
            int card_id = room->askForAG(shenzhuge, stars, true, "qixing");
            if(card_id == -1)
                break;

            stars.removeOne(card_id);
            to_exchange << card_id;

            const Card *card = Sanguosha->getCard(card_id);
            shenzhuge->addCard(card, Player::Hand);
            room->setCardMapping(card_id, shenzhuge, Player::Hand);
            QString take_str = QString("%1:%2").arg(shenzhuge->objectName()).arg(card_id);
            shenzhuge->invoke("takeAG", take_str);

            shenzhuge->invoke("pile", QString("%1:stars-%2").arg(shenzhuge->objectName()).arg(card_id));
        }

        Config.AIDelay = ai_delay;

        shenzhuge->invoke("clearAG");

        if(to_exchange.isEmpty())
            return;

        int n = to_exchange.length();
        const Card *exchange_card = room->askForExchange(shenzhuge, "qixing", n);

        foreach(int card_id, exchange_card->getSubcards()){
            shenzhuge->invoke("pile", QString("%1:stars+%2").arg(shenzhuge->objectName()).arg(card_id));
        }

        LogMessage log;
        log.type = "#QixingExchange";
        log.from = shenzhuge;
        log.arg = QString::number(n);
        room->sendLog(log);

        room->moveCardTo(exchange_card, shenzhuge, Player::Special, false);

        stars.append(exchange_card->getSubcards());

        delete exchange_card;
    }

    static void DiscardStar(ServerPlayer *shenzhuge, int n){
        Room *room = shenzhuge->getRoom();
        QList<int> &stars = shenzhuge->getPile("stars");

        room->fillAG(stars, shenzhuge);

        int i;
        for(i=0; i<n; i++){
            int card_id = room->askForAG(shenzhuge, stars, false, "qixing-discard");

            stars.removeOne(card_id);
            room->throwCard(card_id);

            shenzhuge->invoke("pile", QString("%1:stars-%2").arg(shenzhuge->objectName()).arg(card_id));
        }

        shenzhuge->invoke("clearAG");
    }

    virtual bool onPhaseChange(ServerPlayer *shenzhuge) const{
        if(shenzhuge->getPhase() == Player::Draw){
            Exchange(shenzhuge);
        }

        return false;
    }
};

class QixingStart: public GameStartSkill{
public:
    QixingStart():GameStartSkill("#qixing"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual void onGameStart(ServerPlayer *shenzhuge) const{
        shenzhuge->gainMark("@star", 8);

        Room *room = shenzhuge->getRoom();
        QList<int> stars = room->getNCards(8);
        foreach(int star, stars)
            room->setCardMapping(star, shenzhuge, Player::Special);
        shenzhuge->getPile("stars") = stars;

        foreach(int star, stars){
            shenzhuge->invoke("pile", QString("%1:stars+%2").arg(shenzhuge->objectName()).arg(star));
        }

        Qixing::Exchange(shenzhuge);
    }
};

KuangfengCard::KuangfengCard(){

}

bool KuangfengCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void KuangfengCard::onEffect(const CardEffectStruct &effect) const{
    Qixing::DiscardStar(effect.from, 1);

    effect.from->loseMark("@star");
    effect.to->gainMark("@gale");
}

class KuangfengViewAsSkill: public ZeroCardViewAsSkill{
public:
    KuangfengViewAsSkill():ZeroCardViewAsSkill("kuangfeng"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@kuangfeng";
    }

    virtual const Card *viewAs() const{
        return new KuangfengCard;
    }
};

class Kuangfeng: public TriggerSkill{
public:
    Kuangfeng():TriggerSkill("kuangfeng"){
        view_as_skill = new KuangfengViewAsSkill;

        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@gale") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GalePower";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class QixingAsk: public PhaseChangeSkill{
public:
    QixingAsk():PhaseChangeSkill("#qixing-ask"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if(target->getPhase() == Player::Finish){
            if(target->getMark("@star") > 0)
                room->askForUseCard(target, "@kuangfeng", "@@kuangfeng-card");

            if(target->getMark("@star") > 0)
                room->askForUseCard(target, "@dawu", "@@dawu-card");

            if(target->getMark("@star") > 0)
                room->askForUseCard(target, "@xunlei", "@@xunlei-card");
        }else if(target->getPhase() == Player::Start){
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->getMark("@gale") > 0)
                    player->loseMark("@gale");
                if(player->getMark("@fog") > 0)
                    player->loseMark("@fog");
                if(player->getMark("@thun") > 0)
                    player->loseMark("@thun");
            }
        }

        return false;
    }
};

class QixingClear: public TriggerSkill{
public:
    QixingClear():TriggerSkill("#qixing-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            player->loseAllMarks("@gale");
            player->loseAllMarks("@fog");
        }

        return false;
    }
};

DawuCard::DawuCard(){

}

bool DawuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < Self->getMark("@star");
}

void DawuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int n = targets.length();
    Qixing::DiscardStar(source, n);

    source->loseMark("@star", n);
    foreach(ServerPlayer *target, targets){
        target->gainMark("@fog");
    }
}

class DawuViewAsSkill: public ZeroCardViewAsSkill{
public:
    DawuViewAsSkill():ZeroCardViewAsSkill("dawu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@dawu";
    }

    virtual const Card *viewAs() const{
        return new DawuCard;
    }
};

class Dawu: public TriggerSkill{
public:
    Dawu():TriggerSkill("dawu"){
        view_as_skill = new DawuViewAsSkill;

        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@fog") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
            LogMessage log;
            log.type = "#FogProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            if(damage.nature == DamageStruct::Normal)
                log.arg2 = "normal_nature";
            else log.arg2 = damage.nature == DamageStruct::Fire?"fire_nature":"thunder_nature";
            room->sendLog(log);
            return true;
    }
};

XunleiCard::XunleiCard(){

}

bool XunleiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void XunleiCard::onEffect(const CardEffectStruct &effect) const{
    Qixing::DiscardStar(effect.from, 1);

    effect.from->loseMark("@star");
    effect.to->gainMark("@thun");
}

class XunleiViewAsSkill: public ZeroCardViewAsSkill{
public:
    XunleiViewAsSkill():ZeroCardViewAsSkill("xunlei"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@xunlei";
    }

    virtual const Card *viewAs() const{
        return new XunleiCard;
    }
};

class Xunlei: public TriggerSkill{
public:
    Xunlei():TriggerSkill("xunlei"){
        view_as_skill = new XunleiViewAsSkill;

        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@thun") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){
            LogMessage log;
            log.type = "#ThunPower";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Dushi: public TriggerSkill{
public:
    Dushi():TriggerSkill("dushi"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            Room *room = player->getRoom();
            if(killer != player && !killer->hasSkill("benghuai")){
                killer->gainMark("@collapse");
                room->acquireSkill(killer, "benghuai");
            }
        }

        return false;
    }
};

class Kanpo: public OneCardViewAsSkill{
public:
    Kanpo():OneCardViewAsSkill("kanpo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack() && !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("kanpo");

        return ncard;
    }
};

class Sizhan: public TriggerSkill{
public:
    Sizhan():TriggerSkill("sizhan"){
        events << Predamaged << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *elai, QVariant &data) const{
        if(event == Predamaged){
            Room *room = elai->getRoom();
            if(!room->askForSkillInvoke(elai, "sizhan", data))
                return false;

            DamageStruct damage = data.value<DamageStruct>();

            LogMessage log;
            log.type = "#SizhanPrevent";
            log.from = elai;
            log.arg = QString::number(damage.damage);
            room->sendLog(log);

            elai->gainMark("@struggle", damage.damage);

            return true;
        }else if(event == PhaseChange && elai->getPhase() == Player::Finish){
            int x = elai->getMark("@struggle");
            if(x > 0){
                elai->loseMark("@struggle", x);

                LogMessage log;
                log.type = "#SizhanLoseHP";
                log.from = elai;
                log.arg = QString::number(x);

                Room *room = elai->getRoom();
                room->sendLog(log);
                room->loseHp(elai, x);
            }

            elai->setFlags("-shenli");
        }

        return false;
    }
};

class Shenli: public TriggerSkill{
public:
    Shenli():TriggerSkill("shenli"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *elai, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
           elai->getPhase() == Player::Play && !elai->hasFlag("shenli"))
        {
            elai->setFlags("shenli");

            int x = elai->getMark("@struggle");
            if(x > 0){
//                x = qMin(3, x);
                damage.damage += x;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#ShenliBuff";
                log.from = elai;
                log.arg = QString::number(x);
                log.arg2 = QString::number(damage.damage);
                elai->getRoom()->sendLog(log);
            }
        }

        return false;
    }
};

class Huilei: public TriggerSkill{
public:
    Huilei():TriggerSkill("huilei"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(killer){
            Room *room = player->getRoom();

            LogMessage log;
            log.type = "#HuileiThrow";
            log.from = player;
            log.to << killer;
            room->sendLog(log);

            killer->throwAllEquips();
            killer->throwAllHandCards();
        }

        return false;
    }
};


class ZhichiClear: public TriggerSkill{
public:
    ZhichiClear():TriggerSkill("#zhichi-clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::NotActive)
            player->getRoom()->setTag("Zhichi", QVariant());

        return false;
    }
};

class Zhichi: public TriggerSkill{
public:
    Zhichi():TriggerSkill("zhichi"){
        events << Damaged << CardEffected;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(player->getPhase() != Player::NotActive)
            return false;

        if(event == Damaged){
            room->setTag("Zhichi", player->objectName());

            LogMessage log;
            log.type = "#ZhichiDamaged";
            log.from = player;
            room->sendLog(log);

        }else if(event == CardEffected){
            if(room->getTag("Zhichi").toString() != player->objectName())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Slash") || effect.card->getTypeId() == Card::Trick){
                LogMessage log;
                log.type = "#ZhichiAvoid";
                log.from = player;
                room->sendLog(log);

                return true;
            }
        }
        return false;
    }
};

class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->card_pattern;
                if(pattern == "slash")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        QString pattern = ClientInstance->card_pattern;
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class Wuyan: public TriggerSkill{
public:
    Wuyan():TriggerSkill("wuyan"){
        events << CardEffect << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.to == effect.from)
            return false;

        if(effect.card->getTypeId() == Card::Trick){
            Room *room = player->getRoom();

            if((effect.from && effect.from->hasSkill(objectName()))){
                LogMessage log;
                log.type = "#WuyanBad";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                return true;
            }

            if(effect.to->hasSkill(objectName()) && effect.from){
                LogMessage log;
                log.type = "#WuyanGood";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                return true;
            }
        }

        return false;
    }
};

class Gawa: public PhaseChangeSkill{
public:
    Gawa():PhaseChangeSkill("gawa"){

    }

    virtual bool onPhaseChange(ServerPlayer *shencc) const{
        if(shencc->getPhase() != Player::Finish)
            return false;

        Room *room = shencc->getRoom();
        if(!room->askForSkillInvoke(shencc, objectName()))
            return false;

        QString choice = room->askForChoice(shencc, objectName(), "modify+obtain");

        if(choice == "modify"){
            ServerPlayer *to_modify = room->askForPlayerChosen(shencc, room->getOtherPlayers(shencc), objectName());
            QString kingdom = room->askForKingdom(shencc);
            QString old_kingdom = to_modify->getKingdom();
            room->setPlayerProperty(to_modify, "kingdom", kingdom);

            LogMessage log;
            log.type = "#ChangeKingdom";
            log.from = shencc;
            log.to << to_modify;
            log.arg = old_kingdom;
            log.arg2 = kingdom;
            room->sendLog(log);

        }else if(choice == "obtain"){
            QStringList lords = Sanguosha->getLords();
            QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
            foreach(ServerPlayer *player, players){
                lords.removeOne(player->getGeneralName());
            }

            QStringList lord_skills;
            foreach(QString lord, lords){
                const General *general = Sanguosha->getGeneral(lord);
                QList<const Skill *> skills = general->findChildren<const Skill *>();
                foreach(const Skill *skill, skills){
                    if(skill->isLordSkill() && !shencc->hasSkill(skill->objectName()))
                        lord_skills << skill->objectName();
                }
            }

            if(!lord_skills.isEmpty()){
                QString skill_name = room->askForChoice(shencc, objectName(), lord_skills.join("+"));

                const Skill *skill = Sanguosha->getSkill(skill_name);
                room->acquireSkill(shencc, skill);

                if(skill->inherits("GameStartSkill")){
                    const GameStartSkill *game_start_skill = qobject_cast<const GameStartSkill *>(skill);
                    game_start_skill->onGameStart(shencc);
                }
            }
        }
//        room->playSkillEffect("guixin");
        return false;
    }
};

class Kuangbao: public TriggerSkill{
public:
    Kuangbao():TriggerSkill("kuangbao"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        LogMessage log;
        log.type = event == Damage ? "#KuangbaoDamage" : "#KuangbaoDamaged";
        log.from = player;
        log.arg = QString::number(damage.damage);
        player->getRoom()->sendLog(log);

        player->gainMark("@wrath", damage.damage);

        return false;
    }
};

class Wumou:public TriggerSkill{
public:
    Wumou():TriggerSkill("wumou"){
        frequency = Compulsory;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard") && !card->inherits("DelayedTrick")){
            Room *room = player->getRoom();
            int num = player->getMark("@wrath");
            if(num >= 1 && room->askForChoice(player, objectName(), "discard+losehp") == "discard"){
                player->loseMark("@wrath");
            }else
                room->loseHp(player);
        }

        return false;
    }
};

class Shenfen:public ZeroCardViewAsSkill{
public:
    Shenfen():ZeroCardViewAsSkill("shenfen"){
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@wrath") >= 6 && !Self->hasUsed("ShenfenCard");
    }

    virtual const Card *viewAs() const{
        return new ShenfenCard;
    }
};


ShenfenCard::ShenfenCard(){
    target_fixed = true;
    once = true;
}

void ShenfenCard::use(Room *room, ServerPlayer *shenlubu, const QList<ServerPlayer *> &) const{
    shenlubu->loseMark("@wrath", 6);

    QList<ServerPlayer *> players = room->getOtherPlayers(shenlubu);
    foreach(ServerPlayer *player, players){
        DamageStruct damage;
        damage.card = this;
        damage.from = shenlubu;
        damage.to = player;

        room->damage(damage);
    }

    foreach(ServerPlayer *player, players){
        player->throwAllEquips();
    }

    foreach(ServerPlayer *player, players){
        if(player->getHandcardNum() <= 4)
            player->throwAllHandCards();
        else
            room->askForDiscard(player, "shenfen", 4);
    }

    shenlubu->turnOver();
}

WuqianCard::WuqianCard(){
    once = true;
}

bool WuqianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *) const{
    return targets.isEmpty();
}

void WuqianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    effect.from->loseMark("@wrath", 2);
    room->acquireSkill(effect.from, "wushuang", false);
    effect.from->setFlags("wuqian_used");

    effect.to->addMark("qinggang");
}

class WuqianViewAsSkill: public ZeroCardViewAsSkill{
public:
    WuqianViewAsSkill():ZeroCardViewAsSkill("wuqian"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@wrath") >= 2 && !Self->hasUsed("WuqianCard");
    }

    virtual const Card *viewAs() const{
        return new WuqianCard;
    }
};

class Wuqian: public PhaseChangeSkill{
public:
    Wuqian():PhaseChangeSkill("wuqian"){
        view_as_skill = new WuqianViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *shenlubu) const{
        if(shenlubu->getPhase() == Player::NotActive){
            Room *room = shenlubu->getRoom();
            if(shenlubu->hasFlag("wuqian_used")){
                shenlubu->setFlags("-wuqian_used");
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    player->removeMark("qinggang");
                }

                const General *general2 = shenlubu->getGeneral2();
                if(general2 == NULL || !general2->hasSkill("wushuang"))
                    shenlubu->loseSkill("wushuang");
            }
        }

        return false;
    }
};

//∞Õ¿Ëπ÷»À
XianzhenCard::XianzhenCard(){
    once = true;
    will_throw = false;
}

bool XianzhenCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select != Self && ! to_select->isKongcheng();
}

void XianzhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    const Card *card = Sanguosha->getCard(subcards.first());
    if(effect.from->pindian(effect.to, "xianzhen", card)){
        PlayerStar target = effect.to;
        effect.from->tag["XianzhenTarget"] = QVariant::fromValue(target);
        room->setPlayerFlag(effect.from, "xianzhen_success");
        room->setFixedDistance(effect.from, effect.to, 1);
    }else{
        room->setPlayerFlag(effect.from, "xianzhen_failed");
    }
}

XianzhenSlashCard::XianzhenSlashCard(){
    target_fixed = true;
}

void XianzhenSlashCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *target = card_use.from->tag["XianzhenTarget"].value<PlayerStar>();
    if(target == NULL)
        return;

    const Card *slash = room->askForCard(card_use.from, "slash", "@xianzhen-slash");
    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.from = card_use.from;
        use.to << target;
        room->useCard(use);
    }
}

class XianzhenViewAsSkill: public ViewAsSkill{
public:
    XianzhenViewAsSkill():ViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("XianzhenCard") || Self->hasFlag("xianzhen_success");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        if(Self->hasUsed("XianzhenCard"))
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(! Self->hasUsed("XianzhenCard")){
            if(cards.length() != 1)
                return NULL;

            XianzhenCard *card = new XianzhenCard;
            card->addSubcards(cards);
            return card;
        }else if(Self->hasFlag("xianzhen_success")){
            if(!cards.isEmpty())
                return NULL;

            return new XianzhenSlashCard;
        }else
            return NULL;
    }
};

class Xianzhen: public TriggerSkill{
public:
    Xianzhen():TriggerSkill("xianzhen"){
        view_as_skill = new XianzhenViewAsSkill;

        events << PhaseChange << SlashEffect << SlashHit << SlashMissed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *gaoshun, QVariant &data) const{
        ServerPlayer *target = gaoshun->tag["XianzhenTarget"].value<PlayerStar>();

        if(event == PhaseChange){
            if(gaoshun->getPhase() == Player::Finish && target){
                Room *room = gaoshun->getRoom();
                room->setFixedDistance(gaoshun, target, -1);
                gaoshun->tag.remove("XianzhenTarget");
            }
        }else{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            if(effect.to == target){
                if(event == SlashEffect)
                    target->addMark("qinggang");
                else
                    target->removeMark("qinggang");
            }
        }

        return false;
    }
};

class FanjiGet: public TriggerSkill{
public:
    FanjiGet():TriggerSkill("#fanji-get"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QVariant card_data = room->getTag("FanjiCard");

        if(!card_data.isValid())
            return false;

        CardStar card = card_data.value<CardStar>();
        room->setTag("FanjiCard", QVariant());

        ServerPlayer *lukang = room->findPlayerBySkillName(objectName());
        if(lukang == NULL)
            return false;

        if(!room->obtainable(card, lukang))
            return false;

        if(lukang->askForSkillInvoke("fanji", data))
            lukang->obtainCard(card);

        return false;
    }
};

class Fanji: public TriggerSkill{
    public:
    Fanji():TriggerSkill("fanji"){
        events << CardEffected;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *lukang, QVariant &data) const{
        Room *room = lukang->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.multiple && effect.card->inherits("TrickCard")
            && !effect.card->inherits("DelayedTrick") && effect.from != lukang){
            room->setTag("FanjiCard", QVariant::fromValue(effect.card));
        }
        return false;
    }
};

Evil2Package::Evil2Package()
    :Package("evil2")
{
    //µ–∂‘ ∆¡¶‘ˆ≤π
    General *aoisatan, *setsuna, *rasetsu, *miroku, *zhutou3, *tenkai, *slasher;
    /*ø˚≤Êµ§    ∏’¡“£¨ø‡»‚£¨‘Ÿ∆*/
         aoisatan = new General(this, "aoisatan", "e",4,true);
         aoisatan->addSkill(new Ganglie);
         aoisatan->addSkill(new Kurou);
         aoisatan->addSkill(new Zaiqi);
//…≤ƒ«
         setsuna = new General(this, "setsuna", "e",4,true);
         setsuna->addSkill(new Skill("wansha", Skill::Compulsory));
         setsuna->addSkill(new Kanpo);
         setsuna->addSkill(new Dushi);
//¬ﬁ…≤
        rasetsu = new General(this, "rasetsu", "e",4,true);
        rasetsu->addSkill(new Shenli);
        rasetsu->addSkill(new Sizhan);
        rasetsu->addSkill(new Huilei);
//√÷¿’
        miroku = new General(this, "miroku", "e");
        miroku->addSkill(new Longdan);
        miroku->addSkill(new Zhichi);
        miroku->addSkill(new ZhichiClear);
//        miroku->addSkill(new Wuyan);
//ÃÏ∫£
        tenkai = new General(this, "tenkai", "e",4,true);
        tenkai->addSkill(new Gawa);
        tenkai->addSkill(new Kuangbao);
        tenkai->addSkill(new MarkAssignSkill("@wrath", 2));
        tenkai->addSkill(new Wumou);
        tenkai->addSkill(new Wuqian);
        tenkai->addSkill(new Shenfen);
//÷Ì¬πµ˚
        zhutou3 = new General(this, "zhutou3", "e",4,true);
        zhutou3->addSkill(new Qixing);
        zhutou3->addSkill(new QixingStart);
        zhutou3->addSkill(new QixingAsk);
        zhutou3->addSkill(new QixingClear);
        zhutou3->addSkill(new Kuangfeng);
        zhutou3->addSkill(new Dawu);
        zhutou3->addSkill(new Xunlei);
//…±≈Æ
        slasher = new General(this, "slasher", "e");
        slasher->addSkill(new Wuyan);

        addMetaObject<KurouCard>();
        addMetaObject<KuangfengCard>();
        addMetaObject<DawuCard>();
        addMetaObject<XunleiCard>();
        addMetaObject<WuqianCard>();
        addMetaObject<ShenfenCard>();
        addMetaObject<XianzhenCard>();
        addMetaObject<XianzhenSlashCard>();
}

ADD_PACKAGE(Evil2)
