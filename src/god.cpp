#include "god.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"

Feiying::Feiying()
    :Skill("feiying", Skill::Compulsory)
{
}

XuanhuoCard::XuanhuoCard(){
    once = true;
    will_throw = false;
}

void XuanhuoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "xuanhuo");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    QList<ServerPlayer *> targets = room->getOtherPlayers(effect.to);
    ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "xuanhuo");
    if(target != effect.from)
        room->moveCardTo(card, target, Player::Hand, false);

    card_id = room->askForCardChosen(effect.from, effect.to, "he", "xuanhuo");
    card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);
    targets = room->getOtherPlayers(effect.to);
    target = room->askForPlayerChosen(effect.from, targets, "xuanhuo");
    if(target != effect.from)
        room->moveCardTo(card, target, Player::Hand, false);
}

class Xuanhuo: public OneCardViewAsSkill{
public:
    Xuanhuo():OneCardViewAsSkill("xuanhuo"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("XuanhuoCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XuanhuoCard *card = new XuanhuoCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Luanwu: public ZeroCardViewAsSkill{
public:
    Luanwu():ZeroCardViewAsSkill("luanwu"){
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@chaos") >= 1;
    }
};

LuanwuCard::LuanwuCard(){
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@chaos");
    room->broadcastInvoke("animate", "lightbox:$luanwu");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
            room->cardEffect(this, source, player);
    }
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach(ServerPlayer *player, players){
        int distance = effect.to->distanceTo(player);
        distance_list << distance;

        nearest = qMin(nearest, distance);
    }    

    QList<ServerPlayer *> luanwu_targets;
    int i;
    for(i=0; i<distance_list.length(); i++){
        if(distance_list.at(i) == nearest && effect.to->canSlash(players.at(i))){
            luanwu_targets << players.at(i);
        }
    }

    const Card *slash = NULL;    
    if(!luanwu_targets.isEmpty() && (slash = room->askForCard(effect.to, "slash", "@luanwu-slash"))){
        ServerPlayer *to_slash;
        if(luanwu_targets.length() == 1)
            to_slash = luanwu_targets.first();
        else
            to_slash = room->askForPlayerChosen(effect.to, luanwu_targets, "luanwu");
        room->cardEffect(slash, effect.to, to_slash);
    }else
        room->loseHp(effect.to);
}

class WulingExEffect: public TriggerSkill{
public:
    WulingExEffect():TriggerSkill("#wuling-ex-effect"){
        events << CardEffected << Predamaged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuandi = room->findPlayerBySkillName(objectName());
        if(xuandi == NULL)
            return false;

        QString wuling = xuandi->tag.value("wuling").toString();
        if(event == CardEffected && wuling == "water"){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card && effect.card->inherits("Peach")){
                RecoverStruct recover;
                recover.card = effect.card;
                recover.who = effect.from;
                room->recover(player, recover);

                LogMessage log;
                log.type = "#WulingWater";
                log.from = player;
                room->sendLog(log);
            }
        }else if(event == Predamaged && wuling == "earth"){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal && damage.damage > 1){
                damage.damage = 1;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingEarth";
                log.from = player;
                room->sendLog(log);
            }
        }

        return false;
    }
};

class WulingEffect: public TriggerSkill{
public:
    WulingEffect():TriggerSkill("#wuling-effect"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuandi = room->findPlayerBySkillName(objectName());
        if(xuandi == NULL)
            return false;

        QString wuling = xuandi->tag.value("wuling").toString();
        DamageStruct damage = data.value<DamageStruct>();

        if(wuling == "wind"){
            if(damage.nature == DamageStruct::Fire && !damage.chain){
                damage.damage ++;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingWind";
                log.from = damage.to;
                log.arg = QString::number(damage.damage - 1);
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
            }
        }else if(wuling == "thunder"){
            if(damage.nature == DamageStruct::Thunder && !damage.chain){
                damage.damage ++;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingThunder";
                log.from = damage.to;
                log.arg = QString::number(damage.damage - 1);
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
            }
        }else if(wuling == "fire"){
            if(damage.nature != DamageStruct::Fire){
                damage.nature = DamageStruct::Fire;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingFire";
                log.from = damage.to;
                room->sendLog(log);
            }
        }

        return false;
    }
};

class Wuling: public PhaseChangeSkill{
public:
    Wuling():PhaseChangeSkill("wuling"){
        default_choice = "wind";
    }

    virtual bool onPhaseChange(ServerPlayer *xuandi) const{
        static QStringList effects;
        if(effects.isEmpty()){
            effects << "wind" << "thunder" << "water" << "fire" << "earth";
        }

        if(xuandi->getPhase() == Player::Start){
            QString current = xuandi->tag.value("wuling").toString();
            QStringList choices;
            foreach(QString effect, effects){
                if(effect != current)
                    choices << effect;
            }

            Room *room = xuandi->getRoom();
            QString choice = room->askForChoice(xuandi, objectName(), choices.join("+"));
            if(!current.isEmpty())
                xuandi->loseMark("@" + current);

            xuandi->gainMark("@" + choice);
            xuandi->tag["wuling"] = choice;

            room->playSkillEffect(objectName(), effects.indexOf(choice) + 1);
        }

        return false;
    }
};

GuihanCard::GuihanCard(){
    once = true;
}

void GuihanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caizhaoji = effect.from;
    caizhaoji->getRoom()->swapSeat(caizhaoji, effect.to);
}

class Guihan: public ViewAsSkill{
public:
    Guihan():ViewAsSkill("guihan"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("GuihanCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        if(selected.isEmpty())
            return to_select->getFilteredCard()->isRed();
        else if(selected.length() == 1){
            Card::Suit suit = selected.first()->getFilteredCard()->getSuit();
            return to_select->getFilteredCard()->getSuit() == suit;
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        GuihanCard *card = new GuihanCard;
        card->addSubcards(cards);
        return card;
    }
};

class SavageAssaultAvoid: public TriggerSkill{
public:
    SavageAssaultAvoid(const QString &avoid_skill)
        :TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill)
    {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("SavageAssault") || effect.card->inherits("ArcheryAttack")){
            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = avoid_skill;
            log.arg2 = "savage_assault";
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }

private:
    QString avoid_skill;
};

class Juxiang: public TriggerSkill{
public:
    Juxiang():TriggerSkill("juxiang"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(player->hasSkill(objectName()))
            return false;
        if(use.card->inherits("SavageAssault") || use.card->inherits("ArcheryAttack")){
            Room *room = player->getRoom();
            if(room->getCardPlace(use.card->getId()) == Player::DiscardedPile){
                // finding zhurong;
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *p, players){
                    if(p->hasSkill(objectName())){
                        p->obtainCard(use.card);
                        room->playSkillEffect(objectName());
                        break;
                    }
                }
            }
        }

        return false;
    }
};

class Xingshang: public TriggerSkill{
public:
    Xingshang():TriggerSkill("xingshang"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        if(player->isNude())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *caopi = room->findPlayerBySkillName(objectName());
        if(caopi && caopi->isAlive() && room->askForSkillInvoke(caopi, objectName())){
            QString general_name = player->getGeneralName();
            if(general_name == "caocao" || general_name == "shencaocao" || general_name == "shencc"){
                room->playSkillEffect(objectName(), 3);
            }else if(player->getGeneral()->isMale())
                room->playSkillEffect(objectName(), 1);
            else
                room->playSkillEffect(objectName(), 2);

            caopi->obtainCard(player->getWeapon());
            caopi->obtainCard(player->getArmor());
            caopi->obtainCard(player->getDefensiveHorse());
            caopi->obtainCard(player->getOffensiveHorse());

            DummyCard *all_cards = player->wholeHandCards();
            if(all_cards){
                room->moveCardTo(all_cards, caopi, Player::Hand, false);
                delete all_cards;
            }
        }

        return false;
    }
};

class Pojun: public TriggerSkill{
public:
    Pojun():TriggerSkill("pojun"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isDead())
            return false;

        if(damage.card && damage.card->inherits("Slash") &&
           player->askForSkillInvoke(objectName(), data))
        {
            int x = qMin(5, damage.to->getHp());
            damage.to->drawCards(x);
            damage.to->turnOver();
        }

        return false;
    }
};

class Wuhun: public TriggerSkill{
public:
    Wuhun():TriggerSkill("wuhun"){
        events << DamageDone;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from && damage.from != player)
            damage.from->gainMark("@nightmare", damage.damage);

        return false;
    }
};

class WuhunRevenge: public TriggerSkill{
public:
    WuhunRevenge():TriggerSkill("#wuhun"){
        events << Death;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("wuhun");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *shenguanyu, QVariant &) const{
        Room *room = shenguanyu->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(shenguanyu);

        int max = 0;
        foreach(ServerPlayer *player, players){
            max = qMax(max, player->getMark("@nightmare"));
        }

        if(max == 0)
            return false;

        QList<ServerPlayer *> foes;
        foreach(ServerPlayer *player, players){
            if(player->getMark("@nightmare") == max)
                foes << player;
        }

        if(foes.isEmpty())
            return false;

        ServerPlayer *foe;
        if(foes.length() == 1)
            foe = foes.first();
        else
            foe = room->askForPlayerChosen(shenguanyu, foes, "wuhun");

        JudgeStruct judge;
        judge.pattern = QRegExp("(Peach|GodSalvation):(.*):(.*)");
        judge.good = true;
        judge.reason = "wuhun";
        judge.who = foe;

        room->judge(judge);

        if(judge.isBad()){
            LogMessage log;
            log.type = "#WuhunRevenge";
            log.from = shenguanyu;
            log.to << foe;
            log.arg = QString::number(max);
            room->sendLog(log);

            room->killPlayer(foe);
        }

        return false;
    }
};

class Weidi: public GameStartSkill{
public:
    Weidi():GameStartSkill("weidi"){
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *lord = room->getLord();
        if(lord != player){
            const General *general = lord->getGeneral();
            QList<const Skill *> skills = general->findChildren<const Skill *>();
            foreach(const Skill *skill, skills){
                if(skill->isLordSkill()){
                    room->acquireSkill(player, skill->objectName());
                    return;
                }
            }
        }
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.multiple && effect.card->inherits("TrickCard")){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), data)){
                room->playSkillEffect(objectName());

                LogMessage log;

                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                player->drawCards(1);
                return true;
            }
        }

        return false;
    }
};

JujianCard::JujianCard(){
    once = true;
}

void JujianCard::onEffect(const CardEffectStruct &effect) const{
    int n = subcardsLength();
    effect.to->drawCards(n);

    if(n == 3){
        QSet<Card::CardType> types;

        foreach(int card_id, effect.card->getSubcards()){
            const Card *card = Sanguosha->getCard(card_id);
            types << card->getTypeId();
        }

        if(types.size() == 1){
            Room *room = effect.from->getRoom();

            LogMessage log;
            log.type = "#JujianRecover";
            log.from = effect.from;
            const Card *card = Sanguosha->getCard(subcards.first());
            log.arg = card->getType();
            room->sendLog(log);

            RecoverStruct recover;
            recover.card = this;
            recover.who = effect.from;
            room->recover(effect.from, recover);
        }
    }
}

class Jujian: public ViewAsSkill{
public:
    Jujian():ViewAsSkill("jujian"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3;
    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("JujianCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        JujianCard *card = new JujianCard;
        card->addSubcards(cards);
        return card;
    }
};

class Gongmou: public PhaseChangeSkill{
public:
    Gongmou():PhaseChangeSkill("gongmou"){

    }

    virtual bool onPhaseChange(ServerPlayer *zhongshiji) const{
        switch(zhongshiji->getPhase()){
        case Player::Finish:{
                if(zhongshiji->askForSkillInvoke(objectName())){
                    Room *room = zhongshiji->getRoom();
                    QList<ServerPlayer *> players = room->getOtherPlayers(zhongshiji);
                    ServerPlayer *target = room->askForPlayerChosen(zhongshiji, players, "gongmou");
                    target->gainMark("@conspiracy");
                }
                break;
            }

        case Player::Start:{
                Room *room = zhongshiji->getRoom();
                QList<ServerPlayer *> players = room->getOtherPlayers(zhongshiji);
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@conspiracy") > 0)
                        player->loseMark("@conspiracy");
                }
            }

        default:
            break;
        }


        return false;
    }
};

class GongmouExchange:public PhaseChangeSkill{
public:
    GongmouExchange():PhaseChangeSkill("#gongmou-exchange"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@conspiracy") > 0;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Draw)
            return false;

        player->loseMark("@conspiracy");

        Room *room = player->getRoom();
        ServerPlayer *zhongshiji = room->findPlayerBySkillName("gongmou");
        if(zhongshiji){
            int x = qMin(zhongshiji->getHandcardNum(), player->getHandcardNum());
            if(x == 0)
                return false;

            const Card *to_exchange = NULL;
            if(player->getHandcardNum() == x)
                to_exchange = player->wholeHandCards();
            else
                to_exchange = room->askForExchange(player, "gongmou", x);

            room->moveCardTo(to_exchange, zhongshiji, Player::Hand, false);

            delete to_exchange;

            to_exchange = room->askForExchange(zhongshiji, "gongmou", x);
            room->moveCardTo(to_exchange, player, Player::Hand, false);

            delete to_exchange;

            LogMessage log;
            log.type = "#GongmouExchange";
            log.from = zhongshiji;
            log.to << player;
            log.arg = QString::number(x);
            room->sendLog(log);
        }

        return false;
    }
};

JieyinCard::JieyinCard(){
    once = true;
}

bool JieyinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    return (to_select->isWounded() && to_select->getKingdom()!="e");
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

class Jieyin: public ViewAsSkill{
public:
    Jieyin():ViewAsSkill("jieyin"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("JieyinCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        JieyinCard *jieyin_card = new JieyinCard();
        jieyin_card->addSubcards(cards);

        return jieyin_card;
    }
};

class Buyi: public TriggerSkill{
public:
    Buyi():TriggerSkill("buyi"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return !player->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *wuguotai = room->findPlayerBySkillName(objectName());

        if(wuguotai && wuguotai->askForSkillInvoke(objectName(), data)){
            const Card *card = NULL;
            if(player == wuguotai)
                card = room->askForCardShow(player, wuguotai, objectName());
            else
                card = player->getRandomHandCard();

            room->showCard(player, card->getEffectiveId());

            if(card->getTypeId() != Card::Basic){
                room->throwCard(card);

                RecoverStruct recover;
                recover.who = wuguotai;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){

                    player->setFlags("-jilei");
                    player->setFlags("-jileiB");
                    player->setFlags("-jileiE");
                    player->setFlags("-jileiT");

                    player->invoke("jilei");
                }
            }
        }

        return false;
    }
};


class Jilei: public TriggerSkill{
public:
    Jilei():TriggerSkill("jilei"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        Room *room = yangxiu->getRoom();
        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->playSkillEffect(objectName());

            QString jilei_flag = choice[0].toUpper();
            damage.from->invoke("jilei", jilei_flag);

            damage.from->setFlags("jilei");
            damage.from->setFlags("jilei" + jilei_flag);

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
        }

        return false;
    }
};

ChengxiangCard::ChengxiangCard()
{

}

bool ChengxiangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < subcardsLength() && to_select->isWounded();
}

bool ChengxiangCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() <= subcardsLength();
}

void ChengxiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> to = targets;

    if(to.isEmpty())
        to << source;

    return SkillCard::use(room, source, to);
}

void ChengxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

class ChengxiangViewAsSkill: public ViewAsSkill{
public:
    ChengxiangViewAsSkill():ViewAsSkill("chengxiang"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@chengxiang";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;

        int sum = 0;
        foreach(CardItem *item, selected){
            sum += item->getCard()->getNumber();
        }

        sum += to_select->getCard()->getNumber();

        return sum <= Self->getMark("chengxiang");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int sum = 0;
        foreach(CardItem *item, cards){
            sum += item->getCard()->getNumber();
        }

        if(sum == Self->getMark("chengxiang")){
            ChengxiangCard *card = new ChengxiangCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Chengxiang: public MasochismSkill{
public:
    Chengxiang():MasochismSkill("chengxiang"){
        view_as_skill = new ChengxiangViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caochong, const DamageStruct &damage) const{
        const Card *card = damage.card;
        if(card == NULL)
            return;

        int point = card->getNumber();
        if(point == 0)
            return;

        if(caochong->isNude())
            return;

        Room *room = caochong->getRoom();
        room->setPlayerMark(caochong, objectName(), point);

        QString prompt = QString("@chengxiang-card:::%1").arg(point);
        room->askForUseCard(caochong, "@@chengxiang", prompt);
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(3);
                target->turnOver();

                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

class Yongsi: public TriggerSkill{
public:
    Yongsi():TriggerSkill("yongsi"){
        events << DrawNCards << PhaseChange;
        frequency = Compulsory;
    }

    int getKingdoms(ServerPlayer *yuanshu) const{
        QSet<QString> kingdom_set;
        Room *room = yuanshu->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuanshu, QVariant &data) const{
        if(event == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            room->sendLog(log);

            room->playSkillEffect("yongsi", x);

        }else if(event == PhaseChange && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            int total = yuanshu->getEquips().length() + yuanshu->getHandcardNum();
            Room *room = yuanshu->getRoom();

            if(total <= x){
                yuanshu->throwAllHandCards();
                yuanshu->throwAllEquips();

                LogMessage log;
                log.type = "#YongsiWorst";
                log.from = yuanshu;
                log.arg = QString::number(total);
                room->sendLog(log);

            }else{
                room->askForDiscard(yuanshu, "yongsi", x, false, true);

                LogMessage log;
                log.type = "#YongsiBad";
                log.from = yuanshu;
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }

        return false;
    }
};

HuangtianCard::HuangtianCard(){
    once = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasSkill("huangtian")){
        zhangjiao->obtainCard(this);
        room->setEmotion(zhangjiao, "good");
    }
}

bool HuangtianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian") && to_select != Self;
}

class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){

    }

    virtual bool isEnabledAtPlay() const{
        return !Self->hasUsed("HuangtianCard") && Self->getKingdom() == "di";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "jink" || card->inherits("DelayedTrick");
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

class Baonue: public TriggerSkill{
public:
    Baonue():TriggerSkill("baonue$"){
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "ba";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> dongzhuos;
        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *p, players){
            if(p->hasLordSkill("baonue")){
                dongzhuos << p;
            }
        }

        foreach(ServerPlayer *dongzhuo, dongzhuos){
            QVariant who = QVariant::fromValue(dongzhuo);
            if(dongzhuo->isWounded() && player->askForSkillInvoke(objectName(), who)){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = true;
                judge.reason = "baonue";
                judge.who = player;

                room->judge(judge);

                if(judge.isGood()){
                    room->playSkillEffect(objectName());

                    RecoverStruct recover;
                    recover.who = player;
                    room->recover(dongzhuo, recover);
                }
            }
        }

        return false;
    }
};

GodpPackage::GodpPackage()
    :Package("godp")
{
	//Éñ
    General *yoneda, *ayame, *kaede, *kazuma, *yamazaki, *granma;

    yoneda = new General(this, "yoneda$", "god",4, true); //Ã×ÌïÒ»»ù
    yoneda->addSkill(new Feiying);//·ÉÓ°
	yoneda->addSkill(new Xuanhuo);//Ñ£»ó
        yoneda->addSkill(new MarkAssignSkill("@chaos", 2));
        yoneda->addSkill(new Luanwu);//ÂÒÎä
        yoneda->addSkill(new Huangtian);

    kazuma = new General(this, "kazuma", "god",5,  true); //Õæ¹¬ËÂÒ»Âí
    kazuma->addSkill(new WulingEffect);
    kazuma->addSkill(new WulingExEffect);
        kazuma->addSkill(new Wuling); //ÎåÁé¡¢¹éºº¡¢¾ÞÏó
    kazuma->addSkill(new Guihan);
	kazuma->addSkill(new SavageAssaultAvoid("juxiang"));
    kazuma->addSkill(new Juxiang);

        yamazaki= new General(this, "yamazaki", "god",4,  true); //É½ÆéÕæÖ®½é
    yamazaki->addSkill(new Xingshang); //ÐÐéä£¬ÆÆ¾ü£¬Îä»ê£¬Î±µÛ
	yamazaki->addSkill(new Pojun);
	yamazaki->addSkill(new Wuhun);
    yamazaki->addSkill(new WuhunRevenge);
    yamazaki->addSkill(new Weidi);

    ayame = new General(this, "ayame", "god"); //ÌÙÖ¦ÝÅÆÑ
    ayame->addSkill(new Danlao); //à¢ÀÒ¡¢¾Ù¼ö¡¢¹²Ä±
    ayame->addSkill(new Jujian);
    ayame->addSkill(new Gongmou);
	ayame->addSkill(new GongmouExchange);

    kaede = new General(this, "kaede", "god"); //ÌÙÖ¦·ã
    kaede->addSkill(new Jieyin); //½áÒö¡¢²¹Òæ¡¢¼¦Àß
    kaede->addSkill(new Buyi);
    kaede->addSkill(new Jilei);
    kaede->addSkill(new JileiClear);

        granma = new General(this, "granma$", "god"); //¸ñÀ¼Âê
    granma->addSkill(new Chengxiang); //³ÆÏó¡¢¾ÝÊØ¡¢Ó¹ËÁ
    granma->addSkill(new Jushou);
    granma->addSkill(new Yongsi);
    granma->addSkill(new Baonue);//Ö÷¹«¼¼±©Å°

    addMetaObject<XuanhuoCard>();
    addMetaObject<LuanwuCard>();
    addMetaObject<GuihanCard>();
    addMetaObject<JujianCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<ChengxiangCard>();
    addMetaObject<HuangtianCard>();

    skills << new HuangtianViewAsSkill;
}

ADD_PACKAGE(Godp)
