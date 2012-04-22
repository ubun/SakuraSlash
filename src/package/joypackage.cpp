#include "joypackage.h"
#include "engine.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = move.from;
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardedPile || move.to_place == Player::Special)
       && move.to == NULL
       && from->isAlive()){

        LogMessage log;
        log.card_str = getEffectIdString();
        log.from = from;

        Room *room = from->getRoom();

        if(getSuit() == Spade){            
            log.type = "$ShitLostHp";
            room->sendLog(log);

            room->loseHp(from);

            return;
        }

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Club: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        log.type = "$ShitDamage";
        room->sendLog(log);

        room->damage(damage);
    }
}

bool Shit::HasShit(const Card *card){
    if(card->isVirtualCard()){
        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->objectName() == "shit")
                return true;
        }

        return false;
    }else
        return card->objectName() == "shit";
}

Stink::Stink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("stink");
    target_fixed = true;
}

QString Stink::getSubtype() const{
    return "disgusting_card";
}

QString Stink::getEffectPath(bool is_male) const{
    return "audio/card/common/stink.ogg";
}

void Stink::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *nextfriend = targets.isEmpty() ? source->getNextAlive() : targets.first();
    room->setEmotion(nextfriend, "bad");
    if(!room->askForCard(nextfriend, "jink", "haochou", true)){
        room->swapSeat(nextfriend, nextfriend->getNextAlive());
    }
    else room->setEmotion(nextfriend, "good");
}

// -----------  Deluge -----------------

Deluge::Deluge(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("deluge");

    judge.pattern = QRegExp("(.*):(.*):([AK])");
    judge.good = false;
    judge.reason = objectName();
}

void Deluge::takeEffect(ServerPlayer *target) const{
    QList<const Card *> cards = target->getCards("he");

    Room *room = target->getRoom();
    int n = qMin(cards.length(), target->aliveCount());
    if(n == 0)
        return;

    qShuffle(cards);
    cards = cards.mid(0, n);

    QList<int> card_ids;
    foreach(const Card *card, cards){
        card_ids << card->getEffectiveId();
        room->throwCard(card);
    }

    room->fillAG(card_ids);

    QList<ServerPlayer *> players = room->getOtherPlayers(target);
    players << target;
    players = players.mid(0, n);
    foreach(ServerPlayer *player, players){
        if(player->isAlive()){
            int card_id = room->askForAG(player, card_ids, false, "deluge");
            card_ids.removeOne(card_id);

            room->takeAG(player, card_id);
        }
    }

    foreach(int card_id, card_ids)
        room->takeAG(NULL, card_id);

    room->broadcastInvoke("clearAG");
}

// -----------  Typhoon -----------------

Typhoon::Typhoon(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("typhoon");

    judge.pattern = QRegExp("(.*):(diamond):([2-9])");
    judge.good = false;
    judge.reason = objectName();
}

void Typhoon::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(target);
    foreach(ServerPlayer *player, players){
        if(target->distanceTo(player) == 1){
            int discard_num = qMin(6, player->getHandcardNum());
            if(discard_num == 0)
                room->setEmotion(player, "good");
            else{
                room->setEmotion(player, "bad");
                room->broadcastInvoke("animate", "typhoon:" + player->objectName());
                room->broadcastInvoke("playAudio", "typhoon");

                room->askForDiscard(player, objectName(), discard_num);
            }

            room->getThread()->delay();
        }
    }
}

// -----------  Earthquake -----------------

Earthquake::Earthquake(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("earthquake");

    judge.pattern = QRegExp("(.*):(club):([2-9])");
    judge.good = false;
    judge.reason = objectName();
}

void Earthquake::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    QList<ServerPlayer *> players = room->getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(target->distanceTo(player) <= 1 ||
           (player->getDefensiveCar() && target->distanceTo(player) <= 2)){
            if(player->getEquips().isEmpty()){
                room->setEmotion(player, "good");
            }else{
                room->setEmotion(player, "bad");
                room->broadcastInvoke("playAudio", "earthquake");
                player->throwAllEquips();
            }

            room->getThread()->delay();
        }
    }
}

// -----------  Volcano -----------------

Volcano::Volcano(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("volcano");

    judge.pattern = QRegExp("(.*):(heart):([2-9])");
    judge.good = false;
    judge.reason = objectName();
}

void Volcano::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();

    QList<ServerPlayer *> players = room->getAllPlayers();

    foreach(ServerPlayer *player, players){
        int point = player->getDefensiveCar() && target != player ?
                    3 - target->distanceTo(player) :
                    2 - target->distanceTo(player);
        if(point >= 1){
            DamageStruct damage;
            damage.card = this;
            damage.damage = point;
            damage.to = player;
            damage.nature = DamageStruct::Fire;

            room->broadcastInvoke("playAudio", "volcano");
            room->damage(damage);
        }
    }
}

// -----------  MudSlide -----------------
MudSlide::MudSlide(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("mudslide");

    judge.pattern = QRegExp("(.*):(spade|club):([AK47])");
    judge.good = false;
    judge.reason = objectName();
}

void MudSlide::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    QList<ServerPlayer *> players = room->getAllPlayers();
    int to_destroy = 4;
    foreach(ServerPlayer *player, players){
        room->broadcastInvoke("playAudio", "mudslide");

        QList<const Card *> equips = player->getEquips();
        if(equips.isEmpty()){
            DamageStruct damage;
            damage.card = this;
            damage.to = player;
            room->damage(damage);
        }else{
            int i, n = qMin(equips.length(), to_destroy);
            for(i=0; i<n; i++){
                room->throwCard(equips.at(i));
            }

            to_destroy -= n;
            if(to_destroy == 0)
                break;
        }
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
                if(p->getOffensiveCar() == parent() &&
                   p->askForSkillInvoke("grab_peach", data))
                {
                    room->throwCard(p->getOffensiveCar());
                    p->playCardEffect(objectName());
                    p->obtainCard(use.card);

                    return true;
                }
            }
        }

        return false;
    }
};

Monkey::Monkey(Card::Suit suit, int number)
    :OffensiveCar(suit, number)
{
    setObjectName("monkey");

    grab_peach = new GrabPeach;
    grab_peach->setParent(this);
}

void Monkey::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(grab_peach);
}

void Monkey::onUninstall(ServerPlayer *player) const{

}

QString Monkey::getEffectPath(bool ) const{
    return "audio/card/common/monkey.ogg";
}

class GaleShellSkill: public ArmorSkill{
public:
    GaleShellSkill():ArmorSkill("gale-shell"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GaleShellDamage";
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

GaleShell::GaleShell(Suit suit, int number) :Armor(suit, number){
    setObjectName("gale_shell");
    skill = new GaleShellSkill;

    target_fixed = false;
}

bool GaleShell::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void GaleShell::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

class ThunderShellSkill: public ArmorSkill{
public:
    ThunderShellSkill():ArmorSkill("thunder-shell"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Thunder){
            LogMessage log;
            log.type = "#ThunSheProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            if(damage.nature == DamageStruct::Normal)
                log.arg2 = "normal_nature";
            else
                log.arg2 = damage.nature == DamageStruct::Fire ? "fire_nature" : "thunder_nature";
            player->getRoom()->sendLog(log);
            return true;
        }
        return false;
    }
};

ThunderShell::ThunderShell(Suit suit, int number) :Armor(suit, number){
    setObjectName("thunder_shell");
    skill = new ThunderShellSkill;

    target_fixed = false;
}

bool ThunderShell::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void ThunderShell::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

class YitianSwordSkill : public WeaponSkill{
public:
    YitianSwordSkill():WeaponSkill("yitian_sword"){
        events << DamageComplete;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() != Player::NotActive)
           return false;

        if(player->askForSkillInvoke("yitian_sword"))
            player->getRoom()->askForUseCard(player, "slash", "@askforslash");

        return false;
    }
};

class YitianSword: public Weapon{
public:
    YitianSword(Suit suit, int number): Weapon(suit, number, 2){
        setObjectName("yitian_sword");
        skill = new YitianSwordSkill;
    }
    virtual void onMove(const CardMoveStruct &move) const;
};

void YitianSword::onMove(const CardMoveStruct &move) const{
    if(move.from_place == Player::Equip && move.from->isAlive()){
        Room *room = move.from->getRoom();

        bool invoke = move.from->askForSkillInvoke("yitian-lost");
        if(!invoke)
            return;

        ServerPlayer *target = room->askForPlayerChosen(move.from, room->getAllPlayers(), "yitian-lost");
        DamageStruct damage;
        damage.from = move.from;
        damage.to = target;
        damage.card = this;

        room->damage(damage);
    }
}

DisasterPackage::DisasterPackage()
    :Package("disaster")
{
    QList<Card *> cards;

    cards << new Deluge(Card::Spade, 1)
            << new Typhoon(Card::Spade, 4)
            << new Earthquake(Card::Club, 10)
            << new Volcano(Card::Heart, 13)
            << new MudSlide(Card::Heart, 7);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13)
            << new Shit(Card::Spade, 10)
            << new Stink(Card::Diamond, 1);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

JoyEquipPackage::JoyEquipPackage()
    :Package("joy_equip")
{
    (new Monkey(Card::Diamond, 5))->setParent(this);
    (new GaleShell(Card::Heart, 5))->setParent(this);
    (new ThunderShell(Card::Club, 5))->setParent(this);
    (new YitianSword(Card::Spade, 5))->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Joy)
ADD_PACKAGE(Disaster)
ADD_PACKAGE(JoyEquip)
