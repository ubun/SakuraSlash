#include "general.h"
#include "standard.h"
#include "standard-generals-a.h"
#include "standard-generals-b.h"
#include "standard-generals-b.cpp"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"

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

class Jianxiong:public MasochismSkill{
public:
    Jianxiong():MasochismSkill("jianxiong"){
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, caocao))
            return;

        QVariant data = QVariant::fromValue(card);
        if(room->askForSkillInvoke(caocao, "jianxiong", data)){
            room->playSkillEffect(objectName());
            caocao->obtainCard(card);
        }
    }
};

class Hujia:public TriggerSkill{
public:
    Hujia():TriggerSkill("hujia$"){
        events << CardAsked;
        default_choice = "ignore";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("hujia");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *caocao, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        Room *room = caocao->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("wei", caocao);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->playSkillEffect(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)caocao);
        foreach(ServerPlayer *liege, lieges){
            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink:" + caocao->objectName(), tohelp);
            if(jink){
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class TuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi";
    }
};

class Tuxi:public PhaseChangeSkill{
public:
    Tuxi():PhaseChangeSkill("tuxi"){
        view_as_skill = new TuxiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

class Tiandu:public TriggerSkill{
public:
    Tiandu():TriggerSkill("tiandu"){
        frequency = Frequent;

        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *guojia, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        Room *room = guojia->getRoom();
        if(guojia->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(guojia, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }

            guojia->obtainCard(judge->card);
            room->playSkillEffect(objectName());

            return true;
        }

        return false;
    }
};

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        if(!room->askForSkillInvoke(guojia, objectName()))
            return;

        room->playSkillEffect(objectName());

        int x = damage.damage, i;
        for(i=0; i<x; i++){
            guojia->drawCards(2);
            QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - 2);

            while(room->askForYiji(guojia, yiji_cards))
                ; // empty loop
        }

    }
};

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
            judge.pattern = QRegExp("(.*):(heart):(.*)");
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

class Fankui:public MasochismSkill{
public:
    Fankui():MasochismSkill("fankui"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)){
            int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
            else
                room->obtainCard(simayi, card_id);
            room->playSkillEffect(objectName());
        }
    }
};

class GuicaiViewAsSkill:public OneCardViewAsSkill{
public:
    GuicaiViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@guicai";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new GuicaiCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Guicai: public TriggerSkill{
public:
    Guicai():TriggerSkill("guicai"){
        view_as_skill = new GuicaiViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guicai-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@guicai", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class LuoyiBuff: public TriggerSkill{
public:
    LuoyiBuff():TriggerSkill("#luoyi"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("luoyi") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash") || reason->inherits("Duel")){
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            xuchu->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Luoyi: public DrawCardsSkill{
public:
    Luoyi():DrawCardsSkill("luoyi"){

    }

    virtual int getDrawNum(ServerPlayer *xuchu, int n) const{
        Room *room = xuchu->getRoom();
        if(room->askForSkillInvoke(xuchu, objectName())){
            room->playSkillEffect(objectName());

            xuchu->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class Luoshen:public TriggerSkill{
public:
    Luoshen():TriggerSkill("luoshen"){
        events << PhaseChange << FinishJudge;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhenji, QVariant &data) const{
        if(event == PhaseChange && zhenji->getPhase() == Player::Start){
            Room *room = zhenji->getRoom();
            while(zhenji->askForSkillInvoke("luoshen")){
                zhenji->setFlags("luoshen");
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = zhenji;

                room->judge(judge);
                if(judge.isBad())
                    break;
            }

            zhenji->setFlags("-luoshen");
        }else if(event == FinishJudge){
            if(zhenji->hasFlag("luoshen")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isBlack()){
                    zhenji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
    }
};

class Qingguo:public OneCardViewAsSkill{
public:
    Qingguo():OneCardViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Jink *jink = new Jink(card->getSuit(), card->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(card->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink";
    }
};

class RendeViewAsSkill:public ViewAsSkill{
public:
    RendeViewAsSkill():ViewAsSkill("rende"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ServerInfo.GameMode == "04_1v3"
           && selected.length() + Self->getMark("rende") >= 2)
           return false;
        else
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        RendeCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public PhaseChangeSkill{
public:
    Rende():PhaseChangeSkill("rende"){
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("RendeCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "rende", 0);

        return false;
    }
};

class JijiangViewAsSkill:public ZeroCardViewAsSkill{
public:
    JijiangViewAsSkill():ZeroCardViewAsSkill("jijiang$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("jijiang") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new JijiangCard;
    }
};

class Jijiang: public TriggerSkill{
public:
    Jijiang():TriggerSkill("jijiang$"){
        events << CardAsked;
        default_choice = "ignore";

        view_as_skill = new JijiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("jijiang");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = liubei->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(liubei, objectName()))
            return false;

        room->playSkillEffect(objectName());

        QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(), tohelp);
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class Wusheng:public OneCardViewAsSkill{
public:
    Wusheng():OneCardViewAsSkill("wusheng"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(!card->isRed())
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
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

class Tieji:public SlashBuffSkill{
public:
    Tieji():SlashBuffSkill("tieji"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *machao = effect.from;

        Room *room = machao->getRoom();
        if(effect.from->askForSkillInvoke("tieji", QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = machao;

            room->judge(judge);
            if(judge.isGood()){
                room->slashResult(effect, NULL);
                return true;
            }
        }

        return false;
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect(objectName());

            int n = qMin(5, room->alivePlayerCount());
            room->doGuanxing(zhuge, room->getNCards(n, false), false);
        }

        return false;
    }
};

class Kongcheng: public ProhibitSkill{
public:
    Kongcheng():ProhibitSkill("kongcheng"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Slash") || card->inherits("Duel"))
            return to->isKongcheng();
        else
            return false;
    }
};

class KongchengEffect: public TriggerSkill{
public:
    KongchengEffect():TriggerSkill("#kongcheng-effect"){
        frequency = Compulsory;

        events << CardLost;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand)
                player->getRoom()->playSkillEffect("kongcheng");
        }

        return false;
    }
};

class Jizhi:public TriggerSkill{
public:
    Jizhi():TriggerSkill("jizhi"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

class Zhiheng:public ViewAsSkill{
public:
    Zhiheng():ViewAsSkill("zhiheng"){

    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);

        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhihengCard");
    }
};

class Jiuyuan: public TriggerSkill{
public:
    Jiuyuan():TriggerSkill("jiuyuan$"){
        events << Dying << AskForPeachesDone << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("jiuyuan");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunquan, QVariant &data) const{
        Room *room =  sunquan->getRoom();
        switch(event){
        case Dying: {
                foreach(ServerPlayer *wu, room->getOtherPlayers(sunquan)){
                    if(wu->getKingdom() == "wu"){
                        room->playSkillEffect("jiuyuan", 1);
                        break;
                    }
                }
                break;
            }

        case CardEffected: {
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(effect.card->inherits("Peach") && effect.from->getKingdom() == "wu"
                   && sunquan != effect.from && sunquan->hasFlag("dying"))
                {
                    int index = effect.from->getGeneral()->isMale() ? 2 : 3;
                    room->playSkillEffect("jiuyuan", index);
                    sunquan->setFlags("jiuyuan");

                    LogMessage log;
                    log.type = "#JiuyuanExtraRecover";
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
                if(sunquan->getHp() > 0 && sunquan->hasFlag("jiuyuan"))
                    room->playSkillEffect("jiuyuan", 4);
                sunquan->setFlags("-jiuyuan");

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Yingzi:public DrawCardsSkill{
public:
    Yingzi():DrawCardsSkill("yingzi"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        if(room->askForSkillInvoke(zhouyu, objectName())){
            room->playSkillEffect(objectName());
            return n + 1;
        }else
            return n;
    }
};

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << CardResponsed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lumeng, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("Slash"))
            lumeng->setFlags("keji_use_slash");

        return false;
    }
};

class KejiSkip: public PhaseChangeSkill{
public:
    KejiSkip():PhaseChangeSkill("#keji-skip"){
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *lumeng) const{
        if(lumeng->getPhase() == Player::Start){
            lumeng->setFlags("-keji_use_slash");
        }else if(lumeng->getPhase() == Player::Discard){
            if(!lumeng->hasFlag("keji_use_slash") &&
               lumeng->getSlashCount() == 0 &&
               lumeng->askForSkillInvoke("keji"))
            {
                lumeng->getRoom()->playSkillEffect("keji");

                return true;
            }
        }

        return false;
    }
};

class Lianying: public TriggerSkill{
public:
    Lianying():TriggerSkill("lianying"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *luxun, QVariant &data) const{
        if(luxun->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();

            if(move->from_place == Player::Hand){
                Room *room = luxun->getRoom();
                if(room->askForSkillInvoke(luxun, objectName())){
                    room->playSkillEffect(objectName());

                    luxun->drawCards(1);
                }
            }
        }

        return false;
    }
};

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class Kurou: public ZeroCardViewAsSkill{
public:
    Kurou():ZeroCardViewAsSkill("kurou"){

    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Guose: public OneCardViewAsSkill{
public:
    Guose():OneCardViewAsSkill("guose"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class LiuliViewAsSkill: public OneCardViewAsSkill{
public:
    LiuliViewAsSkill():OneCardViewAsSkill("liuli"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@liuli";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->addSubcard(card_item->getFilteredCard());

        return liuli_card;
    }
};

class Liuli: public TriggerSkill{
public:
    Liuli():TriggerSkill("liuli"){
        view_as_skill = new LiuliViewAsSkill;

        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *daqiao, QVariant &data) const{
        Room *room = daqiao->getRoom();

        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash") && !daqiao->isNude() && room->alivePlayerCount() > 2){
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(effect.from);

            bool can_invoke = false;
            foreach(ServerPlayer *player, players){
                if(daqiao->inMyAttackRange(player)){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke){
                QString prompt = "@liuli:" + effect.from->objectName();
                room->setPlayerFlag(effect.from, "slash_source");
                if(room->askForUseCard(daqiao, "@@liuli", prompt)){
                    foreach(ServerPlayer *player, players){
                        if(player->hasFlag("liuli_target")){
                            room->setPlayerFlag(effect.from, "-slash_source");
                            room->setPlayerFlag(player, "-liuli_target");
                            effect.to = player;

                            room->cardEffect(effect);
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
};

class Chujia: public GameStartSkill{
public:
    Chujia():GameStartSkill("chujia"){
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return GameStartSkill::triggerable(target) && target->getGeneralName() == "sunshangxiang";
    }

    virtual void onGameStart(ServerPlayer *player) const{
        if(player->askForSkillInvoke(objectName())){
            Room *room = player->getRoom();
            room->transfigure(player, "sp_sunshangxiang", true, false);
            room->setPlayerProperty(player, "kingdom", "shu");
        }
    }
};

class Jieyin: public ViewAsSkill{
public:
    Jieyin():ViewAsSkill("jieyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JieyinCard");
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

class Xiaoji: public TriggerSkill{
public:
    Xiaoji():TriggerSkill("xiaoji"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            Room *room = sunshangxiang->getRoom();
            if(room->askForSkillInvoke(sunshangxiang, objectName())){
                room->playSkillEffect(objectName());
                sunshangxiang->drawCards(2);
            }
        }

        return false;
    }
};

class Wushuang: public TriggerSkill{
public:
    Wushuang():TriggerSkill("wushuang"){
        events << SlashProceed;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = lubu->getRoom();
        room->playSkillEffect(objectName());

        QString slasher = lubu->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher);

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
        }

        room->slashResult(effect, jink);

        return true;
    }
};

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());

        return lijian_card;
    }
};

class Biyue: public PhaseChangeSkill{
public:
    Biyue():PhaseChangeSkill("biyue"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        if(diaochan->getPhase() == Player::Finish){
            Room *room = diaochan->getRoom();
            if(room->askForSkillInvoke(diaochan, objectName())){
                room->playSkillEffect(objectName());
                diaochan->drawCards(1);
            }
        }

        return false;
    }
};

class Qingnang: public OneCardViewAsSkill{
public:
    Qingnang():OneCardViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());

        return qingnang_card;
    }
};

class Jijiu: public OneCardViewAsSkill{
public:
    Jijiu():OneCardViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("peach") && player->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

class Tuoqiao: public ZeroCardViewAsSkill{
public:
    Tuoqiao():ZeroCardViewAsSkill("tuoqiao"){
        huanzhuang_card = new HuanzhuangCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("HuanzhuangCard"))
            return false;

        return player->getGeneralName() == "diaochan";
    }

    virtual const Card *viewAs() const{
        return huanzhuang_card;
    }

private:
    HuanzhuangCard *huanzhuang_card;
};

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Indulgence");
    }
};

class Mashu: public DistanceSkill{
public:
    Mashu():DistanceSkill("mashu")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

////////////
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

class ZhenxiangViewAsSkill: public ZeroCardViewAsSkill{
public:
    ZhenxiangViewAsSkill():ZeroCardViewAsSkill("zhenxiangV"){
    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhenxiangCard");
    }
    virtual const Card *viewAs() const{
        return new ZhenxiangCard;
    }
};

class Zhenxiang: public TriggerSkill{
public:
    Zhenxiang():TriggerSkill("zhenxiang"){
        events << CardEffected << PhaseChange;
        view_as_skill = new ZhenxiangViewAsSkill;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *kudou, QVariant &data) const{
        Room *room = kudou->getRoom();
        if(event == PhaseChange && kudou->getPhase() == Player::Finish){
            QString dongchaee_name = room->getTag("Dongchaee").toString();
            if(!dongchaee_name.isEmpty()){
                ServerPlayer *dongchaee = room->findChild<ServerPlayer *>(dongchaee_name);
                room->setPlayerFlag(dongchaee, "-dongchaee");

                room->setTag("Dongchaee", QVariant());
                room->setTag("Dongchaer", QVariant());
            }
        }
        else if(event == CardEffected && kudou->getPhase() == Player::NotActive){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.from != kudou && effect.card->isNDTrick()
                && !effect.card->inherits("AmazingGrace")
                && room->askForSkillInvoke(kudou, objectName(), data)){
                room->showAllCards(effect.from, effect.to);
            }
        }
        return false;
    }
};

class Wuwei:public MasochismSkill{
public:
    Wuwei():MasochismSkill("wuwei$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("wuwei");
    }

    virtual void onDamaged(ServerPlayer *kudou, const DamageStruct &damage) const{
        Room *room = kudou->getRoom();
        if(room->getCurrent() != kudou && damage.from && room->askForSkillInvoke(kudou, objectName())){
            QList<ServerPlayer *> players = room->getOtherPlayers(kudou);
            foreach(ServerPlayer *player, players){
                QString result = room->askForChoice(player, objectName(), "accept+ignore");
                if(result == "ignore")
                    continue;
                const Card *slash = room->askForCard(player, "slash", "@wuwei-slash");
                if(slash){
                    CardUseStruct card_use;
                    card_use.card = slash;
                    card_use.from = kudou;
                    card_use.to << damage.from;
                    room->useCard(card_use);
                }
            }
        }
    }
};

class Rexue:public MasochismSkill{
public:
    Rexue():MasochismSkill("rexue"){
    }

    virtual void onDamaged(ServerPlayer *hatto, const DamageStruct &damage) const{
        Room *room = hatto->getRoom();
        if(hatto->askForSkillInvoke(objectName())){
            int x = damage.damage, i;
            for(i=0; i<x; i++){
                ServerPlayer *target = room->askForPlayerChosen(hatto, room->getOtherPlayers(hatto), "rexue");
                int card_id = room->drawCard();
                target->addToPile("rexue", card_id);
                room->acquireSkill(target, "rexue_effect", false);
            }
        }
    }
};

class RexueEffect:public TriggerSkill{
public:
    RexueEffect():TriggerSkill("rexue_effect"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DyingStruct dying_data = data.value<DyingStruct>();
        if(player == dying_data.who &&
           !dying_data.who->getPile("rexue").isEmpty() &&
           dying_data.who->askForSkillInvoke("rexue_get", data)){
            int peach = player->getPile("rexue").first();
            room->throwCard(peach);
            RecoverStruct recover;
            recover.card = Sanguosha->getCard(peach);
            ServerPlayer *hatto = room->findPlayerBySkillName("rexue");
            if(hatto)
                recover.who = hatto;

            room->recover(player, recover);
            return false;
        }
        return false;
    }
};

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

class Jiaojin:public OneCardViewAsSkill{
public:
    Jiaojin():OneCardViewAsSkill("jiaojin"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JiaojinCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JiaojinCard *card = new JiaojinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Chenshui: public PhaseChangeSkill{
public:
    Chenshui():PhaseChangeSkill("chenshui"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Start && room->askForSkillInvoke(mouri, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(mouri, room->getAlivePlayers(), "chenshuiprotect");
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                if (player == target)
                    continue;
                if (player == mouri)
                    continue;
                const Card *slash = room->askForCard(player, "slash", "chenshui-slash:" + mouri->getGeneralName());
                if(!slash){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.to = player;
                    damage.from = mouri;
                    room->damage(damage);
                }
            }
            mouri->skip(Player::Judge);
            mouri->skip(Player::Draw);
            mouri->skip(Player::Play);
            //return true;
        }
        return false;
    }
};

class Jingxing:public MasochismSkill{
public:
    Jingxing():MasochismSkill("jingxing"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *mori, const DamageStruct &damage) const{
        Room *room = mori->getRoom();
        if(!room->askForSkillInvoke(mori, objectName()))
            return;

        for(int i = damage.damage; i>0; i--){
            ServerPlayer *target = room->askForPlayerChosen(mori, room->getAlivePlayers(), objectName());
            if(target == mori)
                target->drawCards(2);
            else{
                if(!target->isNude())
                    room->askForDiscard(target, objectName(), 1, false, true);
            }
        }
    }
};

class Zhinang: public TriggerSkill{
public:
    Zhinang():TriggerSkill("zhinang"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *conan, QVariant &data) const{
        Room *room = conan->getRoom();
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Hand && room->askForSkillInvoke(conan, objectName())){
            //room->playSkillEffect(objectName());
             conan->drawCards(1);
        }
        return false;
    }
};

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

class Mazui: public ZeroCardViewAsSkill{
public:
    Mazui():ZeroCardViewAsSkill("mazui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MazuiCard");
    }

    virtual const Card *viewAs() const{
        return new MazuiCard;
    }
};

class Fuyuan: public TriggerSkill{
public:
    Fuyuan():TriggerSkill("fuyuan$"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("fuyuan");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard){
            if(player->getMark("aptx") > 0){
                room->detachSkillFromPlayer(player, "zhenxiang");
                room->detachSkillFromPlayer(player, "wuwei");

                LogMessage log;
                log.type = "$Fuyuanrb";
                log.from = player;
                room->sendLog(log);
                player->setMark("aptx", 0);
            }

            QVariant num = player->getHandcardNum();
            player->tag["FC_S"] = num;
            return false;
        }
        if(player->getPhase() == Player::Finish){
            int num = player->tag.value("FC_S").toInt();
            if(num - player->getHandcardNum() >= 2 && player->askForSkillInvoke(objectName(), data)){
                player->setMark("aptx", 1);
                room->acquireSkill(player, "zhenxiang");
                if(player->isLord())
                    room->acquireSkill(player, "wuwei");
            }
        }
        return false;
    }
};

class Pantao: public TriggerSkill{
public:
    Pantao():TriggerSkill("pantao"){
        events << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!target->getRoom()->findPlayerBySkillName(objectName()))
            return false;
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *ai = room->findPlayerBySkillName(objectName());

        if(ai && ai->isWounded() && ai->faceUp() && ai->askForSkillInvoke(objectName())){
            ai->turnOver();

            PlayerStar zhenggong = room->getTag("Pantao").value<PlayerStar>();
            if(zhenggong == NULL){
                PlayerStar p = player;
                room->setTag("Pantao", QVariant::fromValue(p));
                player->gainMark("pantao");
            }

            room->setCurrent(ai);
            ai->play();

            return true;

        }else{
            PlayerStar p = room->getTag("Pantao").value<PlayerStar>();
            if(p){
                p->loseMark("pantao");
                room->setCurrent(p);
                room->setTag("Pantao", QVariant());
            }
        }

        return false;
    }
};

class Qianfu: public TriggerSkill{
public:
    Qianfu():TriggerSkill("qianfu"){
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->faceUp();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isRed())
            return false;

        LogMessage log;
        log.type = "#SkillNullify";
        log.from = player;
        log.arg = objectName();
        log.arg2 = effect.slash->objectName();

        player->getRoom()->sendLog(log);

        return true;
    }
};

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

class Shiyan:public OneCardViewAsSkill{
public:
    Shiyan():OneCardViewAsSkill("shiyan"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShiyanCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShiyanCard *card = new ShiyanCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Tianzhen: public TriggerSkill{
public:
    Tianzhen():TriggerSkill("tianzhen"){
        events << Predamaged << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ayumi, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("TrickCard")){
            LogMessage log;
            log.type = "#TianzhenPrevent";
            log.from = ayumi;
            log.to << damage.to;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            ayumi->getRoom()->sendLog(log);
            return true;
        }
        return false;
    }
};

class Lanman: public OneCardViewAsSkill{
public:
    Lanman():OneCardViewAsSkill("lanman"){
    }

    virtual bool isEnabledAtPlay(const Player *yoshida) const{
        return yoshida->getHandcardNum() < 4;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ExNihilo *exnihilo = new ExNihilo(first->getSuit(), first->getNumber());
        exnihilo->addSubcard(first->getId());
        exnihilo->setSkillName(objectName());
        return exnihilo;
    }
};

class Dontcry: public PhaseChangeSkill{
public:
    Dontcry():PhaseChangeSkill("dontcry"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("cry") < 4
                && target->isAlive();
    }

    virtual bool onPhaseChange(ServerPlayer *ayumi) const{
        Room *room = ayumi->getRoom();
        if(ayumi->getPhase() == Player::Start){
            ayumi->addMark("cry");
        }else if(ayumi->getPhase() == Player::Discard){
            if(ayumi->getMark("cry") == 1)
                //room->playSkillEffect(objectName());
                return true;
        }else if(ayumi->getPhase() == Player::Finish){
            if(ayumi->getMark("cry") == 3 && ayumi->isAlive()){
                room->setPlayerProperty(ayumi, "maxhp", ayumi->getMaxHP()+1);
                //room->playSkillEffect(objectName());
                room->setPlayerProperty(ayumi, "hp", ayumi->getMaxHP());

                LogMessage log;
                log.type = "#BukuWake";
                log.from = ayumi;
                log.arg = objectName();
                room->sendLog(log);
            }
        }
        return false;
    }
};

class DuorenViewAsSkill:public OneCardViewAsSkill{
public:
    DuorenViewAsSkill():OneCardViewAsSkill("duoren"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Jink *jink = new Jink(card->getSuit(), card->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(card->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink";
    }
};

class Duoren: public TriggerSkill{
public:
    Duoren():TriggerSkill("duoren"){
        events << CardResponsed;
        view_as_skill = new DuorenViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *mouriran, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        Room *room = mouriran->getRoom();
        ServerPlayer *source = room->getCurrent();
        if(source->getWeapon() && room->askForSkillInvoke(mouriran, "duoren"))
            mouriran->obtainCard(source->getWeapon());
        return false;
    }
};

class Shouhou: public PhaseChangeSkill{
public:
    Shouhou():PhaseChangeSkill("shouhou"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Start){
            QList<ServerPlayer *> Hurts;
            foreach(ServerPlayer *player, room->getAlivePlayers())
                if(player->isWounded())
                    Hurts << player;
            if(!Hurts.isEmpty() && mouri->askForSkillInvoke(objectName())){
                ServerPlayer *target = room->askForPlayerChosen(mouri, Hurts, "shouhou");
                if(target){
                    RecoverStruct recover;
                    recover.card = NULL;
                    recover.who = mouri;
                    room->recover(target, recover);

                    mouri->skip(Player::Judge);
                    mouri->skip(Player::Draw);
                }
            }
        }
        return false;
    }
};

class Heqi:public TriggerSkill{
public:
    Heqi():TriggerSkill("heqi"){
        events << CardEffected;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *kazuha, QVariant &data) const{
        Room *room = kazuha->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Slash"))
            return false;
        if(kazuha->getHandcardNum() > effect.from->getCardCount(true))
            return false;
        if(effect.from->getJudgingArea().isEmpty() && effect.from->isKongcheng())
            return false;
        if(room->askForSkillInvoke(kazuha, objectName(), data)){
            int card_id = room->askForCardChosen(kazuha, effect.from, "hj", objectName());
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), kazuha, Player::Hand, false);
            else
                room->obtainCard(kazuha, card_id);
            //room->playSkillEffect(objectName());
        }
        return false;
    }
};

ShouqiuCard::ShouqiuCard(){
    target_fixed = true;
}

void ShouqiuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

class ShouqiuViewAsSkill:public OneCardViewAsSkill{
public:
    ShouqiuViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@shouqiu";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ShouqiuCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Shouqiu: public TriggerSkill{
public:
    Shouqiu():TriggerSkill("shouqiu"){
        view_as_skill = new ShouqiuViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@shouqiu-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@shouqiu", prompt, false);

        if(card){
            // the only difference for Guicai & Guidao
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class Shenyong: public TriggerSkill{
public:
    Shenyong():TriggerSkill("shenyong"){
        events << Damage << DamageComplete;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *kyo, QVariant &data) const{
        if(kyo->getPhase() != Player::Play)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->inherits("Slash")){
            return false;
        }
        Room *room = kyo->getRoom();
        if(event == DamageComplete){
            foreach(ServerPlayer *tmp, room->getAllPlayers())
                tmp->setFlags("-shenyong");
            return false;
        }
        damage.to->setFlags("shenyong");
        QList<ServerPlayer *>players = room->getOtherPlayers(damage.to);

        QMutableListIterator<ServerPlayer *> itor(players);
        while(itor.hasNext()){
            itor.next();
            if(!damage.to->inMyAttackRange(itor.value()) || itor.value()->hasFlag("shenyong"))
                itor.remove();
        }

        if(players.isEmpty() || !kyo->askForSkillInvoke(objectName(),data))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(kyo, players, objectName());
        if(target){
            CardEffectStruct effect;
            Slash *slash = new Slash(damage.card->getSuit(), damage.card->getNumber());
            slash->setSkillName("shenyong_slash");
            effect.card = slash;
            effect.from = kyo;
            effect.to = target;
            room->cardEffect(effect);
        }
        return false;
    }
};

class ShentouViewAsSkill: public OneCardViewAsSkill{
public:
    ShentouViewAsSkill():OneCardViewAsSkill("shentou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class Shentou: public PhaseChangeSkill{
public:
    Shentou():PhaseChangeSkill("shentou"){
        view_as_skill = new ShentouViewAsSkill;
    }
    virtual int getPriority() const{
        return 2;
    }
    virtual bool onPhaseChange(ServerPlayer *kid) const{
        if(kid->getPhase() == Player::Discard){
            if(kid->usedTimes("Snatch") > 2 && kid->askForSkillInvoke(objectName()))
                return true;
        }
        return false;
    }
};

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

class Baiyi: public OneCardViewAsSkill{
public:
    Baiyi():OneCardViewAsSkill("baiyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("BaiyiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        BaiyiCard *card = new BaiyiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Feixing: public DistanceSkill{
public:
    Feixing():DistanceSkill("feixing"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()) && to->getEquips().length() == 0)
            return +1;
        else
            return 0;
    }
};

class Yirong:public TriggerSkill{
public:
    Yirong():TriggerSkill("yirong"){
        events << AskForPeachesDone;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("yaiba") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *sharon, QVariant &) const{
        Room *room = sharon->getRoom();
        if(sharon->getHp() <= 0 && sharon->askForSkillInvoke(objectName())){
            room->broadcastInvoke("animate", "lightbox:$yirong");
            sharon->loseMark("yaiba");

            QStringList genlist = Sanguosha->getLimitedGeneralNames();
            foreach(ServerPlayer *player, room->getAllPlayers()){
                genlist.removeOne(player->getGeneralName());
            }

            QString general = room->askForGeneral(sharon, genlist);
            room->transfigure(sharon, general, false);
            //room->acquireSkill(sharon, "yirong", false);
            sharon->getRoom()->setPlayerProperty(sharon, "hp", 3);
        }
        return false;
    }
};

class Wuyu: public TriggerSkill{
public:
    Wuyu():TriggerSkill("wuyu"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *s, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = s;
            log.arg = objectName();
            s->getRoom()->sendLog(log);

            s->drawCards(2);
        }
        return false;
    }
};

class Quzheng: public TriggerSkill{
public:
    Quzheng():TriggerSkill("quzheng"){
        events << CardDiscarded;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("quzheng");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *megure = room->findPlayerBySkillName(objectName());
        if(!megure)
            return false;

        if(player->getPhase() == Player::Discard){
            CardStar card = data.value<CardStar>();
            if(card->getSubcards().isEmpty() || !room->askForSkillInvoke(megure, objectName(), data))
                return false;
            if(megure->hasLordSkill("ranglu")){
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(megure)){
                    if(tmp != player && tmp->getKingdom() == "zhen"){
                        players << tmp;
                    }
                }
                if(!players.isEmpty() && room->askForSkillInvoke(megure, "ranglu", data)){
                    megure = room->askForPlayerChosen(megure, players, "ranglu");
                }
            }
            foreach(int card_id, card->getSubcards()){
                room->obtainCard(megure, card_id);
            }
        }
        return false;
    }
};

class QuzhengSkip: public PhaseChangeSkill{
public:
    QuzhengSkip():PhaseChangeSkill("#quzheng_skip"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Draw;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getHandcardNum() > player->getHp())
            return true;
        return false;
    }
};

/*
class Guilin: public TriggerSkill{
public:
    Guilin():TriggerSkill("guilin"){
        events << PhaseChange;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guilin");
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &) const{
        if(event == PhaseChange && player->getPhase() == Player::Judge && player->getJudgingArea().length()!=0){
            Room *room = player->getRoom();
            ServerPlayer *three = room->findPlayerBySkillName(objectName());
            if(!three->askForSkillInvoke(objectName()))
                return false;
  //          CardEffectStruct effect = data.value<CardEffectStruct>();
            QList<const Card *> three_cards = three->getJudgingArea();
            QList<const Card *> cards = player->getJudgingArea();
            foreach(const Card *card, cards){
               room->moveCardTo(card, three, Player::Judging);
            }
            foreach(const Card *card, three_cards){
               room->moveCardTo(card, player, Player::Judging);
            }
         }
        return false;
    }
};
*/
class Shangchi: public PhaseChangeSkill{
public:
    Shangchi():PhaseChangeSkill("shangchi"){
        default_choice = "me";
    }

    virtual bool onPhaseChange(ServerPlayer *matsumoto) const{
        if(matsumoto->getPhase() == Player::Draw && matsumoto->isWounded()){
            Room *room = matsumoto->getRoom();
            if(room->askForChoice(matsumoto,objectName(),"me+him")=="me")
                room->drawCards(matsumoto,matsumoto->getLostHp());
            else {
                ServerPlayer *target = room->askForPlayerChosen(matsumoto,room->getAlivePlayers(),objectName());
                target->drawCards(matsumoto->getLostHp()-1);
            }
        }

        return false;
    }
};

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

class Diaobing: public ZeroCardViewAsSkill{
public:
    Diaobing():ZeroCardViewAsSkill("diaobing$"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("diaobing");
    }

    virtual const Card *viewAs() const{
        return new DiaobingCard;
    }
};

class AttackPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("Slash") ||
                card->inherits("FireAttack") ||
                card->inherits("Duel");
    }
};

class Qinjian: public TriggerSkill{
public:
    Qinjian():TriggerSkill("qinjian"){
        events << SlashMissed;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash && player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;

            Room *room = player->getRoom();
            room->judge(judge);
            if(judge.isBad())
                return false;
            player->obtainCard(effect.slash);
            room->askForUseCard(player, "slash", "@qinjian-slash");
        }
        return false;
    }
};

void StandardPackage::addGenerals(){
    General *kudoushinichi, *hattoriheiji, *mourikogorou;

    kudoushinichi = new General(this, "kudoushinichi$", "zhen");
    kudoushinichi->addSkill(new Zhenxiang);
    kudoushinichi->addSkill(new Wuwei);

    hattoriheiji = new General(this, "hattoriheiji", "zhen", 3);
    hattoriheiji->addSkill(new Rexue);
    hattoriheiji->addSkill(new Jiaojin);

    mourikogorou = new General(this, "mourikogorou", "zhen", 3);
    mourikogorou->addSkill(new Chenshui);
    mourikogorou->addSkill(new Jingxing);

    General *edogawaconan, *haibaraai, *yoshidaayumi;
    edogawaconan = new General(this, "edogawaconan$", "shao", 3);
    edogawaconan->addSkill(new Zhinang);
    edogawaconan->addSkill(new Mazui);
    edogawaconan->addSkill(new Fuyuan);

    haibaraai = new General(this, "haibaraai", "shao", 3, false);
    haibaraai->addSkill(new Pantao);
    haibaraai->addSkill(new Shiyan);
    haibaraai->addSkill(new Qianfu);

    yoshidaayumi = new General(this, "yoshidaayumi", "shao", 2, false);
    yoshidaayumi->addSkill(new Tianzhen);
    yoshidaayumi->addSkill(new Lanman);
    yoshidaayumi->addSkill(new Dontcry);

    General *mouriran, *touyamakazuha, *kyougokumakoto;
    mouriran = new General(this, "mouriran", "woo", 3, false);
    mouriran->addSkill(new Duoren);
    mouriran->addSkill(new Shouhou);

    touyamakazuha = new General(this, "touyamakazuha", "woo", 3, false);
    touyamakazuha->addSkill(new Heqi);
    touyamakazuha->addSkill(new Shouqiu);

    kyougokumakoto = new General(this, "kyougokumakoto", "woo");
    kyougokumakoto->addSkill(new Shenyong);

    General *kaitoukid, *sharon;
    kaitoukid = new General(this, "kaitoukid", "yi", 3);
    kaitoukid->addSkill(new Shentou);
    kaitoukid->addSkill(new Baiyi);
    kaitoukid->addSkill(new Feixing);

    sharon = new General(this, "sharon", "yi", 3, false);
    sharon->addSkill(new Yirong);
    sharon->addSkill(new MarkAssignSkill("yaiba", 1));
    related_skills.insertMulti("yirong", "#yaiba");
    sharon->addSkill(new Wuyu);

    General *megurejyuuzou, /*shiratorininzaburou,*/ *matsumotokiyonaka, *otagiritoshirou;
    megurejyuuzou = new General(this, "megurejyuuzou", "jing");
    megurejyuuzou->addSkill(new Quzheng);
    megurejyuuzou->addSkill(new QuzhengSkip);
    related_skills.insertMulti("quzheng", "#quzheng_skip");
    megurejyuuzou->addSkill(new Skill("ranglu$"));
/*    shiratorininzaburou = new General(this, "shiratorininzaburou", "jing");
    shiratorininzaburou->addSkill(new Guilin);
*/
    matsumotokiyonaka = new General(this, "matsumotokiyonaka$", "jing");
    matsumotokiyonaka->addSkill(new Shangchi);
    matsumotokiyonaka->addSkill(new Diaobing);

    otagiritoshirou = new General(this, "otagiritoshirou", "jing");
    otagiritoshirou->addSkill(new Qinjian);

    // for skill cards

    addMetaObject<ZhenxiangCard>();
    addMetaObject<JiaojinCard>();
    addMetaObject<MazuiCard>();
    addMetaObject<ShiyanCard>();
    addMetaObject<ShouqiuCard>();
    addMetaObject<BaiyiCard>();
    addMetaObject<DiaobingCard>();

    skills << new RexueEffect;
    patterns[".At"] = new AttackPattern;

    General *kurobakaitou, *nakamoriaoko;
    kurobakaitou = new General(this, "kurobakaitou", "guai");
    kurobakaitou->addSkill(new Tishen);
    kurobakaitou->addSkill(new MarkAssignSkill("fake", 1));
    related_skills.insertMulti("tishen", "#fake");
    //kurobakaitou->addSkill(new MarkAssignSkill("magic", 1));
    kurobakaitou->addSkill(new Moshu);

    nakamoriaoko = new General(this, "nakamoriaoko", "guai", 4, false);
    nakamoriaoko->addSkill(new Renxing);
    nakamoriaoko->addSkill(new Qingmei);

    General *gin, *vodka, *akaishuichi;
    gin = new General(this, "gin$", "hei");
    gin->addSkill(new Ansha);
    gin->addSkill(new MarkAssignSkill("@ansha", 1));
    related_skills.insertMulti("ansha", "#@ansha");
    gin->addSkill(new Juelu);
    gin->addSkill(new Heiyi);

    vodka = new General(this, "vodka", "hei");
    vodka->addSkill(new Maixiong);
    vodka->addSkill(new Dashou);

    akaishuichi = new General(this, "akaishuichi", "te");
    akaishuichi->addSkill(new Jushen);
    akaishuichi->addSkill(new Xunzhi);
    akaishuichi->addSkill(new Xunzhiresult);
    related_skills.insertMulti("xunzhi", "#xunzhiresult");

    General *agasahiroshi, *kobayashisumiko, *yamamuramisae, *aoyamagoushou;

    agasahiroshi = new General(this, "agasahiroshi$", "za");
    agasahiroshi->addSkill(new Gaizao);
    agasahiroshi->addSkill(new Suyuan);
    agasahiroshi->addSkill(new Baomu);

    kobayashisumiko = new General(this, "kobayashisumiko", "za", 4, false);
    kobayashisumiko->addSkill(new Yuanding);
    kobayashisumiko->addSkill(new Qiniao);
    kobayashisumiko->addSkill(new QiniaoSkip);
    related_skills.insertMulti("qiniao", "#qiniaoskip");

    yamamuramisae = new General(this, "yamamuramisae", "za", 3, false);
    yamamuramisae->addSkill(new Biaoche);
    yamamuramisae->addSkill(new Jingshen);

    aoyamagoushou = new General(this, "aoyamagoushou", "za");
    aoyamagoushou->addSkill(new Long);

    // for skill cards
    addMetaObject<CheatCard>();

    addMetaObject<RenxingCard>();
    addMetaObject<AnshaCard>();
    addMetaObject<MaixiongCard>();
    addMetaObject<YuandingCard>();
    addMetaObject<JingshenCard>();
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "uzumaki", "god", 5, true, true);
    new General(this, "haruno", "god", 5, false, true);
}

ADD_PACKAGE(Test)
