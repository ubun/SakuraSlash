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

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Spade: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        from->getRoom()->damage(damage);
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

//girls
Sakura::Sakura(Suit suit, int number):BasicCard(suit, number){
    setObjectName("sakura");
    target_fixed = true;
}

QString Sakura::getSubtype() const{
    return "girl_card";
}

void Sakura::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
//    default_choice = "later";
    room->throwCard(this);
    foreach(ServerPlayer *player, room->getAlivePlayers())
        if(player->getGeneralName() == objectName()){
            room->drawCards(source,1);
            return;
        }
    room->transfigure(source,objectName(),false);
//    room->acquireSkill(source, "sakura", false);
}

void Sakura::onMove(const CardMoveStruct &move) const{
    if(move.to_place != Player::Hand) return;
    Room *room = move.to->getRoom();
    ServerPlayer *from = move.to;
//    if(move.from_place == Player::DrawPile){
    QString answer = room->askForChoice(from, objectName(), "use+zhi+later");
        if (answer=="use")
            use(room,move.to,room->getAllPlayers());
        else if(answer=="zhi"){
            room->throwCard(this);
            room->drawCards(from, 1);
        }
        else return;
//    }
}

Erica::Erica(Suit suit, int number):BasicCard(suit, number){
    setObjectName("erica");
    target_fixed = true;
}

QString Erica::getSubtype() const{
    return "girl_card";
}

void Erica::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
//    default_choice = "later";
    room->throwCard(this);
    foreach(ServerPlayer *player, room->getAlivePlayers())
        if(player->getGeneralName() == objectName()){
            room->drawCards(source,1);
            return;
        }
    room->transfigure(source, objectName(), false);
//    room->acquireSkill(source, "sakura", false);
}

void Erica::onMove(const CardMoveStruct &move) const{
    if(move.to_place != Player::Hand) return;
    Room *room = move.to->getRoom();
    ServerPlayer *from = move.to;
//    if(move.from_place == Player::DrawPile){
    QString answer = room->askForChoice(from, objectName(), "use+zhi+later");
        if (answer=="use")
            use(room,move.to,room->getAllPlayers());
        else if(answer=="zhi"){
            room->throwCard(this);
            room->drawCards(from, 1);
        }
        else return;
//    }
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
        if(target->distanceTo(player) <= 1){
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
        int point = 2 - target->distanceTo(player);
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

// -----------  Locust -----------------

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
    if(target->isKongcheng())
        room->loseHp(target);
    else room->askForDiscard(target,objectName(),1);
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

//extra 2 items
class YitianSwordSkill : public WeaponSkill{
public:
    YitianSwordSkill():WeaponSkill("yitian_sword"){
        events << DamageComplete;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() != Player::NotActive)
           return false;

        if(player->askForSkillInvoke("yitian"))
            player->getRoom()->askForUseCard(player, "slash", "yitian-slash");

        return false;
    }
};

YitianSword::YitianSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("yitian_sword");
    skill = new YitianSwordSkill;
}

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

MoonSpear::MoonSpear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
        setObjectName("moon_spear");
        skill = new MoonSpearSkill;
}


JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;

    cards
            << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 5)
            << new Shit(Card::Spade, 9)
            << new Shit(Card::Diamond, 13)
            << new Sakura(Card::Heart, 4)
            << new Erica(Card::Diamond, 4);

    cards
            << new YitianSword(Card::Spade, 6)
            << new MoonSpear(Card::Diamond, 12)
            << new Deluge(Card::Spade, 1)   //洪水
            << new Typhoon(Card::Diamond, 4) //台风
            << new Earthquake(Card::Club, 10)  //地震
            << new Volcano(Card::Heart, 13)  //火山
            << new MudSlide(Card::Heart, 7)  //泥石流
            << new Locust(Card::Club, 6);  //虫灾



    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Joy);
