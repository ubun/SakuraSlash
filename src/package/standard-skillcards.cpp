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
        if(!room->askForCard(target, pattern, "@sb:" + pattern.mid(1), QVariant::fromValue((PlayerStar)source))){
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

JXCard::JXCard(){
}

bool JXCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->getEquips().isEmpty();
}

void JXCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->throwCard(this);
    room->obtainCard(effect.from, room->askForCardChosen(effect.to, effect.from, "e", "jx"));
}

CLCard::CLCard(){
    will_throw = false;
}

void CLCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target;
    if(!targets.isEmpty())
        target = targets.first();
    else
        target = source;
    const Card *c = Sanguosha->getCard(this->getSubcards().first());
    const EquipCard *equipped = qobject_cast<const EquipCard *>(c);
    equipped->use(room,target,targets);
}

YongleCard::YongleCard(){
}

int YongleCard::getKingdoms(const Player *Self) const{
    QSet<QString> kingdom_set;
    QList<const Player *> players = Self->getSiblings();
    players << Self;
    foreach(const Player *player, players){
        if(player->isDead())
            continue;
        kingdom_set << player->getKingdom();
    }
    return kingdom_set.size();
}

bool YongleCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = getKingdoms(Self);
    return targets.length() < x && !to_select->isKongcheng();
}

bool YongleCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = getKingdoms(Self);
    return targets.length() <= x && !targets.isEmpty();
}

void YongleCard::use(Room *room, ServerPlayer *fangla, const QList<ServerPlayer *> &targets) const{
    foreach(ServerPlayer *tmp, targets){
        const Card *card = tmp->getRandomHandCard();
        fangla->obtainCard(card);
    }
    foreach(ServerPlayer *tmp, targets){
        const Card *card = room->askForCardShow(fangla, tmp, "yongle");
        tmp->obtainCard(card);
    }
}
