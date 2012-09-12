#include "maneuvering.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"

NatureSlash::NatureSlash(Suit suit, int number, DamageStruct::Nature nature)
    :Slash(suit, number)
{
    this->nature = nature;
}

bool NatureSlash::match(const QString &pattern) const{
    if(pattern == "slash")
        return true;
    else
        return Slash::match(pattern);
}

ThunderSlash::ThunderSlash(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Thunder)
{
    setObjectName("thunder_slash");
}

FireSlash::FireSlash(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Fire)
{
    setObjectName("fire_slash");
    nature = DamageStruct::Fire;
}

Analeptic::Analeptic(Card::Suit suit, int number)
    :BasicCard(suit, number)
{
    setObjectName("analeptic");
    target_fixed = true;
    once = true;
}

QString Analeptic::getSubtype() const{
    return "buff_card";
}

QString Analeptic::getEffectPath(bool ) const{
    return QString("audio/card/common/%1.ogg").arg("analeptic");
}

bool Analeptic::IsAvailable(const Player *player){
    return !player->hasUsed("Analeptic");
}

bool Analeptic::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

void Analeptic::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(targets.isEmpty())
        room->cardEffect(this, source, source);
    else
        foreach(ServerPlayer *tmp, targets)
            room->cardEffect(this, source, tmp);
}

void Analeptic::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    QString who = effect.to->objectName();
    QString animation_str = QString("analeptic:%1:%2").arg(who).arg(who);
    room->broadcastInvoke("animate", animation_str);

    if(effect.to->hasFlag("dying")){
        // recover hp
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }else{
        LogMessage log;
        log.type = "#Drank";
        log.from = effect.to;
        log.arg = objectName();
        room->sendLog(log);

        room->setPlayerFlag(effect.to, "drank");
    }
}

JuicePeach::JuicePeach(Suit suit, int number):Peach(suit, number){
    setObjectName("juice_peach");
    target_fixed = false;
}

bool JuicePeach::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool JuicePeach::isAvailable(const Player *player) const{
    return true;
}

class FanSkill: public WeaponSkill{
public:
    FanSkill():WeaponSkill("fan"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.nature == DamageStruct::Normal){
            if(player->getRoom()->askForSkillInvoke(player, objectName(), data)){
                effect.nature = DamageStruct::Fire;

                data = QVariant::fromValue(effect);
            }
        }

        return false;
    }
};

Fan::Fan(Suit suit, int number):Weapon(suit, number, 4){
    setObjectName("fan");
    skill = new FanSkill;
}

class GudingBladeSkill: public WeaponSkill{
public:
    GudingBladeSkill():WeaponSkill("guding_blade"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
            damage.to->isKongcheng())
        {
            Room *room = damage.to->getRoom();

            LogMessage log;
            log.type = "#GudingBladeEffect";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

GudingBlade::GudingBlade(Suit suit, int number):Weapon(suit, number, 2){
    setObjectName("guding_blade");
    skill = new GudingBladeSkill;
}

class VineSkill: public ArmorSkill{
public:
    VineSkill():ArmorSkill("vine"){
        events << Predamaged << SlashEffected << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.nature == DamageStruct::Normal){
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                player->getRoom()->sendLog(log);

                return true;
            }
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("AOE")){
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.card->objectName();
                player->getRoom()->sendLog(log);

                return true;
            }
        }else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature == DamageStruct::Fire){
                LogMessage log;
                log.type = "#VineDamage";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                player->getRoom()->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

Vine::Vine(Suit suit, int number):Armor(suit, number){
    setObjectName("vine");
    skill = new VineSkill;
}

class SilverLionSkill: public ArmorSkill{
public:
    SilverLionSkill():ArmorSkill("silver_lion"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#SilverLion";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

SilverLion::SilverLion(Suit suit, int number):Armor(suit, number){
    setObjectName("silver_lion");
    skill = new SilverLionSkill;
}

void SilverLion::onUninstall(ServerPlayer *player) const{
    if(player->isAlive() && player->getMark("qinggang") == 0){
        RecoverStruct recover;
        recover.card = this;
        player->getRoom()->recover(player, recover);
    }
}

FireAttack::FireAttack(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("fire_attack");
}

bool FireAttack::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return Self->getHandcardNum() >= 2;
    else
        return true;
}

void FireAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.to->isKongcheng())
        return;

    const Card *card = room->askForCardShow(effect.to, effect.from, objectName());
    room->showCard(effect.to, card->getEffectiveId());

    QString suit_str = card->getSuitString();
    QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
    QString prompt = QString("@fire-attack:%1::%2").arg(effect.to->getGeneralName()).arg(suit_str);
    if(room->askForCard(effect.from, pattern, prompt)){
        DamageStruct damage;
        damage.card = this;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Fire;

        room->damage(damage);
    }

    if(card->isVirtualCard())
        delete card;
}

IronChain::IronChain(Card::Suit suit, int number)
    :TrickCard(suit, number, false)
{
    setObjectName("iron_chain");
}

QString IronChain::getSubtype() const{
    return "damage_spread";
}

bool IronChain::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    return true;
}

bool IronChain::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 2;
}

void IronChain::onUse(Room *room, const CardUseStruct &card_use) const{
    if(card_use.to.isEmpty()){
        room->throwCard(this);
        card_use.from->playCardEffect(objectName());
        card_use.from->drawCards(1);
    }else
        TrickCard::onUse(room, card_use);
}

void IronChain::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    TrickCard::use(room, source, targets);
}

void IronChain::onEffect(const CardEffectStruct &effect) const{
    bool chained = ! effect.to->isChained();
    effect.to->setChained(chained);

    effect.to->getRoom()->broadcastProperty(effect.to, "chained");
}

SupplyShortage::SupplyShortage(Card::Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("supply_shortage");

    judge.pattern = QRegExp("(.*):(club):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool SupplyShortage::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    if(Self->hasSkill("qicai") || Self->hasSkill("chuanyao"))
        return true;

    int distance = Self->distanceTo(to_select);
    if(Self->hasSkill("duanliang"))
        return distance <= 2;
    else
        return distance <= 1;
}

void SupplyShortage::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Draw);
}

BlackDragonPackage::BlackDragonPackage()
    :Package("black_dragon")
{
    QList<Card *> cards;

    // spade
    cards
            << new GudingBlade(Card::Spade, 1)
            << new Vine(Card::Spade, 2);
    Card *ana = new Analeptic(Card::Spade, 3);
    ana->setObjectName("ana_gin");
    cards
            << ana
            << new ThunderSlash(Card::Spade, 4)
            << new ThunderSlash(Card::Spade, 5)
            << new ThunderSlash(Card::Spade, 6)
            << new ThunderSlash(Card::Spade, 7)
            << new ThunderSlash(Card::Spade, 8);
    ana = new Analeptic(Card::Spade, 9);
    ana->setObjectName("ana_vodka");
    cards
            << ana
            << new SupplyShortage(Card::Spade,10)
            << new IronChain(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new Nullification(Card::Spade, 13);

    // club
    cards
            << new SilverLion(Card::Club, 1)
            << new Vine(Card::Club, 2);
    ana = new Analeptic(Card::Club, 3);
    ana->setObjectName("ana_vermouth");
    cards
            << ana
            << new SupplyShortage(Card::Club, 4)
            << new ThunderSlash(Card::Club, 5)
            << new ThunderSlash(Card::Club, 6)
            << new ThunderSlash(Card::Club, 7)
            << new ThunderSlash(Card::Club, 8);
    ana = new Analeptic(Card::Club, 9);
    ana->setObjectName("ana_sherry");
    cards
            << ana
            << new IronChain(Card::Club, 10)
            << new IronChain(Card::Club, 11)
            << new IronChain(Card::Club, 12)
            << new IronChain(Card::Club, 13);

    // heart
    cards
            << new Nullification(Card::Heart, 1)
            << new FireAttack(Card::Heart, 2)
            << new FireAttack(Card::Heart, 3)
            << new FireSlash(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Peach(Card::Heart, 6)
            << new FireSlash(Card::Heart, 7)
            << new Jink(Card::Heart, 8)
            << new Jink(Card::Heart, 9)
            << new FireSlash(Card::Heart, 10)
            << new Jink(Card::Heart, 11)
            << new Jink(Card::Heart, 12)
            << new Nullification(Card::Heart, 13);

    // diamond
    cards
            << new Fan(Card::Diamond, 1)
            << new JuicePeach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new FireSlash(Card::Diamond, 5)
            << new Jink(Card::Diamond, 6)
            << new Jink(Card::Diamond, 7)
            << new Jink(Card::Diamond, 8);
    ana = new Analeptic(Card::Diamond, 9);
    ana->setObjectName("ana_chianti");
    cards
            << ana
            << new Jink(Card::Diamond, 10)
            << new Jink(Card::Diamond, 11)
            << new FireAttack(Card::Diamond, 12);
    ana = new DefensiveCar(Card::Diamond, 13);
    ana->setObjectName("citroBX");
    cards << ana;

    foreach(Card *card, cards)
        card->setParent(this);
    skills << new Skill("citroBX");

    type = CardPack;
}

WindJink::WindJink(Suit suit, int number)
    :Jink(suit, number)
{
    setObjectName("wind_jink");
}

SoilJink::SoilJink(Suit suit, int number)
    :Jink(suit, number)
{
    setObjectName("soil_jink");
}

class BowSkill: public ArmorSkill{
public:
    BowSkill():ArmorSkill("bow"){
        events << HpChanged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        int x = (qrand() % 3) + 1;
        if(player->isAlive() && player->askForSkillInvoke(objectName())){
            player->drawCards(x);
        }
        return false;
    }
};

Bow::Bow(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("bow");
    skill = new BowSkill;
}

class RenwangShieldSkill: public ArmorSkill{
public:
    RenwangShieldSkill():ArmorSkill("renwang_shield"){
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isBlack()){
            LogMessage log;
            log.type = "#ArmorNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }
};

RenwangShield::RenwangShield(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("renwang_shield");
    skill = new RenwangShieldSkill;
}

class YxSwordSkill: public WeaponSkill{
public:
    YxSwordSkill():WeaponSkill("yx_sword"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.card && damage.card->inherits("Slash") && room->askForSkillInvoke(player, objectName(), data)){
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            QMutableListIterator<ServerPlayer *> itor(players);

            while(itor.hasNext()){
                itor.next();
                if(!player->inMyAttackRange(itor.value()))
                    itor.remove();
            }

            if(players.isEmpty())
                return false;

            QVariant victim = QVariant::fromValue(damage.to);
            room->setTag("YxSwordVictim", victim);
            ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());
            room->removeTag("YxSwordVictim");
            damage.from = target;
            data = QVariant::fromValue(damage);
        }
        return damage.to->isDead();
    }
};

YxSword::YxSword(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("yx_sword");
    skill = new YxSwordSkill;
}

class BatSkill: public WeaponSkill{
public:
    BatSkill():WeaponSkill("bat"){
        events << SlashMissed;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(player->getRoom()->obtainable(effect.jink, player))
            player->obtainCard(effect.jink);
        return false;
    }
};

Bat::Bat(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("bat");
    skill = new BatSkill;
}

class RailgunSkill: public WeaponSkill{
public:
    RailgunSkill():WeaponSkill("railgun"){
        events << SlashEffect << SlashHit;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashHit){
            Room *room = player->getRoom();
            if(effect.to->getWeapon())
                room->throwCard(effect.to->getWeapon());
            if(effect.to->getArmor())
                room->throwCard(effect.to->getArmor());
            return false;
        }
        if(effect.nature == DamageStruct::Normal){
            effect.nature = DamageStruct::Thunder;
            data = QVariant::fromValue(effect);
        }

        return false;
    }
};

Railgun::Railgun(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("railgun");
    skill = new RailgunSkill;
}

Inspiration::Inspiration(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("inspiration");
}

void Inspiration::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->getThread()->delay(500);
    effect.to->drawCards(qMax(1, effect.to->getLostHp()));
}

Sacrifice::Sacrifice(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("sacrifice");
}

bool Sacrifice::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(!to_select->isWounded())
        return false;

    return true;
}

void Sacrifice::onEffect(const CardEffectStruct &effect) const{
    if(!effect.to->isWounded())
        return;

    Room *room = effect.to->getRoom();
    room->loseHp(effect.from);

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover, true);
}

Potential::Potential(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("potential");
    target_fixed = true;
}

bool Potential::IsAvailable(const Player *player){
    return !player->hasUsed("Potential");
}

bool Potential::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

void Potential::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->setPlayerFlag(effect.to, "NewTurn");
}

RedAlert::RedAlert(Suit suit, int number)
    :AOE(suit, number){
    setObjectName("red_alert");
}

void RedAlert::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *trick = room->askForCard(effect.to, "TrickCard", "redalert-trick:" + effect.from->getGeneralName());
    if(!trick)
        room->loseHp(effect.to);
}

Turnover::Turnover(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("turnover");
}

bool Turnover::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(Self->hasSkill("chuanyao"))
        return true;
    return Self->inMyAttackRange(to_select);
}

void Turnover::onEffect(const CardEffectStruct &effect) const{
    if(!effect.to->getRoom()->askForCard(effect.to, "slash", "turnover-slash:" + effect.from->getGeneralName()))
        effect.to->turnOver();
}

Locust::Locust(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("locust");

    judge.pattern = QRegExp("(.*):(.*):([JQ])");
    judge.good = true;
    judge.reason = objectName();
}

void Locust::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->moveCardTo(this, source->getNextAlive(), Player::Judging);
}

void Locust::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    room->broadcastInvoke("playAudio", "locust");
    if(target->isKongcheng())
        room->loseHp(target);
    else
        room->askForDiscard(target,objectName(), 1);
    onNullified(target);
//    room->moveCardTo(this, target->getNextAlive(), Player::Judging);
}

void Locust::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    LogMessage log;
    log.from = effect.to;
    log.type = "#DelayedTrick";
    log.arg = effect.card->objectName();
    room->sendLog(log);

    JudgeStruct judge_struct = judge;
    judge_struct.who = effect.to;
    room->judge(judge_struct);

    if(judge_struct.isBad()){
        takeEffect(effect.to);
    }else {
        if(room->askForChoice(effect.to,objectName(),"move+throw")=="throw")
            room->throwCard(this);
        else
//        room->moveCardTo(this, effect.to->getNextAlive(), Player::Judging);
        onNullified(effect.to);
    }
}

Wolf::Wolf(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("wolf");
    target_fixed = true;
}

void Wolf::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    int count = 0;
    foreach(ServerPlayer *tmp, room->getOtherPlayers(effect.to)){
        if(tmp->isAllNude())
            continue;
        QList<const Card *> cards = tmp->getCards("hej");
        int index = qrand() % cards.length();
        effect.to->obtainCard(cards.at(index));
        count ++;
        if(count == 4)
            room->loseHp(effect.to);
    }
}

class TanteiSkill: public ArmorSkill{
public:
    TanteiSkill():ArmorSkill("tantei"){
        events << CardAsked << Damaged << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(player == damage.to && damage.to->getArmor() && damage.to->getArmor()->objectName() == objectName()){
                foreach(ServerPlayer *tmp, room->getAllPlayers())
                    if(tmp->getArmor() && tmp->getArmor()->objectName() == objectName())
                        tmp->drawCards(1);
            }
            return false;
        }
        if(event == CardFinished)
            room->removeTag("Tantei");
        if(!player->getArmor() || player->getArmor()->objectName() != objectName())
            return false;

        const QString pattern = data.toString();
        if(pattern != "slash" && pattern != "jink")
            return false;

        if(room->getTag("Tantei").toBool())
            return false;
        QList<ServerPlayer *> friends;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(tmp->getArmor() && tmp->getArmor()->objectName() == objectName())
                friends << tmp;
        }
        if(!friends.isEmpty() && room->askForSkillInvoke(player, objectName())){
            room->setTag("Tantei", true);
            QVariant tohelp = QVariant::fromValue((PlayerStar)player);
            foreach(ServerPlayer *fri, friends){
                const Card *slash = room->askForCard(fri, pattern, "@tantei:" + player->objectName() + ":" + pattern, tohelp);
                if(slash){
                    room->provide(slash);
                    room->removeTag("Tantei");
                    return true;
                }
            }
        }
        return false;
    }
};

Tantei::Tantei(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("tantei");
    skill = new TanteiSkill;
}

class InjectorSkill: public ZeroCardViewAsSkill{
public:
    InjectorSkill():ZeroCardViewAsSkill("injector"){
    }

    virtual const Card *viewAs() const{
        Duel *d = new Duel(Card::NoSuit, 0);
        d->setSkillName(objectName());

        return d;
    }
};

Injector::Injector(Suit suit, int number)
    :Weapon(suit, number, 1)
{
    setObjectName("injector");
    attach_skill = true;
}

ThunderBirdPackage::ThunderBirdPackage()
    :Package("thunder_bird")
{
    QList<Card *> cards;

    // spade
    cards
            << new YxSword(Card::Spade, 1)
            << new Dismantlement(Card::Spade, 2)
            << new ThunderSlash(Card::Spade, 3)
            << new Peach(Card::Spade, 4)
            << new Railgun(Card::Spade, 5)
            << new Slash(Card::Spade, 6)
            << new Potential(Card::Spade, 7)
            << new Tantei(Card::Spade, 8)
            << new SavageAssault(Card::Spade, 9)
            << new Snatch(Card::Spade,10)
            << new Inspiration(Card::Spade, 11)
            << new ThunderSlash(Card::Spade, 12);
    Card *ana = new OffensiveCar(Card::Spade, 13);
    ana->setObjectName("jaguarE");
    cards << ana;

    // club
    cards
            << new WindJink(Card::Club, 1)
            << new Locust(Card::Club, 2)
            << new Turnover(Card::Club, 3)
            << new ThunderSlash(Card::Club, 4)
            << new Injector(Card::Club, 5)
            << new Slash(Card::Club, 6)
            << new Nullification(Card::Club, 7)
            << new Wolf(Card::Club, 8)
            << new Duel(Card::Club, 9)
            << new ThunderSlash(Card::Club, 10)
            << new RenwangShield(Card::Club, 11)
            << new ExNihilo(Card::Club, 12)
            << new Tantei(Card::Club, 13)
                        ;

    // heart
    cards
            << new Bow(Card::Heart, 1)
            << new Sacrifice(Card::Heart, 2)
            << new Slash(Card::Heart, 3)
            << new Collateral(Card::Heart, 4)
            << new FireSlash(Card::Heart, 5)
            << new WindJink(Card::Heart, 6)
            << new Tantei(Card::Heart, 7)
            << new FireSlash(Card::Heart, 8)
            << new Snatch(Card::Heart, 9)
            << new JuicePeach(Card::Heart, 10)
            << new SoilJink(Card::Heart, 11)
            << new Lightning(Card::Heart, 12)
            << new Nullification(Card::Heart, 13);

    // diamond
    cards
            << new ArcheryAttack(Card::Diamond, 1)
            << new Dismantlement(Card::Diamond, 2)
            << new Turnover(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new Potential(Card::Diamond, 5)
            << new RedAlert(Card::Diamond, 6);
    ana = new Analeptic(Card::Diamond, 7);
    ana->setObjectName("ana_korn");
    cards
            << ana
            << new Bat(Card::Diamond, 8)
            << new WindJink(Card::Diamond, 9)
            << new FireSlash(Card::Diamond, 10)
            << new IronChain(Card::Diamond, 11)
            << new SoilJink(Card::Diamond, 12)
            << new Sacrifice(Card::Diamond, 13);

    foreach(Card *card, cards)
        card->setParent(this);
    skills << new InjectorSkill
            << new Skill("jaguarE");

    type = CardPack;
}

ADD_PACKAGE(BlackDragon)
ADD_PACKAGE(ThunderBird)
