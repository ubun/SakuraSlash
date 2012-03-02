#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "engine.h"
#include "client.h"

SQSJCard::SQSJCard(){
}

bool SQSJCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    if(to_select == Self)
        return false;
    if(to_select->hasFlag("Sq1"))
        return false;
    return Self->canSlash(to_select);
}

void SQSJCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->throwCard(this);
    effect.to->setFlags("Sqsj");
}

DCCard::DCCard(){
}

bool DCCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 3)
        return false;
    return to_select->isWounded();
}

void DCCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(this);
    RecoverStruct ov;
    ov.card = this;
    ov.who = effect.from;
    room->recover(effect.to, ov);
}

DLDCard::DLDCard(){
    will_throw = false;
}

bool DLDCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void DLDCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->moveCardTo(effect.card, effect.to, Player::Hand, false);
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    damage.card = effect.card;
    room->damage(damage);
}

QingnangCard::QingnangCard(){
    once = true;
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

CKCard::CKCard(){
    target_fixed = true;
    will_throw = false;
}

void CKCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

SBCard::SBCard(){
    target_fixed = true;
}

void SBCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const QString pattern = Sanguosha->getCard(getSubcards().first())->isRed() ?
                            ".red" : ".black";
    foreach(ServerPlayer *target, room->getOtherPlayers(source)){
        if(!room->askForCard(target, pattern, QVariant::fromValue((PlayerStar)source))){
            DamageStruct dmg;
            dmg.from = source;
            dmg.card = this;
            dmg.to = target;
            room->damage(dmg);
        }
    }
}

YYSDCard::YYSDCard(){
}

bool YYSDCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!Self->inMyAttackRange(to_select))
        return false;
    if(targets.isEmpty())
        return to_select->isWounded();
    return targets.length() == 1;
}

bool YYSDCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void YYSDCard::use(Room *room, ServerPlayer *vodka, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->loseHp(vodka);
    if(vodka->isAlive()){
        ServerPlayer *target = targets.first();
        RecoverStruct recover;
        recover.card = this;
        recover.recover = qMin(2, target->getLostHp());
        recover.who = vodka;
        room->recover(target, recover);

        target = targets.last();
        DamageStruct tc;
        tc.card = this;
        tc.from = vodka;
        tc.to = target;
        room->damage(tc);
    }
}

HDCard::HDCard(){
}

bool HDCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->inMyAttackRange(to_select);
}

void HDCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &r) const{
    room->loseHp(source);
    if(source->isAlive()){
        const Card *card1 = room->askForCard(r.first(), ".red", "@hd:" + source->objectName());
        if(card1)
            card1 = room->askForCard(r.first(), ".red", "@hd:" + source->objectName());
        if(!card1){
            DamageStruct dmg;
            dmg.from = source;
            dmg.to = r.first();
            room->damage(dmg);
        }
    }
}

WQQCard::WQQCard(){
}

bool WQQCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->inMyAttackRange(to_select) && to_select->getGeneral()->isMale();
}

void WQQCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = effect.from->tag["TianxiangDamage"].value<DamageStruct>();
    damage.to = effect.to;
    damage.chain = true;
    room->damage(damage);
}

XinzhanCard::XinzhanCard(){
    target_fixed = true;
    once = true;
}

void XinzhanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(3), left;
    left = cards;

    QList<int> hearts;
    foreach(int card_id, cards){
        const Card *card = Sanguosha->getCard(card_id);
        if(card->getSuit() == Card::Heart)
            hearts << card_id;
    }

    if(!hearts.isEmpty()){
        room->fillAG(cards, source);

        while(!hearts.isEmpty()){
            int card_id = room->askForAG(source, hearts, true, "xinzhan");
            if(card_id == -1)
                break;

            if(!hearts.contains(card_id))
                continue;

            hearts.removeOne(card_id);
            left.removeOne(card_id);

            source->obtainCard(Sanguosha->getCard(card_id));
            room->showCard(source, card_id);
        }

        source->invoke("clearAG");
    }

    if(!left.isEmpty())
        room->doGuanxing(source, left, true);
}

GanluCard::GanluCard(){
    once = true;
}

void GanluCard::swapEquip(ServerPlayer *first, ServerPlayer *second, int index) const{
    const EquipCard *e1 = first->getEquip(index);
    const EquipCard *e2 = second->getEquip(index);

    Room *room = first->getRoom();

    if(e1)
        first->obtainCard(e1);

    if(e2)
        room->moveCardTo(e2, first, Player::Equip);

    if(e1)
        room->moveCardTo(e1, second, Player::Equip);
}

bool GanluCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool GanluCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    switch(targets.length()){
    case 0: return true;
    case 1: {
            int n1 = targets.first()->getEquips().length();
            int n2 = to_select->getEquips().length();
            return qAbs(n1-n2) <= Self->getLostHp();
        }

    default:
        return false;
    }
}

void GanluCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *first = targets.first();
    ServerPlayer *second = targets.at(1);

    int i;
    for(i=0; i<4; i++)
        swapEquip(first, second, i);

    LogMessage log;
    log.type = "#GanluSwap";
    log.from = source;
    log.to = targets;
    room->sendLog(log);
}
