#include "mountain.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "generaloverview.h"
#include "client.h"

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

/*
#include <QCommandLinkButton>

QiaobianCard::QiaobianCard(){

}

bool QiaobianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->getPhase() == Player::Draw)
        return targets.length() <= 2 && !targets.isEmpty();
    else if(Self->getPhase() == Player::Play)
        return targets.length() == 1;
    else
        return targets.isEmpty();
}

bool QiaobianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->getPhase() == Player::Draw)
        return targets.length() < 2 && to_select != Self && !to_select->isKongcheng();
    else if(Self->getPhase() == Player::Play){
        return targets.isEmpty() &&
                (!to_select->getJudgingArea().isEmpty() || !to_select->getEquips().isEmpty());
    }else
        return false;
}

void QiaobianCard::use(Room *room, ServerPlayer *zhanghe, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(zhanghe->getPhase() == Player::Draw){
        foreach(ServerPlayer *target, targets){
            int card_id = room->askForCardChosen(zhanghe, target, "h", "qiaobian");
            room->moveCardTo(Sanguosha->getCard(card_id), zhanghe, Player::Hand, false);
        }
    }else if(zhanghe->getPhase() == Player::Play){
        PlayerStar from = targets.first();
        if(!from->hasEquip() && from->getJudgingArea().isEmpty())
            return;

        int card_id = room->askForCardChosen(zhanghe, from , "ej", "qiaobian");
        const Card *card = Sanguosha->getCard(card_id);
        Player::Place place = room->getCardPlace(card_id);

        int equip_index = -1;
        const DelayedTrick *trick = NULL;
        if(place == Player::Equip){
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            equip_index = static_cast<int>(equip->location());
        }else{
            trick = DelayedTrick::CastFrom(card);
        }

        QList<ServerPlayer *> tos;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(equip_index != -1){
                if(p->getEquip(equip_index) == NULL)
                    tos << p;
            }else{
                if(!zhanghe->isProhibited(p, trick) && !p->containsTrick(trick->objectName()))
                    tos << p;
            }
        }

        if(trick && trick->isVirtualCard())
            delete trick;

        room->setTag("QiaobianTarget", QVariant::fromValue(from));
        ServerPlayer *to = room->askForPlayerChosen(zhanghe, tos, "qiaobian");
        if(to)
            room->moveCardTo(card, to, place);
        room->removeTag("QiaobianTarget");
    }
}

class QiaobianViewAsSkill: public OneCardViewAsSkill{
public:
    QiaobianViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QiaobianCard *card = new QiaobianCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@qiaobian";
    }
};

class Qiaobian: public PhaseChangeSkill{
public:
    Qiaobian():PhaseChangeSkill("qiaobian"){
        view_as_skill = new QiaobianViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *zhanghe) const{
        Room *room = zhanghe->getRoom();

        switch(zhanghe->getPhase()){
        case Player::Start:
        case Player::Finish:
        case Player::NotActive: return false;

        case Player::Judge: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-judge");
        case Player::Draw: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-draw");
        case Player::Play: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-play");
        case Player::Discard: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-discard");
        }

        return false;
    }
};*/

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
/*
class Guixiang: public GameStartSkill{
public:
    Guixiang():GameStartSkill("guixiang"){
        frequency = Limited;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        if(player->getGeneralName() == "caiwenji"
           && player->askForSkillInvoke(objectName()))
        {
            player->getRoom()->setPlayerProperty(player, "general", "sp_caiwenji");
            player->getRoom()->setPlayerProperty(player, "kingdom", "wei");
        }
    }
};

class Tuntian: public DistanceSkill{
public:
    Tuntian():DistanceSkill("tuntian"){
        frequency = NotFrequent;
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if(from->hasSkill(objectName()))
            return -from->getPile("field").length();
        else
            return 0;
    }
};

class TuntianGet: public TriggerSkill{
public:
    TuntianGet():TriggerSkill("#tuntian-get"){
        events << CardLost << CardLostDone << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();

            if((move->from_place == Player::Hand || move->from_place == Player::Equip) && move->to!=player)
                player->tag["InvokeTuntian"] = true;
        }else if(event == CardLostDone){
            if(!player->tag.value("InvokeTuntian", false).toBool())
                return false;
            player->tag.remove("InvokeTuntian");

            if(player->askForSkillInvoke("tuntian", data)){
                Room *room = player->getRoom();

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.good = false;
                judge.reason = "tuntian";
                judge.who = player;

                room->judge(judge);
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "tuntian" && judge->isGood()){
                player->addToPile("field", judge->card->getEffectiveId());
                return true;
            }
        }

        return false;
    }
};

class Zaoxian: public PhaseChangeSkill{
public:
    Zaoxian():PhaseChangeSkill("zaoxian"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("zaoxian") == 0
                && target->getPile("field").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *dengai) const{
        Room *room = dengai->getRoom();

        room->setPlayerMark(dengai, "zaoxian", 1);
        room->loseMaxHp(dengai);

        LogMessage log;
        log.type = "#ZaoxianWake";
        log.from = dengai;
        log.arg = QString::number(dengai->getPile("field").length());
        room->sendLog(log);

        room->acquireSkill(dengai, "jixi");

        return false;
    }
};

JixiCard::JixiCard(){
    target_fixed = true;
}

void JixiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *dengai = card_use.from;

    QList<int> fields = dengai->getPile("field");
    if(fields.isEmpty())
        return ;

    int card_id;
    if(fields.length() == 1)
        card_id = fields.first();
    else{
        room->fillAG(fields, dengai);
        card_id = room->askForAG(dengai, fields, true, "jixi");
        dengai->invoke("clearAG");

        if(card_id == -1)
            return;
    }

    const Card *card = Sanguosha->getCard(card_id);
    Snatch *snatch = new Snatch(card->getSuit(), card->getNumber());
    snatch->setSkillName("jixi");
    snatch->addSubcard(card_id);

    QList<ServerPlayer *> targets;
    QList<const Player *> empty_list;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        if(!snatch->targetFilter(empty_list, p, dengai))
            continue;

        if(dengai->isProhibited(p, snatch))
            continue;

        targets << p;
    }

    if(targets.isEmpty())
        return;

    ServerPlayer *target = room->askForPlayerChosen(dengai, targets, "jixi");

    CardUseStruct use;
    use.card = snatch;
    use.from = dengai;
    use.to << target;

    room->useCard(use);
}

class Jixi:public ZeroCardViewAsSkill{
public:
    Jixi():ZeroCardViewAsSkill("jixi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("field").isEmpty();
    }

    virtual const Card *viewAs() const{
        return new JixiCard;
    }
};

class Jiang: public TriggerSkill{
public:
    Jiang():TriggerSkill("jiang"){
        events << CardUsed << CardEffected;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunce, QVariant &data) const{
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            card = effect.card;
        }

        if(card == NULL)
            return false;

        if(card->inherits("Duel") || (card->inherits("Slash") && card->isRed())){
            if(sunce->askForSkillInvoke(objectName(), data))
                sunce->drawCards(1);
        }

        return false;
    }
};

class Hunzi: public PhaseChangeSkill{
public:
    Hunzi():PhaseChangeSkill("hunzi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("hunzi") == 0
                && target->getPhase() == Player::Start
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();

        LogMessage log;
        log.type = "#HunziWake";
        log.from = sunce;
        room->sendLog(log);

        room->loseMaxHp(sunce);

        room->acquireSkill(sunce, "yinghun");
        room->acquireSkill(sunce, "yingzi");

        room->setPlayerMark(sunce, "hunzi", 1);

        return false;
    }
};

ZhibaCard::ZhibaCard(){
    will_throw = false;
}

bool ZhibaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("sunce_zhiba") && to_select != Self && !to_select->isKongcheng();
}

void ZhibaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *sunce = targets.first();
    if(sunce->getMark("hunzi") > 0 &&
       room->askForChoice(sunce, "zhiba_pindian", "accept+reject") == "reject")
    {
        return;
    }

    source->pindian(sunce, "zhiba", this);
}

class ZhibaPindian: public OneCardViewAsSkill{
public:
    ZhibaPindian():OneCardViewAsSkill("zhiba_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhibaCard") && player->getKingdom() == "wu" && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhibaCard *card = new ZhibaCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class SunceZhiba: public TriggerSkill{
public:
    SunceZhiba():TriggerSkill("sunce_zhiba$"){
        events << GameStart << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            if(!player->hasLordSkill(objectName()))
                return false;

            Room *room = player->getRoom();
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if(!p->hasSkill("zhiba_pindian"))
                    room->attachSkillToPlayer(p, "zhiba_pindian");
            }
        }else if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == "zhiba" &&
               pindian->to->hasLordSkill(objectName()) &&
               !pindian->isSuccess())
            {
                pindian->to->obtainCard(pindian->from_card);
                pindian->to->obtainCard(pindian->to_card);
            }
        }

        return false;
    }
};

TiaoxinCard::TiaoxinCard(){
    once = true;
    mute = true;
}

bool TiaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->canSlash(Self);
}

void TiaoxinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    if(effect.from->hasArmorEffect("eight_diagram") || effect.from->hasSkill("bazhen"))
        room->playSkillEffect("tiaoxin", 3);
    else
        room->playSkillEffect("tiaoxin", qrand() % 2 + 1);

    const Card *slash = room->askForCard(effect.to, "slash", "@tiaoxin-slash:" + effect.from->objectName());

    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.to << effect.from;
        use.from = effect.to;
        room->useCard(use);
    }else if(!effect.to->isNude()){
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "tiaoxin"));
    }
}

class Tiaoxin: public ZeroCardViewAsSkill{
public:
    Tiaoxin():ZeroCardViewAsSkill("tiaoxin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("TiaoxinCard");
    }

    virtual const Card *viewAs() const{
        return new TiaoxinCard;
    }
};

class Zhiji: public PhaseChangeSkill{
public:
    Zhiji():PhaseChangeSkill("zhiji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("zhiji") == 0
                && target->getPhase() == Player::Start
                && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *jiangwei) const{
        Room *room = jiangwei->getRoom();

        LogMessage log;
        log.type = "#ZhijiWake";
        log.from = jiangwei;
        room->sendLog(log);

        room->playSkillEffect("zhiji");
        room->broadcastInvoke("animate", "lightbox:$zhiji:5000");
        room->getThread()->delay(5000);

        if(room->askForChoice(jiangwei, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = jiangwei;
            room->recover(jiangwei, recover);
        }else
            room->drawCards(jiangwei, 2);

        room->setPlayerMark(jiangwei, "zhiji", 1);
        room->acquireSkill(jiangwei, "guanxing");

        room->loseMaxHp(jiangwei);

        return false;
    }
};

ZhijianCard::ZhijianCard(){
    will_throw = false;
}

bool ZhijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card);
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhijianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *erzhang = effect.from;
    erzhang->getRoom()->moveCardTo(this, effect.to, Player::Equip);
    erzhang->drawCards(1);
}

class Zhijian: public OneCardViewAsSkill{
public:
    Zhijian():OneCardViewAsSkill("zhijian"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhijianCard *zhijian_card = new ZhijianCard();
        zhijian_card->addSubcard(card_item->getFilteredCard());
        return zhijian_card;
    }
};

class Guzheng: public TriggerSkill{
public:
    Guzheng():TriggerSkill("guzheng"){
        events << CardDiscarded;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guzheng");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());

        if(erzhang == NULL)
            return false;

        if(player->getPhase() == Player::Discard){
            QVariantList guzheng = erzhang->tag["Guzheng"].toList();

            CardStar card = data.value<CardStar>();
            foreach(int card_id, card->getSubcards()){
                guzheng << card_id;
            }

            erzhang->tag["Guzheng"] = guzheng;
        }

        return false;
    }
};

class GuzhengGet: public PhaseChangeSkill{
public:
    GuzhengGet():PhaseChangeSkill("#guzheng-get"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guzheng");
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->isDead())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());
        if(erzhang == NULL)
            return false;

        QVariantList guzheng_cards = erzhang->tag["Guzheng"].toList();
        erzhang->tag.remove("Guzheng");

        QList<int> cards;
        foreach(QVariant card_data, guzheng_cards){
            int card_id = card_data.toInt();
            if(room->getCardPlace(card_id) == Player::DiscardedPile)
                cards << card_id;
        }

        if(cards.isEmpty())
            return false;

        if(erzhang->askForSkillInvoke("guzheng", cards.length())){
            room->fillAG(cards, erzhang);

            int to_back = room->askForAG(erzhang, cards, false, objectName());
            player->obtainCard(Sanguosha->getCard(to_back));

            cards.removeOne(to_back);

            erzhang->invoke("clearAG");

            foreach(int card_id, cards)
                erzhang->obtainCard(Sanguosha->getCard(card_id));
        }

        return false;
    }
};

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

    General *morofushitakaaki = new General(this, "morofushitakaaki", "zhen", 3);
    morofushitakaaki->addSkill(new Kongcheng);
    morofushitakaaki->addSkill(new Kanpo);

    General *kawaguchisatoshi = new General(this, "kawaguchisatoshi", "shao", 1);

    General *sawadahiroki = new General(this, "sawadahiroki", "shao", 3);

    General *kojimagenji = new General(this, "kojimagenji", "woo");
    kojimagenji->addSkill(new Zhibao);

    General *kamenyaibaa = new General(this, "kamenyaibaa", "yi");

    General *datewataru = new General(this, "datewataru", "jing", 3);

    General *yokomizo = new General(this, "yokomizo", "jing", 3);

    General *kurobatouichi = new General(this, "kurobatouichi", "guai", 1);

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

ADD_PACKAGE(Mountain);
