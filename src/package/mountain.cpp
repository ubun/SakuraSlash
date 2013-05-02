#include "mountain.h"
#include "engine.h"
#include "standard.h"
#include "maneuvering.h"
#include "carditem.h"
#include "generaloverview.h"
#include "client.h"

class Bansha: public TriggerSkill{
public:
    Bansha():TriggerSkill("bansha"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *yamato, QVariant &data) const{
        Room* room = yamato->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        PlayerStar target = effect.to;
        if(!target->isKongcheng() &&
           target->getHandcardNum() >= yamato->getHandcardNum() &&
           room->askForSkillInvoke(yamato, objectName(), QVariant::fromValue(target))){
            room->playSkillEffect(objectName());
            yamato->obtainCard(target->getRandomHandCard(), false);
        }
        return false;
    }
};

class Kongcheng: public ProhibitSkill{
public:
    Kongcheng():ProhibitSkill("kongcheng"){
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card) const{
        if(to->getHandcardNum() < to->getHp())
            return card->inherits("Slash") || card->inherits("Duel") || card->inherits("Turnover");
        else
            return false;
    }
};

class Kanpo: public OneCardViewAsSkill{
public:
    Kanpo():OneCardViewAsSkill("kanpo"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isBlack() && !card->inherits("TrickCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("kanpo");

        return ncard;
    }
};

class Foyuan: public OneCardViewAsSkill{
public:
    Foyuan():OneCardViewAsSkill("foyuan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isBlack() && card->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();

        SupplyShortage *shortage = new SupplyShortage(card->getSuit(), card->getNumber());
        shortage->setSkillName(objectName());
        shortage->addSubcard(card);

        return shortage;
    }
};

class Jihun: public TriggerSkill{
public:
    Jihun():TriggerSkill("jihun"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *satoshi = room->findPlayerBySkillName(objectName());
        if(!satoshi)
            return false;
        QString propty = damage.from ? QString("@jihun:%1:%2").arg(damage.from->objectName()).arg(damage.to->objectName()) :
                         QString("@jihun:%1:%2").arg(QString()).arg(damage.to->objectName());
        const Card *card = room->askForCard(satoshi, ".", propty, data);
        if(card){
            damage.card = card;
            LogMessage log;
            log.type = "$Jihun";
            log.from = satoshi;
            log.to << damage.to;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Chengshi:public MasochismSkill{
public:
    Chengshi():MasochismSkill("chengshi"){
    }

    virtual void onDamaged(ServerPlayer *ki, const DamageStruct &damage) const{
        Room *room = ki->getRoom();
        if(damage.card && damage.card->inherits("Slash")){
            if(ki->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = ki;

                room->judge(judge);
                if(judge.isGood()){
                    QList<ServerPlayer *> players;
                    foreach(ServerPlayer *tmp, room->getAllPlayers()){
                        if(tmp->isWounded())
                            players << tmp;
                    }
                    if(!players.isEmpty()){
                        RecoverStruct stt;
                        stt.who = ki;
                        PlayerStar t = room->askForPlayerChosen(ki, players, objectName());
                        room->recover(t, stt, true);
                    }
                }
            }
        }
    }
};

class Yimeng: public TriggerSkill{
public:
    Yimeng():TriggerSkill("yimeng"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        if(!player->askForSkillInvoke(objectName()))
            return false;
        PlayerStar target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
        if(target){
            QString general = player->getGeneralName();
            room->transfigure(player, target->getGeneralName(), false, false);
            room->transfigure(target, general, false, false);
        }
        return false;
    }
};

class Zhibao: public MasochismSkill{
public:
    Zhibao():MasochismSkill("zhibao"){
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        if(!damage.from || !damage.card || !damage.card->inherits("Slash"))
            return;
        Room *room = player->getRoom();
        ServerPlayer *wusong = room->findPlayerBySkillName(objectName());
        if(wusong){
            if(damage.from->isDead())
                return;
            if(player != wusong && !wusong->isKongcheng()){
                QString prompt = QString("@zhibao:%1:%2").arg(damage.to->objectName()).arg(damage.from->objectName());
                const Card *card = room->askForCard(wusong, "BasicCard", prompt, QVariant::fromValue(damage));
                if(card){
                    Duel *slash = new Duel(Card::NoSuit, 0);
                    slash->setSkillName(objectName());
                    CardUseStruct use;
                    use.card = slash;
                    use.from = wusong;
                    use.to << damage.from;
                    room->useCard(use);
                }
            }
        }
    }
};

FeitiCard::FeitiCard(){
    once = true;
}

bool FeitiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !Self->inMyAttackRange(to_select);
}

void FeitiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->loseMark("@yaiba");
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    damage.from->setFlags("Feiti");
    room->damage(damage);
    damage.from->setFlags("-Feiti");
}

class FeitiViewAsSkill: public ZeroCardViewAsSkill{
public:
    FeitiViewAsSkill():ZeroCardViewAsSkill("feiti"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@feiti";
    }

    virtual const Card *viewAs() const{
        return new FeitiCard;
    }
};

class Feiti: public TriggerSkill{
public:
    Feiti():TriggerSkill("feiti"){
        events << PhaseChange << Predamage;
        view_as_skill = new FeitiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(!player->hasFlag("Feiti") && damage.damage > 0 && player->askForSkillInvoke(objectName())){
                player->gainMark("@yaiba", damage.damage);
                return true;
            }
        }else{
            if(!player->getPhase() == Player::Finish || !player->hasMark("@yaiba"))
                return false;
            room->askForUseCard(player, "@@feiti", "@feiti");
        }
        return false;
    }
};

ShehangCard::ShehangCard(){
    mute = true;
    will_throw = false;
}

bool ShehangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() == 2)
        return true;
    return targets.length() == 1 && Self->canSlash(targets.first());
}

bool ShehangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select->hasEquip() && to_select != Self;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return to_select != Self && first->hasEquip() && Self->canSlash(to_select);
    }else
        return false;
}

void ShehangCard::onUse(Room *room, const CardUseStruct &card_use) const{
    QList<ServerPlayer *> targets = card_use.to;
    PlayerStar source = card_use.from;
    if(room->getCurrent() != source)
        return;
    if(targets.length() > 1)
        targets.removeFirst();
    else if(targets.length() == 1 && source->canSlash(targets.first()))
        targets = card_use.to;
    else
        return;

    int card_id = card_use.to.first()->getEquips().length() == 1 ? card_use.to.first()->getEquips().first()->getId():
                  room->askForCardChosen(source, card_use.to.first(), "e", skill_name);
    if(card_id > -1){
        const Card *weapon = Sanguosha->getCard(card_id);
        Slash *slash = new Slash(weapon->getSuit(), weapon->getNumber());
        slash->setSkillName(skill_name);
        slash->addSubcard(weapon->getEffectiveId());
        CardUseStruct use;
        use.card = slash;
        use.from = source;
        use.to = targets;
        room->useCard(use);
    }
}

class ShehangViewAsSkill:public ZeroCardViewAsSkill{
public:
    ShehangViewAsSkill():ZeroCardViewAsSkill("shehang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPhase() != Player::NotActive && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new ShehangCard;
    }
};

class Shehang: public TriggerSkill{
public:
    Shehang():TriggerSkill("shehang"){
        events << CardAsked;
        view_as_skill = new ShehangViewAsSkill;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if(asked != "slash" && asked != "jink")
            return false;
        Room *room = player->getRoom();
        if(asked == "slash" && room->getCurrent() != player)
            return false;
        if(asked == "jink" && room->getCurrent() == player)
            return false;
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(tmp->hasEquip())
                players << tmp;
        }
        if(players.isEmpty())
            return false;
        if(room->askForSkillInvoke(player, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());
            const Card *card = NULL;
            if(target->getEquips().length() == 1)
                card = target->getEquips().first();
            else
                card = Sanguosha->getCard(room->askForCardChosen(player, target, "e", objectName()));
            if(asked == "slash"){
                Slash *shehang_card = new Slash(card->getSuit(), card->getNumber());
                shehang_card->setSkillName(objectName());
                shehang_card->addSubcard(card);
                room->provide(shehang_card);
            }
            else{
                Jink *shehang_card = new Jink(card->getSuit(), card->getNumber());
                shehang_card->setSkillName(objectName());
                shehang_card->addSubcard(card);
                room->provide(shehang_card);
            }
        }
        return false;
    }
};

CanwuCard::CanwuCard(){
    once = true;
    will_throw = false;
}

bool CanwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->isKongcheng() || !to_select->isWounded())
        return false;
    return to_select->getGender() == General::Male;
}

void CanwuCard::use(Room *room, ServerPlayer *can5, const QList<ServerPlayer *> &targets) const{
    PlayerStar target = targets.first();
    bool success = can5->pindian(target, skill_name, this);
    if(success){
        RecoverStruct ttt;
        ttt.who = can5;
        room->recover(can5, ttt, true);
        room->recover(target, ttt, true);
    }
}

class Canwu: public OneCardViewAsSkill{
public:
    Canwu():OneCardViewAsSkill("canwu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("CanwuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new CanwuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Chongwu: public TriggerSkill{
public:
    Chongwu():TriggerSkill("chongwu"){
        events << HpRecover;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover_struct = data.value<RecoverStruct>();
        int recover = recover_struct.recover;
        if(recover < 1)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *chong5 = room->findPlayerBySkillName(objectName());
        if(chong5 && room->getCurrent() != chong5){
            if(chong5->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->loseHp(chong5);
                return true;
            }
        }
        return false;
    }
};

class Guifu: public TriggerSkill{
public:
    Guifu():TriggerSkill("guifu"){
        events << GameStart << Predamage << Predamaged << HpLost << MaxHpLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart)
            player->gainMark("@hatchet", 2);
        else if(event == HpLost || event == MaxHpLost)
            return true;
        else{
            DamageStruct damage = data.value<DamageStruct>();
            if(event == Predamage)
                player->gainMark("@hatchet", damage.damage);
            else
                player->loseMark("@hatchet", damage.damage);

            if(!player->hasMark("@hatchet")){
                JudgeStruct judge;
                judge.pattern = QRegExp("(Peach|TrickCard):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);
                if(judge.isGood())
                    player->gainMark("@hatchet", 2);
                else{
                    room->setPlayerProperty(player, "hp", 0);
                    room->enterDying(player, &damage);
                }
            }
            return true;
        }
        return false;
    }
};

ShengongCard::ShengongCard(){
    once = true;
}

void ShengongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *touichi = targets.first();
    room->loseHp(source);
    touichi->gainMark("@hatchet");
}

bool ShengongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("shengong") && to_select != Self;
}

class ShengongViewAsSkill: public ZeroCardViewAsSkill{
public:
    ShengongViewAsSkill():ZeroCardViewAsSkill("shengongv"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShengongCard") && player->getKingdom() == "guai";
    }

    virtual const Card *viewAs() const{
        return new ShengongCard;
    }
};

class Shengong: public GameStartSkill{
public:
    Shengong():GameStartSkill("shengong$"){
    }

    virtual void onGameStart(ServerPlayer *touichi) const{
        Room *room = touichi->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players)
            room->attachSkillToPlayer(player, "shengongv");
    }

    virtual void onIdied(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(room->findPlayerBySkillName("shengong"))
            return;
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *tmp, players)
            room->detachSkillFromPlayer(tmp, "shengongv", false);
    }
};

/*
class BasicPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->getTypeId() == Card::Basic;
    }
};

class Xiangle: public TriggerSkill{
public:
    Xiangle():TriggerSkill("xiangle"){
        events << CardEffected;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();

            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#Xiangle";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);

            return !room->askForCard(effect.from, ".basic", "@xiangle-discard", data);
        }

        return false;
    }
};

class Fangquan: public PhaseChangeSkill{
public:
    Fangquan():PhaseChangeSkill("fangquan"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        switch(liushan->getPhase()){
        case Player::Play: {
                bool invoked = liushan->askForSkillInvoke(objectName());
                if(invoked)
                    liushan->setFlags("fangquan");
                return invoked;
            }

        case Player::Finish: {
                if(liushan->hasFlag("fangquan")){
                    Room *room = liushan->getRoom();

                    if(liushan->isKongcheng())
                        return false;

                    room->askForDiscard(liushan, "fangquan", 1);

                    ServerPlayer *player = room->askForPlayerChosen(liushan, room->getOtherPlayers(liushan), objectName());

                    QString name = player->getGeneralName();
                    if(name == "zhugeliang" || name == "shenzhugeliang" || name == "wolong")
                        room->playSkillEffect("fangquan", 1);
                    else
                        room->playSkillEffect("fangquan", 2);

                    LogMessage log;
                    log.type = "#Fangquan";
                    log.from = liushan;
                    log.to << player;
                    room->sendLog(log);

                    player->gainAnExtraTurn();
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};*/

class Wulian:public TriggerSkill{
public:
    Wulian():TriggerSkill("wulian"){
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(player->isAlive()){
            Room *room = effect.from->getRoom();
            if(effect.from->getHp() == effect.to->getHp() ||
               effect.from->getHp() == effect.to->getHandcardNum()){
                LogMessage log;
                log.type = "#Wulian";
                log.from = player;
                log.to << effect.to;
                log.arg = objectName();
                room->sendLog(log);
                room->slashResult(effect, NULL);
                return true;
            }
        }
        return false;
    }
};

class Cheji: public DistanceSkill{
public:
    Cheji():DistanceSkill("cheji"){
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if(from->hasSkill(objectName()) && !from->getArmor())
            return -1;
        else
            return 0;
    }
};

class Luosha: public TriggerSkill{
public:
    Luosha():TriggerSkill("luosha"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.to->getEquips().isEmpty() || !damage.card->inherits("Slash"))
            return false;
        Room *room = player->getRoom();

        if(room->askForSkillInvoke(player, objectName(), data)){
            room->playSkillEffect(objectName());
            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

ZhaodaiCard::ZhaodaiCard(){
    will_throw = false;
    once = true;
}

void ZhaodaiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
    if(effect.from->getRoom()->askForChoice(effect.from, "zhaodai", "tian+zi") == "tian")
        effect.to->drawCards(1);
    else
        effect.from->drawCards(1);
}

class Zhaodai: public OneCardViewAsSkill{
public:
    Zhaodai():OneCardViewAsSkill("zhaodai"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhaodaiCard");
    }

    virtual bool viewFilter(const CardItem *watch) const{
        return watch->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhaodaiCard *card = new ZhaodaiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Kaxiang: public TriggerSkill{
public:
    Kaxiang():TriggerSkill("kaxiang"){
        events << Predamaged;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *linko, QVariant &data) const{
        Room* room = linko->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from || damage.from == damage.to)
            return false;
        if(!linko->isKongcheng() && !damage.from->isKongcheng() &&
           linko->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            bool success = linko->pindian(damage.from, objectName());
            if(success){
                LogMessage log;
                log.type = "#Kaxiang";
                log.from = damage.from;
                log.to << linko;
                log.arg = QString::number(damage.damage);
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class Biaoche: public DistanceSkill{
public:
    Biaoche():DistanceSkill("biaoche"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        //from->parent()->findChildren<const Player *>()
        if(from->hasSkill(objectName()) && to->getHp() > from->getHp())
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() < from->getHp())
            correct ++;
        return correct;
    }
};

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
    }
}

class Jingshen: public OneCardViewAsSkill{
public:
    Jingshen():OneCardViewAsSkill("jingshen"){
    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JingshenCard");
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
    virtual const Card *viewAs(CardItem *card_item) const{
        JingshenCard *card = new JingshenCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

MountainPackage::MountainPackage()
    :Package("mountain")
{
    General *yamatokansuke = new General(this, "yamatokansuke", "zhen", 3);
    yamatokansuke->addSkill(new Skill("bamian", Skill::Compulsory));
    yamatokansuke->addSkill(new Bansha);

    General *morofushitakaaki = new General(this, "morofushitakaaki", "zhen", 3);
    morofushitakaaki->addSkill(new Kongcheng);
    morofushitakaaki->addSkill(new Kanpo);

    General *kawaguchisatoshi = new General(this, "kawaguchisatoshi", "shao", "3/4");
    kawaguchisatoshi->addSkill(new Foyuan);

    General *sawadahiroki = new General(this, "sawadahiroki", "shao", 3);
    sawadahiroki->addSkill(new Jihun);
    sawadahiroki->addSkill(new Chengshi);
    sawadahiroki->addSkill(new Yimeng);

    General *kojimagenji = new General(this, "kojimagenji", "woo");
    kojimagenji->addSkill(new Zhibao);

    General *kamenyaibaa = new General(this, "kamenyaibaa", "yi");
    kamenyaibaa->addSkill(new Feiti);
    addMetaObject<FeitiCard>();

    General *datewataru = new General(this, "datewataru", "jing", 3);
    datewataru->addSkill(new Shehang);
    addMetaObject<ShehangCard>();

    General *yokomizo = new General(this, "yokomizo", "jing", "3/4");
    yokomizo->addSkill(new Canwu);
    yokomizo->addSkill(new Chongwu);
    addMetaObject<CanwuCard>();

    General *kurobatouichi = new General(this, "kurobatouichi$", "guai", 1);
    kurobatouichi->addSkill(new Guifu);
    kurobatouichi->addSkill(new Shengong);
    addMetaObject<ShengongCard>();
    skills << new ShengongViewAsSkill;

    General *rye = new General(this, "rye", "hei", 3);

    General *kir = new General(this, "kir", "hei", 4, false);
    kir->addSkill(new Wulian);

    General *andrewcamel = new General(this, "andrewcamel", "te", 3);
    andrewcamel->addSkill(new Cheji);
    andrewcamel->addSkill(new Luosha);

    General *enomotoazusa = new General(this, "enomotoazusa", "za", 3, false);
    enomotoazusa->addSkill(new Zhaodai);
    enomotoazusa->addSkill(new Kaxiang);
    addMetaObject<ZhaodaiCard>();

    General *yamamuramisae = new General(this, "yamamuramisae", "za", 3, false);
    yamamuramisae->addSkill(new Biaoche);
    yamamuramisae->addSkill(new Jingshen);
    addMetaObject<JingshenCard>();
}

ADD_PACKAGE(Mountain)
