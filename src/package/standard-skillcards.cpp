#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
    once = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    if(source->isAlive())
        room->drawCards(source, subcards.length());
}


RendeCard::RendeCard(){
    will_throw = false;
}

void RendeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = NULL;
    if(targets.isEmpty()){
        foreach(ServerPlayer *player, room->getAlivePlayers()){
            if(player != source){
                target = player;
                break;
            }
        }
    }else
        target = targets.first();

    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

JieyinCard::JieyinCard(){
    once = true;
    mute = true;
}

bool JieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);

    int index = -1;
    if(effect.from->getGeneral()->isMale()){
        if(effect.from == effect.to)
            index = 5;
        else if(effect.from->getHp() >= effect.to->getHp())
            index = 3;
        else
            index = 4;
    }else{
        index = 1 + qrand() % 2;
    }

    room->playSkillEffect("jieyin", index);
}

TuxiCard::TuxiCard(){
}

bool TuxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "h", "tuxi");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

FanjianCard::FanjianCard(){
    once = true;
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);
    room->showCard(target, card_id);

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

        room->damage(damage);
    }
}

KurouCard::KurouCard(){
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if(source->isAlive())
        room->drawCards(source, 2);
}

LijianCard::LijianCard(){
    once = true;
}

bool LijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!to_select->getGeneral()->isMale())
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

bool LijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LijianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("lijian");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
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

GuicaiCard::GuicaiCard(){
    target_fixed = true;
    will_throw = false;
}

void GuicaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

LiuliCard::LiuliCard()
{
}


bool LiuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasFlag("slash_source"))
        return false;

    if(!Self->canSlash(to_select))
        return false;

    int card_id = subcards.first();
    if(Self->getWeapon() && Self->getWeapon()->getId() == card_id)
        return Self->distanceTo(to_select) <= 1;
    else
        return true;
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "liuli_target");
}

JijiangCard::JijiangCard(){

}

bool JijiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void JijiangCard::use(Room *room, ServerPlayer *liubei, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
    const Card *slash = NULL;

    QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
    foreach(ServerPlayer *liege, lieges){
        slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(), tohelp);
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

HuanzhuangCard::HuanzhuangCard(){
    target_fixed = true;
}

void HuanzhuangCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *diaochan = card_use.from;

    if(diaochan->getGeneralName() == "diaochan"){
        room->transfigure(diaochan, "sp_diaochan", false, false);
    }else if(diaochan->getGeneralName() == "sp_diaochan"){
        room->transfigure(diaochan, "diaochan", false, false);
    }
}

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

///////////////

ZhenxiangCard::ZhenxiangCard(){
    once = true;
}

bool ZhenxiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void ZhenxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    room->setPlayerFlag(effect.to, "dongchaee");
    room->setTag("Dongchaee", effect.to->objectName());
    room->setTag("Dongchaer", effect.from->objectName());

    room->showAllCards(effect.to, effect.from);

    LogMessage log;
    log.type = "$Zhenxiangissingle";
    log.from = effect.from;
    log.to << effect.to;
    room->sendLog(log);
}

JiaojinCard::JiaojinCard(){
    once = true;
    will_throw = false;
}

bool JiaojinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    return true;
}

void JiaojinCard::use(Room *room, ServerPlayer *heiji, const QList<ServerPlayer *> &targets) const{

    ServerPlayer *target = targets.first();
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    room->showCard(heiji, card->getEffectiveId());
    QString suit_str = card->getSuitString();
    QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
    QString prompt = QString("@jiaojinask:%1::%2").arg(heiji->getGeneralName()).arg(suit_str);
    if(!room->askForCard(target, pattern, prompt)){
        if(target->getCardCount(true) <= 3)
            target->throwAllCards();
        else
            room->askForDiscard(target, "jiaojin", 3, false, true);
    }
}

MazuiCard::MazuiCard(){
    once = true;
}

void MazuiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = true;
    judge.reason = objectName();
    judge.who = source;

    room->judge(judge);
    if(judge.isGood()){
        target->turnOver();
    }
}

ShiyanCard::ShiyanCard(){
    once = true;
    target_fixed = true;
    will_throw = false;
}

void ShiyanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    Card::Suit suit = card->getSuit();
    JudgeStruct judge;
//      judge.pattern = QRegExp("(.*):(spade|club):(.*)");
//      judge.good = true;
      judge.reason = objectName();
      judge.who = source;

    room->judge(judge);
    source->obtainCard(judge.card);

    LogMessage log;
    log.from = source;
    log.card_str = card->getEffectIdString();

    if(judge.card->getSuit()==suit){
        log.type = "$Shiyansuc";
        room->sendLog(log);

        room->drawCards(source,2);
    }
    else{
        log.type = "$Shiyanfail";
        room->sendLog(log);
    }
}


ShouqiuCard::ShouqiuCard(){
    target_fixed = true;
}

void ShouqiuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

BaiyiCard::BaiyiCard(){
    once = true;
}

bool BaiyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void BaiyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->obtainCard(this);
    room->showAllCards(source, targets.first());
}

DiaobingCard::DiaobingCard(){
}

bool DiaobingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    return targets.isEmpty();
}

void DiaobingCard::use(Room *room, ServerPlayer *matsu, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges("jing", matsu);
    const Card *slash = NULL;
    foreach(ServerPlayer *liege, lieges){
        QString result = room->askForChoice(liege, "diaobing", "accept+ignore");
        if(result == "ignore")
            continue;
        slash = room->askForCard(liege, ".At", "@diaobing-slash");
        if(slash){
            if(slash->inherits("Slash") && matsu->getSlashCount() > 0)
                continue;
            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = matsu;
            card_use.to << targets.first();
            room->useCard(card_use);
        }
    }
}

RenxingCard::RenxingCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool RenxingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->getHp() < Self->getHp())
        return false;
    if(to_select->isKongcheng())
        return false;
    return true;
}

void RenxingCard::use(Room *room, ServerPlayer *aoko, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    bool success = aoko->pindian(target, objectName(), this);
    if(success){
        if(target->isNude())
            return;
        int card_id = room->askForCardChosen(aoko, target, "he", "renxing");
        room->obtainCard(aoko, card_id);
    }else{
        room->loseHp(aoko);
    }
}

AnshaCard::AnshaCard(){
    once = true;
    target_fixed = true;
}

void AnshaCard::use(Room *room, ServerPlayer *gin, const QList<ServerPlayer *> &) const{
    ServerPlayer *target = room->askForPlayerChosen(gin, room->getOtherPlayers(gin), "ansha");
    if(target){
        target->setMark("ansha",1);
        room->throwCard(this);
        gin->loseMark("anshamark");
    }
}

MaixiongCard::MaixiongCard(){
    will_throw = false;
}

bool MaixiongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self || to_select->getFlags() == "daoh")
        return false;

    return true;
}

void MaixiongCard::use(Room *room, ServerPlayer *vodka, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty() || targets.first()->getFlags() == "daoh")
        return;

    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Hand, false);
    target->setFlags("daoh");

    int old_value = vodka->getMark("maixiong");
    int new_value = old_value + subcards.length();
    vodka->setMark("maixiong", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = vodka;
        room->recover(vodka, recover);
    }
}

YuandingCard::YuandingCard(){
    will_throw = false;
}

bool YuandingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void YuandingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->showCard(effect.from, effect.card->getId(), effect.to);
    const Card *cart = room->askForCard(effect.to, ".", "@yuandingask:" + effect.from->objectName(), false);
    if(cart){
        effect.from->obtainCard(cart);
        effect.to->obtainCard(effect.card);
    }
}

JingshenCard::JingshenCard(){
    once = true;
    target_fixed = true;
}

bool JingshenCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() < 2;
}

void JingshenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const QList<int> &jipin = source->getPile("jipin");
    source->addToPile("jipin", this->getSubcards().first(), false);
    if(jipin.length() >= room->getAlivePlayers().length() - 1){
        ServerPlayer *target = room->askForPlayerChosen(source, room->getAlivePlayers(), "jingshen");
        if(!target) target = source;
        foreach(int card_id, source->getPile("jipin"))
            room->obtainCard(target, card_id);
/*        room->fillAG(jipin);
        QList<ServerPlayer *> players;
        players << source;
        players << room->getOtherPlayers(source);
        players = players.mid(0, room->getAlivePlayers().length());
        foreach(ServerPlayer *player, players){
           int card_id = room->askForAG(player, jipin, false, objectName());
           //source->removeCardFromPile("jipin", card_id);
           room->throwCard(card_id);
           room->takeAG(player, card_id);
        }
        room->broadcastInvoke("clearAG");
*/    }
}
