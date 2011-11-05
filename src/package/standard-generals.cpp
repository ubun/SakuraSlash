#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-skillcards.h"
#include "ai.h"

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

class Tishen: public TriggerSkill{
public:
    Tishen():TriggerSkill("tishen"){
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("fake") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *kaitou, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != kaitou)
            return false;

        Room *room = kaitou->getRoom();
        if(kaitou->askForSkillInvoke(objectName(), data)){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = kaitou;

            room->judge(judge);

            if(judge.isGood()){
                room->broadcastInvoke("animate", "lightbox:$tishen");
                kaitou->loseMark("fake");
                room->setPlayerProperty(kaitou, "hp", 3);
                return true;
            }
        }

        return false;
    }
};

class Moshu: public TriggerSkill{
public:
    Moshu():TriggerSkill("moshu"){
        events << PhaseChange << GameStart;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *kaitou = room->findPlayerBySkillName(objectName());
        if(!kaitou)
            return false;
        if(event == GameStart)
            kaitou->setMark("magic",1);
        if(player->getPhase() == Player::Draw && kaitou->getMark("magic")>0){
            if(player==kaitou)
                return false;
            QString choice = room->askForChoice(kaitou,objectName(),"zero+one+two");
            const Card *card;

            LogMessage log;
            log.type = "#Moshu";
            log.from = kaitou;
            log.arg2 = objectName();

            if(choice=="zero")
                return false;
            else if(choice=="one"){
                kaitou->drawCards(1);
                card = room->askForCard(kaitou,".","moshu-only");
                if(!card)
                    card = kaitou->getHandcards().first();
                room->moveCardTo(card, NULL, Player::DrawPile, true);

                log.arg = QString::number(1);
            }
            else{
                kaitou->drawCards(2);
                card = room->askForCard(kaitou,".","moshu-first");
                if(!card)
                    card = kaitou->getHandcards().first();
                room->moveCardTo(card, NULL, Player::DrawPile, true);
                card = room->askForCard(kaitou,".","moshu-second");
                if(!card)
                    card = kaitou->getHandcards().first();
                room->moveCardTo(card, NULL, Player::DrawPile, true);

                log.arg = QString::number(2);
            }
            room->sendLog(log);
            kaitou->setMark("magic",0);
         }
        else if(player==kaitou && player->getPhase() == Player::Start){
            kaitou->setMark("magic",1);
        }
        return false;
    }
};

class Renxing: public OneCardViewAsSkill{
public:
    Renxing():OneCardViewAsSkill("renxing"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("RenxingCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new RenxingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Qingmei: public TriggerSkill{
public:
    Qingmei():TriggerSkill("qingmei"){
        events << Pindian;
        frequency = Compulsory;
    }
    virtual int getPriority() const{
        return -1;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *aoko = room->findPlayerBySkillName(objectName());
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->from != aoko && pindian->to != aoko)
            return false;
        Card *pdcd;
        if(pindian->from == aoko && pindian->to_card->inherits("BasicCard")){
            pdcd = Sanguosha->cloneCard(pindian->to_card->objectName(), pindian->to_card->getSuit(), 1);
            pdcd->addSubcard(pindian->to_card);
            pdcd->setSkillName(objectName());
            pindian->to_card = pdcd;
        }
        else if(pindian->to == aoko && pindian->from_card->inherits("BasicCard")){
            pdcd = Sanguosha->cloneCard(pindian->from_card->objectName(), pindian->from_card->getSuit(), 1);
            pdcd->addSubcard(pindian->from_card);
            pdcd->setSkillName(objectName());
            pindian->from_card = pdcd;
        }

        LogMessage log;
        log.type = "#Qingmeieff";
        log.from = aoko;
        log.arg = objectName();
        room->sendLog(log);

        data = QVariant::fromValue(pindian);
        return false;
    }
};

class AnshaViewAsSkill: public ViewAsSkill{
public:
    AnshaViewAsSkill():ViewAsSkill("ansha"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@ansha") > 0;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{

        if(selected.length() >= 4)
            return false;
        if(to_select->isEquipped())
            return false;
        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getSuit() == item->getFilteredCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 4){
            AnshaCard *card = new AnshaCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Ansha: public PhaseChangeSkill{
public:
    Ansha():PhaseChangeSkill("ansha"){
        view_as_skill = new AnshaViewAsSkill;
        frequency = Limited;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("anshamark") > 0;
    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish){
            room->setPlayerMark(player, "anshamark", 0);
            ServerPlayer *gin = room->findPlayerBySkillName(objectName());
            if(gin){
                DamageStruct damage;
                damage.from = gin;
                damage.to = player;
                damage.damage = 3;
                room->setEmotion(gin, "good");
                room->loseMaxHp(gin);
                room->damage(damage);
            }
        }
        return false;
    }
};

class Juelu: public TriggerSkill{
public:
    Juelu():TriggerSkill("juelu"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gin, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()){
            Room *room = gin->getRoom();
            if(room->askForSkillInvoke(gin, objectName())){
                const Card *card = room->askForCard(gin, "slash", "juelu-slash");
                if(card){
                    // if player is drank, unset his flag
                    if(gin->hasFlag("drank"))
                        room->setPlayerFlag(gin, "-drank");
                    room->cardEffect(card, gin, damage.to);
                }
            }
        }
        return false;
    }
};

class Heiyi: public PhaseChangeSkill{
public:
    Heiyi():PhaseChangeSkill("heiyi$"){
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "hei";
    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Draw)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *gin = room->getLord();
        if(gin->hasLordSkill(objectName())){
            if(player != gin && player->askForSkillInvoke(objectName())){
                gin->addMark("@heiyi");
                return true;
            }
            else if(player == gin){
                for(; gin->getMark("@heiyi") > 0; gin->loseMark("@heiyi")){
                    gin->drawCards(2);
                }
            }
        }
        return false;
    }
};

class MaixiongViewAsSkill:public OneCardViewAsSkill{
public:
    MaixiongViewAsSkill():OneCardViewAsSkill("maixiong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MaixiongCard *card = new MaixiongCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Maixiong: public PhaseChangeSkill{
public:
    Maixiong():PhaseChangeSkill("maixiong"){
        view_as_skill = new MaixiongViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start)
            target->setMark("maixiong", 0);
        else if(target->getPhase() == Player::NotActive)
            target->getRoom()->setPlayerMark(target, "daoh", 0);
        return false;
    }
};

class Dashou: public TriggerSkill{
public:
    Dashou():TriggerSkill("dashou"){
        events << PhaseChange;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish && player->askForSkillInvoke(objectName(), data)){
            foreach(ServerPlayer *other , room->getOtherPlayers(player)){
                const Card *card = room->askForCard(other, ".", "@dashou-get:" + player->objectName(), false);
                if(card){
                    player->obtainCard(card);
                    player->addMark("dashou");
                }
            }
            if(player->getMark("dashou")>2)
                player->turnOver();
        }
        player->setMark("dashou",0);
        return false;
    }
};

class Jushen:public TriggerSkill{
public:
    Jushen():TriggerSkill("jushen"){
        frequency = Compulsory;
        events << CardUsed << SlashProceed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *akai, QVariant &data) const{
        Room *room = akai->getRoom();
        LogMessage log;
        log.arg = objectName();
        if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            log.type = "#Jushenslash";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);

            effect.from->getRoom()->slashResult(effect, NULL);
            return true;
        }
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
            if(use.to.length() == 0 || !akai->inMyAttackRange(use.to.first()))
                return false;

            if(card->inherits("Duel") || card->inherits("FireAttack")){
                DamageStruct damage;
                damage.from = akai;
                damage.to = use.to.first();
                if(card->inherits("FireAttack"))
                    damage.nature = DamageStruct::Fire;
                room->setEmotion(akai, "good");

                log.type = "#Jushenattack";
                log.from = damage.from;
                log.to << damage.to;
                room->sendLog(log);

                room->damage(damage);
                room->throwCard(use.card->getId());
                return true;
            }
        }
        return false;
    }
};

class Xunzhi: public TriggerSkill{
public:
    Xunzhi():TriggerSkill("xunzhi"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *shuichi, QVariant &data) const{
        QList<ServerPlayer *> targets;
        ServerPlayer *target;
        Room *room = shuichi->getRoom();

        DamageStar damage = data.value<DamageStar>();
        if(damage && damage->from && damage->to == shuichi && damage->from != shuichi)
            target = damage->from;
        else
            target = room->askForPlayerChosen(shuichi, room->getAlivePlayers(), objectName());
        targets << target;
        targets << room->askForPlayerChosen(shuichi, room->getOtherPlayers(target), objectName());

        foreach(target, targets){
            target->gainMark("@aka",1);
            target->acquireSkill("xunzhiresult");
        }
        return false;
    }
};

class Xunzhiresult:public TriggerSkill{
public:
    Xunzhiresult():TriggerSkill("#xunzhiresult"){
        events << DrawNCards;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@aka")>0;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        LogMessage log;
        log.type = "#Xunzhieffect";
        log.from = player;
        log.arg = "xunzhi";
        data = data.toInt() - 1;
        player->getRoom()->sendLog(log);
        return false;
    }
};

class Gaizao:public TriggerSkill{
public:
    Gaizao():TriggerSkill("gaizao"){
        events << PhaseChange;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Draw)
            return false;

        Room *room = player->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->hasEquip()){
                targets << tmp;
            }
        }
        if(targets.length()!=0 && room->askForSkillInvoke(player,objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player,targets,objectName());
            int card_id = room->askForCardChosen(player, target, "e", objectName());
            room->throwCard(card_id);
            player->drawCards(1);
            target->drawCards(1);
        }
        return false;
    }
};

class Suyuan: public TriggerSkill{
public:
    Suyuan():TriggerSkill("suyuan"){
        events << Predamage;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *agasa = room->findPlayerBySkillName(objectName());
        if(!agasa)
            return false;
        if((damage.from == agasa || damage.to == agasa) && room->askForSkillInvoke(agasa, objectName(), data)){
            damage.from = room->askForPlayerChosen(agasa, room->getOtherPlayers(damage.from), objectName());
            LogMessage log;
            log.type = "#SuyuanChange";
            log.from = agasa;
            log.to << damage.from;
            log.arg = objectName();
            room->sendLog(log);
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Baomu: public TriggerSkill{
public:
    Baomu():TriggerSkill("baomu$"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();

        if(player != dying_data.who || dying_data.who->getKingdom() != "shao")
            return false;
        Room *room = player->getRoom();
        ServerPlayer *cancer = room->findPlayerBySkillName(objectName());

        if(cancer && cancer->askForSkillInvoke(objectName())){
            const Card *recovcd = room->askForCard(cancer, ".S", objectName());
            if(!recovcd)
                return false;
            RecoverStruct recover;
            recover.who = cancer;
            recover.card = recovcd;
            room->recover(dying_data.who, recover);
        }
        return false;
    }
};

class Yuanding:public OneCardViewAsSkill{
public:
    Yuanding():OneCardViewAsSkill("yuanding"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YuandingCard *card = new YuandingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Qiniao: public TriggerSkill{
public:
    Qiniao():TriggerSkill("qiniao"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sumiko, QVariant &data) const{
        if(sumiko->getPhase() == Player::Discard)
           sumiko->setMark("qiniao", sumiko->getHandcardNum());
        if(sumiko->getPhase() == Player::Finish){
           if(sumiko->getMark("qiniao") - sumiko->getHandcardNum() < 2 && sumiko->askForSkillInvoke(objectName())){
               Room *room = sumiko->getRoom();
               ServerPlayer *target = room->askForPlayerChosen(sumiko,room->getAlivePlayers(),objectName());
               target->gainMark("@bird",1);
           }
        }
        return false;
    }
};

class QiniaoSkip: public TriggerSkill{
public:
    QiniaoSkip():TriggerSkill("#qiniaoskip"){
        events << PhaseChange;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@bird")>0;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start){
            player->skip(Player::Judge);
            player->loseMark("@bird");
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

class Long: public TriggerSkill{
public:
    Long():TriggerSkill("long"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *aoyamagoushou, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = aoyamagoushou->getRoom();
        if(room->askForSkillInvoke(aoyamagoushou, objectName())){
            room->playSkillEffect(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):([2357JK])");
            judge.good = true;
            judge.reason = objectName();
            judge.who = aoyamagoushou;

            room->judge(judge);
            if(judge.isGood()){
                damage.to->drawCards(2);
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }else{
                judge.pattern = QRegExp("(.*):(.*):([A])");
                judge.good = true;
                judge.reason = objectName();
                judge.who = aoyamagoushou;
                if(judge.isGood()){
                  damage.damage +=2;
                  data = QVariant::fromValue(damage);
                } else {
                    aoyamagoushou->drawCards(1);
                    return true;
                }
            }
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

    addMetaObject<ZhenxiangCard>();
    addMetaObject<JiaojinCard>();
    addMetaObject<MazuiCard>();
    addMetaObject<ShiyanCard>();
    addMetaObject<ShouqiuCard>();
    addMetaObject<BaiyiCard>();
    addMetaObject<DiaobingCard>();
    addMetaObject<RenxingCard>();
    addMetaObject<AnshaCard>();
    addMetaObject<MaixiongCard>();
    addMetaObject<YuandingCard>();
    addMetaObject<JingshenCard>();

    skills << new RexueEffect;
    patterns[".At"] = new AttackPattern;
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "uzumaki", "god", 5, true, true);
    new General(this, "haruno", "god", 5, false, true);
}

ADD_PACKAGE(Test)
