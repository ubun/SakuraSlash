#include "maneuvering.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "skill.h"

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

QString Analeptic::getEffectPath(bool is_male) const{
    return Card::getEffectPath();
}

bool Analeptic::IsAvailable(){
    return ! Self->hasUsed("Analeptic");
}

bool Analeptic::isAvailable() const{
    return IsAvailable();
}

void Analeptic::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->cardEffect(this, source, source);
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
        log.from = effect.from;
        room->sendLog(log);

        room->setPlayerFlag(effect.to, "drank");
    }
}

class FanSkill: public WeaponSkill{
public:
    FanSkill():WeaponSkill("fan"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.nature == DamageStruct::Normal){
            if(player->getRoom()->askForSkillInvoke(player, objectName())){
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

class MachineSkill: public ArmorSkill{
public:
    MachineSkill():ArmorSkill("machine"){
       events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start){
            Room *room = player->getRoom();
            int x = player->getHp() - player->getHandcardNum();
            if(x<=0){
                QList<ServerPlayer *> targets;
                targets << player;
                const EquipCard *equipped = qobject_cast<const EquipCard *>(player->getArmor());
                equipped->use(room,player->getNextAlive(),targets);
//                room->throwCard(player->getNextAlive()->getArmor());
//                room->moveCardTo(player->getArmor(),player->getNextAlive(),Player::Equip);
            }else
               player->drawCards(x);
        }
        return false;
    }
};

Machine::Machine(Suit suit, int number):Armor(suit, number){
    setObjectName("machine");
    skill = new MachineSkill;
}

class DoggySkill: public ArmorSkill{
public:
    DoggySkill():ArmorSkill("doggy"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start){
        Room *room = player->getRoom();
        if(player->isWounded()){
            RecoverStruct recover;
            recover.card = NULL;
            recover.who = player;
            room->recover(player, recover);
        }else{
            room->loseHp(player);
        }
    }
        return false;
    }
};

Doggy::Doggy(Suit suit, int number):Armor(suit, number){
    setObjectName("doggy");
    skill = new DoggySkill;
}

FireAttack::FireAttack(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("fire_attack");
}

bool FireAttack::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{    
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

QString IronChain::getEffectPath(bool is_male) const{
    return QString();
}

bool IronChain::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() >= 2)
        return false;

    return true;
}

bool IronChain::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    if(getSkillName() == "guhuo")
        return targets.length() == 1 || targets.length() == 2;
    else
        return targets.length() <= 2;
}

void IronChain::onUse(Room *room, const CardUseStruct &card_use) const{
    if(card_use.to.isEmpty()){
        room->throwCard(this);
        room->playCardEffect("@recast", card_use.from->getGeneral()->isMale());
        card_use.from->drawCards(1);
    }else
        TrickCard::onUse(room, card_use);
}

void IronChain::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{   
    room->throwCard(this);

    room->playCardEffect("@tiesuo", source->getGeneral()->isMale());
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

bool SupplyShortage::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    if(Self->hasSkill("qicai"))
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

class FloriationSkill: public ArmorSkill{
public:
    FloriationSkill():ArmorSkill("floriation"){
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

Floriation::Floriation(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("floriation");
    skill = new FloriationSkill;
}
//暂时无法实现
class YajiaoSkill: public WeaponSkill{
public:
    YajiaoSkill():WeaponSkill("yajiao"){
        events << SlashMissed;
    }
    virtual int getPriority() const{
        return 2;
    }
    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("Jink"))
            player->obtainCard(card_star);
        return false;
    }
};

Yajiao::Yajiao(Suit suit, int number):Weapon(suit, number, 3){
    setObjectName("yajiao");
    skill = new YajiaoSkill;
}


//樱战杀增补

//装备篇
class PistolSkill: public WeaponSkill{
public:
    PistolSkill():WeaponSkill("pistol"){
        events << CardEffected << CardResponsed << HpRecover << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        room->setPlayerProperty(player, "atk", player->getHp());
        player->setAttackRange(player->getHp());
        if(event==Damaged)
            return true;
        else {
           room->setPlayerProperty(player, "atk", player->getHp());
           return false;
       }
    }
};

Pistol::Pistol(Suit suit, int number):Weapon(suit, number, 0){
    setObjectName("pistol");
    skill = new PistolSkill;
}

void Pistol::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    Room *room = player->getRoom();
    room->setPlayerProperty(player, "atk", player->getHp());
    if(attach_skill)
        room->attachSkillToPlayer(player, objectName());
}

class MuramasaSkill: public WeaponSkill{
public:
    MuramasaSkill():WeaponSkill("muramasa"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.card && damage.card->inherits("Slash")){
        QString choice = room->askForChoice(player, "muramasa", "sky+yks");
        if(choice=="yks")
            damage.from = NULL;
        else {
        QList<ServerPlayer *> players = room->getAllPlayers();
        ServerPlayer *target = room->askForPlayerChosen(player, players, "muramasa");
        damage.from = target;
    }
        if(damage.from==player) return false;
        room->damage(damage);
        return true;
       }
    }
};

Muramasa::Muramasa(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("muramasa");
    skill = new MuramasaSkill;
}

Mazinka::Mazinka(Card::Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("mazinka");
}

void Mazinka::onUninstall(ServerPlayer *player) const{
    if(player->isAlive()){
        Room *room = player->getRoom();
        room->killPlayer(room->getCurrent());
    }
}

void Mazinka::onMove(const CardMoveStruct &move) const{
    if(move.from_place == Player::Equip && move.from->isAlive()){
        move.from->getRoom()->killPlayer(move.from->getRoom()->getCurrent());
    }
}

class GrabPeach: public TriggerSkill{
public:
    GrabPeach():TriggerSkill("grab_peach"){
        events << CardUsed;
    }
    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach")){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            foreach(ServerPlayer *p, players){
                if(p->getArmor() == parent() && p->askForSkillInvoke("grab_peach", data))
                {
                    room->throwCard(p->getArmor());
                    room->playCardEffect(objectName(), p->getGeneral()->isMale());
                    p->obtainCard(use.card);
                    return true;
                }
            }
        }
        return false;
    }
};

Cat::Cat(Card::Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("cat");
    grab_peach = new GrabPeach;
    grab_peach->setParent(this);
}

void Cat::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(grab_peach);
}

void Cat::onUninstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->removeTriggerSkill(grab_peach);
}

QString Cat::getEffectPath(bool ) const{
    return "audio/card/common/cat.ogg";
}

class UfoSkill: public ArmorSkill{
public:
    UfoSkill():ArmorSkill("ufo"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature == DamageStruct::Fire){
                LogMessage log;
                log.type = "#UFODamage";
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

Ufo::Ufo(Suit suit, int number) :Armor(suit, number){
    setObjectName("ufo");
    skill = new UfoSkill;
}

void Ufo::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *target = source;
    if(room->askForSkillInvoke(source, objectName())){
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *player, room->getAllPlayers()){
           if(source->inMyAttackRange(player))
              players << player;
        }
          target = room->askForPlayerChosen(source, players, objectName());
    }
    if(target->getArmor())
       room->throwCard(target->getArmor());
    room->moveCardTo(this, target, Player::Equip, true);
}


class ETSkill: public ArmorSkill{
public:
    ETSkill():ArmorSkill("et"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Thunder){
                LogMessage log;
                log.type = "#ETProtect";
                log.from = player;
                log.arg = QString::number(damage.damage);
                if(damage.nature == DamageStruct::Normal)
                    log.arg2 = "normal_nature";
                else log.arg2 = damage.nature == DamageStruct::Fire?"fire_nature":"thunder_nature";
                player->getRoom()->sendLog(log);
                return true;
            }
        return false;
    }
};

ET::ET(Suit suit, int number) :Armor(suit, number){
    setObjectName("et");
    skill = new ETSkill;
}

void ET::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *target = source;
    if(room->askForSkillInvoke(source, objectName()))
      target = room->askForPlayerChosen(source, room->getAllPlayers(), objectName());
    if(target->getArmor())
       room->throwCard(target->getArmor());
    room->moveCardTo(this, target, Player::Equip, true);
}

Redalert::Redalert(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("redalert");
}

void Redalert::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *trick = room->askForCard(effect.to, "trick", "redalert-trick:" + effect.from->getGeneralName());
    if(trick == NULL)
        room->loseHp(effect.to);
}

Renew::Renew(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("renew");
    target_fixed = true;
}

void Renew::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->setCurrent(source);
    source->play();
}

Hitself::Hitself(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("hitself");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(diamond):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool Hitself::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    return true;
}

void Hitself::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Discard);
}

Turnover::Turnover(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("turnover");
}

bool Turnover::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Turnover::onEffect(const CardEffectStruct &effect) const{
    effect.to->turnOver();
}

Bathroom::Bathroom(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("bathroom");

}

bool Bathroom::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Bathroom::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->showAllCards(effect.to, effect.from);
}

ManeuveringPackage::ManeuveringPackage()
    :Package("maneuvering")
{
    QList<Card *> cards;

    // spade
    cards << new GudingBlade(Card::Spade, 1)
            << new Machine(Card::Spade, 2)
            << new Analeptic(Card::Spade, 3)
            << new ThunderSlash(Card::Spade, 4)
            << new ThunderSlash(Card::Spade, 5)
            << new ThunderSlash(Card::Spade, 6)
            << new ThunderSlash(Card::Spade, 7)
            << new ThunderSlash(Card::Spade, 8)
            << new Analeptic(Card::Spade, 9)
            << new SupplyShortage(Card::Spade,10)
            << new IronChain(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new Nullification(Card::Spade, 13);

    // club
    cards << new Doggy(Card::Club, 1)
            << new Machine(Card::Club, 2)
            << new Analeptic(Card::Club, 3)
            << new SupplyShortage(Card::Club, 4)
            << new ThunderSlash(Card::Club, 5)
            << new ThunderSlash(Card::Club, 6)
            << new ThunderSlash(Card::Club, 7)
            << new ThunderSlash(Card::Club, 8)
            << new Analeptic(Card::Club, 9)
            << new IronChain(Card::Club, 10)
            << new Floriation(Card::Club, 11)
            << new IronChain(Card::Club, 12)
            << new IronChain(Card::Club, 13);

    // heart
    cards << new Nullification(Card::Heart, 1)
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
    cards << new Fan(Card::Diamond, 1)
            << new Peach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new FireSlash(Card::Diamond, 5)
            << new Jink(Card::Diamond, 6)
            << new Jink(Card::Diamond, 7)
            << new Jink(Card::Diamond, 8)
            << new Analeptic(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)
            << new Jink(Card::Diamond, 11)
            << new FireAttack(Card::Diamond, 12)
            << new Bathroom(Card::Diamond, 13);



//樱战杀增补
    DefensiveHorse *opukiloo = new DefensiveHorse(Card::Spade, 13);
    opukiloo->setObjectName("opukiloo");
    OffensiveHorse *xiangjing = new OffensiveHorse(Card::Diamond, 13);
    xiangjing->setObjectName("xiangjing");

    cards
        << new Hitself(Card::Heart, 1)  //养精蓄锐
        << new Muramasa(Card::Spade, 1) //妖刀村正
        << new ArcheryAttack(Card::Diamond, 1)
        << new Jink(Card::Club, 1)

        << new Bathroom(Card::Heart, 2) //浴室探查
        << new Dismantlement(Card::Spade, 2)
        << new Dismantlement(Card::Diamond, 2)
        << new Hitself(Card::Club, 2)

        << new Slash(Card::Heart, 3)
        << new ThunderSlash(Card::Spade, 3)
        << new Turnover(Card::Diamond, 3)  //背后偷袭
        << new Turnover(Card::Club, 3)

        << new Collateral(Card::Heart, 4)
        << new Peach(Card::Spade, 4)
        << new FireSlash(Card::Diamond, 4)
        << new ThunderSlash(Card::Club, 4)

        << new FireSlash(Card::Heart, 5)
        << new ET(Card::Spade, 5)  //樱的战斗服 有色眼镜
        << new Ufo(Card::Diamond, 5)  //巫毒娃娃
        << new Cat(Card::Club, 5)  //拿破仑

        << new Jink(Card::Heart, 6)
        << new Slash(Card::Spade, 6)
        << new Redalert(Card::Diamond, 6) //袭击警报
        << new Slash(Card::Club, 6)

        << new Renew(Card::Heart, 7) //灵力爆发
        << new Analeptic(Card::Spade, 7)
        << new Renew(Card::Diamond, 7)
        << new Nullification(Card::Club, 7)

        << new FireSlash(Card::Heart, 8)
        << new Pistol(Card::Spade, 8) //左轮手枪
        << new Pistol(Card::Diamond, 8)
        << new Collateral(Card::Club, 8)

        << new Snatch(Card::Heart, 9)
        << new SavageAssault(Card::Spade, 9)
        << new Jink(Card::Diamond, 9)
        << new Duel(Card::Club, 9)

        << new Peach(Card::Heart, 10)
        << new Snatch(Card::Spade, 10)
        << new FireSlash(Card::Diamond, 10)
        << new ThunderSlash(Card::Club, 10)

        << new Jink(Card::Heart, 11)
        << new Hitself(Card::Spade, 11)
        << new IronChain(Card::Diamond, 11)
        << new Jink(Card::Club, 11)

        << new Lightning(Card::Heart, 12)
        << new ThunderSlash(Card::Spade, 12)
        << new Jink(Card::Diamond, 12)
        << new ExNihilo(Card::Club, 12)

        << new Nullification(Card::Heart, 13)
        << opukiloo
        << xiangjing
        << new Mazinka(Card::Club, 13); //魔神器



    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Maneuvering)
