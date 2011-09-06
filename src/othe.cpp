#include "othe.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "skill.h"
#include "evil.h"
#include "maneuvering.h"

GuicaiCard::GuicaiCard(){
    target_fixed = true;
}

void GuicaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

class GuicaiViewAsSkill:public OneCardViewAsSkill{
public:
    GuicaiViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guicai";
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

        const Card *card = room->askForCard(player, "@guicai", prompt, false);
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

GongxinCard::GongxinCard(){
    once = true;
}

bool GongxinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void GongxinCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->doGongxin(effect.from, effect.to);
}

class Gongxin: public ZeroCardViewAsSkill{
public:
    Gongxin():ZeroCardViewAsSkill("gongxin"){
        default_choice = "discard";
    }

    virtual const Card *viewAs() const{
        return new GongxinCard;
    }

    virtual bool isEnabledAtPlay() const{
        return !Self->hasUsed("GongxinCard");
    }
};

class Hujia2: public TriggerSkill{
public:
    Hujia2():TriggerSkill("hujia2"){
        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *caizhaoji, QVariant &data) const{
        if(event == PhaseChange && caizhaoji->getPhase() == Player::Finish){
            int times = 0;
            Room *room = caizhaoji->getRoom();
            while(caizhaoji->askForSkillInvoke(objectName())){
                caizhaoji->setFlags("caizhaoji_hujia");

                room->playSkillEffect(objectName());

                times ++;
                if(times == 4){
                    caizhaoji->turnOver();
                }

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = caizhaoji;

                room->judge(judge);

                if(judge.isBad())
                    break;
            }

            caizhaoji->setFlags("-caizhaoji_hujia");
        }else if(event == FinishJudge){
            if(caizhaoji->hasFlag("caizhaoji_hujia")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isRed()){
                    caizhaoji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
    }
};

class Toudu: public MasochismSkill{
public:
    Toudu():MasochismSkill("toudu"){

    }

    virtual void onDamaged(ServerPlayer *dengshizai, const DamageStruct &) const{
        if(dengshizai->faceUp())
            return;

        if(dengshizai->isKongcheng())
            return;

        if(!dengshizai->askForSkillInvoke("toudu"))
            return;

        Room *room = dengshizai->getRoom();
        if(!room->askForDiscard(dengshizai, "toudu", 1, false, false))
            return;

        dengshizai->turnOver();

        QList<ServerPlayer *> players = room->getOtherPlayers(dengshizai), targets;
        foreach(ServerPlayer *player, players){
            if(dengshizai->canSlash(player, false)){
                targets << player;
            }
        }

        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(dengshizai, targets, "toudu");

            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName("toudu");

            CardUseStruct use;
            use.card = slash;
            use.from = dengshizai;
            use.to << target;
            room->useCard(use);
        }
    }
};

LexueCard::LexueCard(){
    once = true;
    mute = true;
}

bool LexueCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void LexueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->playSkillEffect("lexue", 1);
    const Card *card = room->askForCardShow(effect.to, effect.from, "lexue");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Basic || card->isNDTrick()){
        room->setPlayerMark(effect.from, "lexue", card_id);
        room->setPlayerFlag(effect.from, "lexue");
    }else{
        effect.from->obtainCard(card);
        room->setPlayerFlag(effect.from, "-lexue");
    }
}

class Lexue: public ViewAsSkill{
public:
    Lexue():ViewAsSkill("lexue"){

    }

    virtual int getEffectIndex(ServerPlayer *, const Card *card) const{
        if(card->getTypeId() == Card::Basic)
            return 2;
        else
            return 3;
    }

    virtual bool isEnabledAtPlay() const{
        if(Self->hasUsed("LexueCard") && Self->hasFlag("lexue")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable();
        }else
            return true;
    }

    virtual bool isEnabledAtResponse() const{
        if(Self->getPhase() == Player::NotActive)
            return false;

        if(!Self->hasFlag("lexue"))
            return false;

        if(Self->hasUsed("LexueCard")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return ClientInstance->card_pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("LexueCard") && selected.isEmpty() && Self->hasFlag("lexue")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("LexueCard")){
            if(!Self->hasFlag("lexue"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->addSubcards(cards);
            new_card->setSkillName(objectName());
            return new_card;
        }else{
            return new LexueCard;
        }
    }
};

class Luanji:public ViewAsSkill{
public:
    Luanji():ViewAsSkill("luanji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            ArcheryAttack *aa = new ArcheryAttack(first->getSuit(), 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        }else
            return NULL;
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

class Guangrong: public TriggerSkill{
public:
    Guangrong():TriggerSkill("guangrong"){
        events << CardLost;

        frequency = Frequent;
    }

virtual bool trigger(TriggerEvent, ServerPlayer *conan, QVariant &data) const{
    Room *room = conan->getRoom();
    if(room->getCurrent()==conan)
        return false;
    if(data.canConvert<CardMoveStruct>()){
        CardMoveStruct move = data.value<CardMoveStruct>();
        if(move.from_place != Player::Equip && room->askForSkillInvoke(conan, objectName())){
             room->playSkillEffect(objectName());
             conan->drawCards(1);
        }
    }
    return false;
    }
};

RendeCard::RendeCard(){
    will_throw = false;
}

void RendeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    source->setMark("rende", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

class RendeViewAsSkill:public ViewAsSkill{
public:
    RendeViewAsSkill():ViewAsSkill("rende"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        SkillCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public PhaseChangeSkill{
public:
    Rende():PhaseChangeSkill("rende"){
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start)
            target->setMark("rende", 0);

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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardMoveStruct move = data.value<CardMoveStruct>();
            if(move.from_place == Player::Hand)
                player->getRoom()->playSkillEffect("kongcheng");
        }

        return false;
    }
};

class Hongyan: public FilterSkill{
public:
    Hongyan():FilterSkill("hongyan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *new_card = Card::Clone(card);
        if(new_card) {
            new_card->setSuit(Card::Heart);
            new_card->setSkillName(objectName());
            return new_card;
        }else
            return card;
    }
};

class HongyanRetrial: public TriggerSkill{
public:
    HongyanRetrial():TriggerSkill("#hongyan-retrial"){
        frequency = Compulsory;

        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->getSuit() == Card::Spade){
            LogMessage log;
            log.type = "#HongyanJudge";
            log.from = player;

            Card *new_card = Card::Clone(judge->card);
            new_card->setSuit(Card::Heart);
            new_card->setSkillName("hongyan");
            judge->card = new_card;

            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

class Guixin: public MasochismSkill{
public:
    Guixin():MasochismSkill("guixin"){

    }

    virtual void onDamaged(ServerPlayer *shencc, const DamageStruct &damage) const{
        Room *room = shencc->getRoom();
        int i, x = damage.damage;
        for(i=0; i<x; i++){
            if(shencc->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());

                QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
                if(players.length() >=5)
                    room->broadcastInvoke("animate", "lightbox:$guixin");

                foreach(ServerPlayer *player, players){
                    if(!player->isAllNude()){
                        int card_id = room->askForCardChosen(shencc, player, "hej", objectName());
                        if(room->getCardPlace(card_id) == Player::Hand)
                            room->moveCardTo(Sanguosha->getCard(card_id), shencc, Player::Hand, false);
                        else
                            room->obtainCard(shencc, card_id);
                    }
                }

                shencc->turnOver();
            }else
                break;
        }
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Predamage << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(event == Predamage){
            player->tag["InvokeKuanggu"] = player->distanceTo(damage.to) <= 1;
        }else if(event == Damage){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            if(invoke){
                Room *room = player->getRoom();

                room->playSkillEffect(objectName());

                LogMessage log;
                log.type = "#KuangguRecover";
                log.from = player;
                log.arg = QString::number(damage.damage);
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class Zaoyao: public PhaseChangeSkill{
public:
    Zaoyao():PhaseChangeSkill("zaoyao"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *caochong) const{
        if(caochong->getPhase() == Player::Start)
            caochong->skip(Player::Discard);
        if(caochong->getPhase() == Player::Finish && caochong->getHandcardNum() > 10){
            caochong->throwAllHandCards();
            caochong->getRoom()->loseHp(caochong);
        }
        return false;
    }
};

class Shenjun: public TriggerSkill{
public:
    Shenjun():TriggerSkill("shenjun"){
        events << PhaseChange << Predamaged;
        frequency = Compulsory;
    }   

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange){
            if(player->getPhase() == Player::Start){
                LogMessage log;
                log.from = player;
                log.type = "#ShenjunFlip";
                room->sendLog(log);

                QString new_general = "sb2";
                if(player->getGeneral()->isMale())
                    new_general.append("f");
                room->transfigure(player, new_general, false, false);
            }
        }else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Thunder && damage.from &&
               damage.from->getGeneral()->isMale() != player->getGeneral()->isMale()){

                LogMessage log;
                log.type = "#ShenjunProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);

                return true;
            }
        }

        return false;
    }
};

class ShuangxiongViewAsSkill: public OneCardViewAsSkill{
public:
    ShuangxiongViewAsSkill():OneCardViewAsSkill("shuangxiong"){
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("shuangxiong") != 0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        int value = Self->getMark("shuangxiong");
        if(value == 1)
            return to_select->getFilteredCard()->isBlack();
        else if(value == 2)
            return to_select->getFilteredCard()->isRed();

        return false;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Duel *duel = new Duel(card->getSuit(), card->getNumber());
        duel->addSubcard(card);
        duel->setSkillName(objectName());
        return duel;
    }
};

class Shuangxiong: public TriggerSkill{
public:
    Shuangxiong():TriggerSkill("shuangxiong"){
        view_as_skill = new ShuangxiongViewAsSkill;

        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shuangxiong, QVariant &data) const{
        Room *room = shuangxiong->getRoom();

        if(event == PhaseChange){
            if(shuangxiong->getPhase() == Player::Start){
                room->setPlayerMark(shuangxiong, "shuangxiong", 0);
            }else if(shuangxiong->getPhase() == Player::Draw){
                if(shuangxiong->askForSkillInvoke(objectName())){
                    shuangxiong->setFlags("shuangxiong");

                    JudgeStruct judge;
                    judge.pattern = QRegExp("(.*)");
                    judge.good = true;
                    judge.reason = objectName();
                    judge.who = shuangxiong;

                    room->judge(judge);

                    room->setPlayerMark(shuangxiong, "shuangxiong", judge.card->isRed() ? 1 : 2);
                    shuangxiong->setFlags("-shuangxiong");

                    return true;
                }
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(shuangxiong->hasFlag("shuangxiong")){
                shuangxiong->obtainCard(judge->card);
                return true;
            }
        }

        return false;
    }
};

ZhihengCard::ZhihengCard(){
    target_fixed = true;
    once = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->drawCards(source, subcards.length());
}

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

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("ZhihengCard");
    }
};

class LukangWeiyan: public PhaseChangeSkill{
public:
    LukangWeiyan():PhaseChangeSkill("lukang_weiyan"){
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getHandcardNum() > player->getMaxCards()){
            return "choice2";
        }else{
            return "choice1";
        }
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            Room *room = target->getRoom();
            QString choice = room->askForChoice(target, objectName(), "normal+choice1+choice2");

            if(choice == "normal")
                return false;

            LogMessage log;
            log.from = target;

            QList<Player::Phase> &phases = target->getPhases();
            if(choice == "choice1"){
                // discard phase is before draw phase
                // that is: discard -> draw -> play
                phases.removeOne(Player::Discard);
                int index = phases.indexOf(Player::Draw);
                phases.insert(index, Player::Discard);

                log.type = "#WeiyanChoice1";
            }else{
                // drawing phase is after discard phase
                // that is: play -> discard -> draw
                phases.removeOne(Player::Draw);
                int index = phases.indexOf(Player::Discard);
                phases.insert(index+1, Player::Draw);

                log.type = "#WeiyanChoice2";
            }

            room->sendLog(log);
        }

        return false;
    }
};

TianyiCard::TianyiCard(){
    once = true;
    will_throw = false;
}

bool TianyiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TianyiCard::use(Room *room, ServerPlayer *taishici, const QList<ServerPlayer *> &targets) const{
    bool success = taishici->pindian(targets.first(), "tianyi", this);
    if(success){
        room->setPlayerFlag(taishici, "tianyi_success");
    }else{
        room->setPlayerFlag(taishici, "tianyi_failed");
    }
}

class TianyiViewAsSkill: public OneCardViewAsSkill{
public:
    TianyiViewAsSkill():OneCardViewAsSkill("tianyi"){

    }

    virtual bool isEnabledAtPlay() const{
        return !Self->hasUsed("TianyiCard") && !Self->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new TianyiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Tianyi: public PhaseChangeSkill{
public:
    Tianyi():PhaseChangeSkill("tianyi"){
        view_as_skill = new TianyiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(target->hasFlag("tianyi_failed"))
                room->setPlayerFlag(target, "-tianyi_failed");
            if(target->hasFlag("tianyi_success"))
                room->setPlayerFlag(target, "-tianyi_success");
        }

        return false;
    }
};

class Mengjin: public TriggerSkill{
public:
    Mengjin():TriggerSkill("mengjin"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *pangde, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude()){
            Room *room = pangde->getRoom();
            if(pangde->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                int to_throw = room->askForCardChosen(pangde, effect.to, "he", objectName());
                room->throwCard(to_throw);
            }
        }

        return false;
    }
};

QiaocaiCard::QiaocaiCard(){
    once = true;
}

void QiaocaiCard::onEffect(const CardEffectStruct &effect) const{
    QList<const Card *> cards = effect.to->getJudgingArea();
    foreach(const Card *card, cards){
        effect.from->obtainCard(card);
    }
 }

class Qiaocai: public ZeroCardViewAsSkill{
public:
    Qiaocai():ZeroCardViewAsSkill("qiaocai"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("QiaocaiCard");
    }

    virtual const Card *viewAs() const{
        return new QiaocaiCard;
    }
};

ShensuCard::ShensuCard(){
    mute = true;
}

bool ShensuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("shensu");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}

class ShensuViewAsSkill: public ViewAsSkill{
public:
    ShensuViewAsSkill():ViewAsSkill("shensu"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ClientInstance->card_pattern.endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@shensu");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(ClientInstance->card_pattern.endsWith("1")){
            if(cards.isEmpty())
                return new ShensuCard;
            else
                return NULL;
        }else{
            if(cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public PhaseChangeSkill{
public:
    Shensu():PhaseChangeSkill("shensu"){
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiahouyuan) const{
        Room *room = xiahouyuan->getRoom();

        if(xiahouyuan->getPhase() == Player::Judge){
            if(room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1")){
                xiahouyuan->skip(Player::Draw);
                return true;
            }
        }else if(xiahouyuan->getPhase() == Player::Play){
            if(room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2")){
                return true;
            }
        }

        return false;
    }
};

class Zhenggong: public TriggerSkill{
public:
    Zhenggong():TriggerSkill("zhenggong"){
        events << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return (!target->hasSkill(objectName()) && target->getHandcardNum()>target->getHp());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *dengshizai = room->findPlayerBySkillName(objectName());

            if(dengshizai && dengshizai->faceUp() && dengshizai->askForSkillInvoke(objectName())){
                dengshizai->turnOver();

                PlayerStar zhenggong = room->getTag("Zhenggong").value<PlayerStar>();
                if(zhenggong == NULL){
                    PlayerStar p = player;
                    room->setTag("Zhenggong", QVariant::fromValue(p));
                    player->gainMark("@zhenggong");
                }

                room->setCurrent(dengshizai);
                dengshizai->play();

                return true;

            }else{
                PlayerStar p = room->getTag("Zhenggong").value<PlayerStar>();
                if(p){
                    p->loseMark("@zhenggong");
                    room->setCurrent(p);
                    room->setTag("Zhenggong", QVariant());
                }
            }

            return false;
        }
};

DimengCard::DimengCard(){
    once = true;
}

bool DimengCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(to_select == Self)
        return false;

    if(targets.isEmpty())
        return true;

    if(targets.length() == 1){
        int max_diff = Self->getCardCount(true);
        return max_diff >= qAbs(to_select->getHandcardNum() - targets.first()->getHandcardNum());
    }

    return false;
}

bool DimengCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

void DimengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    // make sure n1 >= n2
    if(n1 < n2){
        qSwap(a, b);
        qSwap(n1, n2);
    }

    int diff = n1 - n2;
    if(diff != 0){
        room->askForDiscard(source, "dimeng", diff, false, true);
    }

    DummyCard *card1 = a->wholeHandCards();
    DummyCard *card2 = b->wholeHandCards();

    if(card1){
        room->moveCardTo(card1, b, Player::Hand, false);
        delete card1;
    }

    room->getThread()->delay();

    if(card2){
        room->moveCardTo(card2, a, Player::Hand, false);
        delete card2;
    }

    LogMessage log;
    log.type = "#Dimeng";
    log.from = a;
    log.to << b;
    log.arg = QString::number(n1);
    log.arg2 = QString::number(n2);
    room->sendLog(log);
}

class Dimeng: public ZeroCardViewAsSkill{
public:
    Dimeng():ZeroCardViewAsSkill("dimeng"){

    }

    virtual const Card *viewAs() const{
        return new DimengCard;
    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("DimengCard");
    }
};

class Naughty: public OneCardViewAsSkill{
public:
    Naughty():OneCardViewAsSkill("naughty"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return (to_select->getCard()->getSuit() == Card::Heart && !to_select->getFilteredCard()->inherits("TrickCard"));
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Hitself *hitself = new Hitself(first->getSuit(), first->getNumber());
        hitself->addSubcard(first->getId());
        hitself->setSkillName(objectName());
        return hitself;
    }
};

class Guiding: public TriggerSkill{
public:
    Guiding():TriggerSkill("guiding"){
//         events << Predamage << Predamaged;
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        QString result = room->askForChoice(player, objectName(), "none+fire+elec");
        if(result=="fire")
            damage.nature = DamageStruct::Fire;
        else
            damage.nature = result=="elec"?DamageStruct::Thunder:DamageStruct::Normal;

            data = QVariant::fromValue(damage);

            LogMessage log;
            log.type = "#Guiding";
            log.from = player;
            player->getRoom()->sendLog(log);
        return false;
    }
};

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

bool GanluCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

bool GanluCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
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

class Ganlu: public ZeroCardViewAsSkill{
public:
    Ganlu():ZeroCardViewAsSkill("ganlu"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("GanluCard");
    }

    virtual const Card *viewAs() const{
        return new GanluCard;
    }
};

class Getitps: public TriggerSkill{
public:
    Getitps():TriggerSkill("getitps"){
//当你指定一名角色成为杀的目标时，该角色可以选择让你莫两张牌，终止杀的结算
        events << SlashEffect;
        default_choice = "yes";
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            Room *room = player->getRoom();
            if(room->askForChoice(effect.to, objectName(), "yes+no")=="yes" && room->askForChoice(effect.from, objectName(), "ye5+n0")=="ye5"){
                    room->playSkillEffect(objectName());
                    effect.from->drawCards(2);

                    LogMessage log;
                    log.type = "#Getitps";
                    log.from = effect.from;
                    log.to << effect.to;
                    room->sendLog(log);

                    return true;
                }
            return false;
    }
};

class Faner: public TriggerSkill{
public:
    Faner():TriggerSkill("faner"){
        events << PhaseChange;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange && player->getPhase() == Player::Judge){
  //          CardEffectStruct effect = data.value<CardEffectStruct>();
            Room *room = player->getRoom();
            QList<const Card *> cards = player->getJudgingArea();
                foreach(const Card *card, cards){
                        room->moveCardTo(card, player->getNextAlive(), Player::Judging);
                }
            }
        return false;
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
                room->playSkillEffect("jiuyuan", 1);
                break;
            }

        case CardEffected: {
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(effect.card->inherits("Peach") && effect.from->getKingdom() == "yin"
                   && sunquan != effect.from && sunquan->hasFlag("dying"))
                {
                    int index = effect.from->getGeneral()->isMale() ? 2 : 3;
                    room->playSkillEffect("jiuyuan", index);

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
                if(sunquan->getHp() > 0)
                    room->playSkillEffect("jiuyuan", 4);

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Nono1: public OneCardViewAsSkill{
public:
    Nono1():OneCardViewAsSkill("nono1"){
//装备牌当五谷丰登
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        AmazingGrace *amazingrace = new AmazingGrace(first->getSuit(), first->getNumber());
        amazingrace->addSubcard(first->getId());
        amazingrace->setSkillName(objectName());
        return amazingrace;
    }
};

class Nono2:public OneCardViewAsSkill{
public:
    Nono2():OneCardViewAsSkill("nono2"){
    }
//无懈和闪互用
    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();

        if(ClientInstance->getStatus() == Client::Responsing){
                QString pattern = ClientInstance->card_pattern;
                if(pattern == "nullification")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Nullification");//无懈当闪
            }
        return false;
    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        QString pattern = ClientInstance->card_pattern;
        return pattern == "jink" || pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Nullification")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else if(card->inherits("Jink")){
            Card *nullif = new Nullification(card->getSuit(), card->getNumber());
            nullif->addSubcard(card);
            nullif->setSkillName(objectName());

            return nullif;
        }else
            return NULL;
    }
};

OthePackage::OthePackage()
    :Package("othe")
{
    //暗处支持华击团的家伙们
    General *kayama, *tsubaki, *kasumi, *yuri, *nonomura, *sb1, *sb2, *sb3;
/*加山雄一 6血
鬼才、英姿、攻心*/
    kayama = new General(this, "kayama$", "yin", 5, true);
    kayama->addSkill(new Guicai);
    kayama->addSkill(new Yingzi);
    kayama->addSkill(new Gongxin);
    kayama->addSkill(new Jiuyuan);
/*高村椿
胡笳，偷渡，乐学*/
    tsubaki = new General(this, "tsubaki", "yin");
        tsubaki->addSkill(new Hujia2);;
        tsubaki->addSkill(new Toudu);
        tsubaki->addSkill(new Lexue);
/*藤井霞
乱击，天妒，光荣*/
    kasumi = new General(this, "kasumi", "yin");
    kasumi->addSkill(new Luanji);
    kasumi->addSkill(new Tiandu);
    kasumi->addSkill(new Guangrong);
/*神原由里
仁德，红颜，空城*/
    yuri = new General(this, "yuri", "yin");
    yuri->addSkill(new Rende);
    yuri->addSkill(new Kongcheng);
    yuri->addSkill(new KongchengEffect);
    yuri->addSkill(new Hongyan);
    yuri->addSkill(new HongyanRetrial);
/*清流院琴音
归心，狂骨，聪慧，早夭*/
        sb1 = new General(this, "sb1", "yin",4,true);
        sb1->addSkill(new Guixin);
        sb1->addSkill(new Kuanggu);
	sb1->addSkill(new Zaoyao);
/*太田斧彦
神君，双雄，制衡*/
        sb2 = new General(this, "sb2", "yin",4,true);
        sb2->addSkill(new Shenjun);
	sb2->addSkill(new Shuangxiong);
        sb2->addSkill(new Zhiheng);
    General *sb2f = new General(this, "sb2f", "yin", 4, false, true);
    sb2f->addSkill("shenjun");
    sb2f->addSkill("shuangxiong");
    sb2f->addSkill("zhiheng");
/*丘菊之丞
猛进，天义，樵采*/
        sb3 = new General(this, "sb3", "yin",4,true);
	sb3->addSkill(new Tianyi);
	sb3->addSkill(new Mengjin);
        sb3->addSkill(new Qiaocai);

        General *mell, *ci, *poshui;
/*茜
完杀，毒士，看破*/
        ci = new General(this, "ci", "ba");
        ci->addSkill(new Dimeng);
        ci->addSkill(new Naughty);
        ci->addSkill(new Guiding);
/*梅尔
挥泪，龙胆，甘露*/
        mell = new General(this, "mell", "ba");
        mell->addSkill(new Ganlu);
        mell->addSkill(new Getitps);
        mell->addSkill(new Faner);
/*迫水典通
义从、神速、争功*/
        poshui = new General(this, "poshui", "god",4,true);
        poshui->addSkill(new Skill("yicong", Skill::Compulsory));
        poshui->addSkill(new Shensu);
        poshui->addSkill(new Zhenggong);
/*野野村蕾 围堰*/
        nonomura = new General(this, "nonomura", "yin");
        nonomura->addSkill(new Nono1);
        nonomura->addSkill(new Nono2);
        nonomura->addSkill(new LukangWeiyan);

    addMetaObject<GuicaiCard>();
    addMetaObject<GongxinCard>();
    addMetaObject<LexueCard>();
    addMetaObject<ShensuCard>();
    addMetaObject<RendeCard>();
    addMetaObject<DimengCard>();
    addMetaObject<TianyiCard>();
    addMetaObject<QiaocaiCard>();
    addMetaObject<GanluCard>();
    addMetaObject<ZhihengCard>();
}

ADD_PACKAGE(Othe)
