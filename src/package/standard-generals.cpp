#include "general.h"
#include "standard.h"
#include "standard-generals.h"
#include "standard-skillcards.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"

class XJQ: public TriggerSkill{
public:
    XJQ():TriggerSkill("xjq"){
        events << CardFinished;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *kudou, QVariant &data) const{
        Room *room = kudou->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash") && room->askForSkillInvoke(kudou, objectName(), data)){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = kudou;

            room->judge(judge);
            if(judge.isGood())
                room->askForUseCard(kudou, "slash", "@xjq");
        }
        return false;
    }
};

class JYT: public TriggerSkill{
public:
    JYT():TriggerSkill("jyt"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){
            Room *room = damage.to->getRoom();

            LogMessage log;
            log.type = "#JYTEffect";
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            room->sendLog(log);

            damage.damage = 0;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class WQ: public OneCardViewAsSkill{
public:
    WQ():OneCardViewAsSkill("wq"){
        frequency = Nirvana;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Diamond;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->isWeak() && pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("wq");

        return ncard;
    }
};

class FNF: public PhaseChangeSkill{
public:
    FNF():PhaseChangeSkill("fnf"){
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Draw && room->askForSkillInvoke(mouri, objectName())){
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = mouri;
            use.to << room->askForPlayerChosen(mouri, room->getAlivePlayers(), objectName());

            room->useCard(use);
            return true;
        }
        return false;
    }
};

class SQSJViewAsSkill: public OneCardViewAsSkill{
public:
    SQSJViewAsSkill():OneCardViewAsSkill("sqsj"){
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SQSJCard *card = new SQSJCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sqsj";
    }
};

class SQSJ:public TriggerSkill{
public:
    SQSJ():TriggerSkill("sqsj"){
        events << CardUsed;
        view_as_skill = new SQSJViewAsSkill;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yueying, QVariant &data) const{
        if(!yueying->isWeak())
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        if(card->inherits("Slash") && use.to.length() == 1){
            Room *room = yueying->getRoom();
            use.to.first()->setFlags("Sq1");
            if(room->askForUseCard(yueying, "@@sqsj", "@sqsj")){
                room->playSkillEffect(objectName());
                foreach(ServerPlayer *tmp, room->getOtherPlayers(use.to.first())){
                    if(tmp->hasFlag("Sqsj")){
                        tmp->setFlags("-Sqsj");
                        use.to << tmp;
                    }
                }
                data = QVariant::fromValue(use);
            }
            use.to.first()->setFlags("-Sq1");
        }
        return false;
    }
};

class ZZ:public TriggerSkill{
public:
    ZZ():TriggerSkill("zz"){
        events << DamageComplete;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *mori, QVariant &data) const{
        Room *room = mori->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from)
            return false;
        if(room->askForSkillInvoke(mori, objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = mori;

            room->judge(judge);
            if(judge.isGood())
                damage.from->setFlags("ShutUp");
        }
        return false;
    }
};

class TM:public MasochismSkill{
public:
    TM():MasochismSkill("tm"){
        frequency = Nirvana;
    }

    virtual void onDamaged(ServerPlayer *mori, const DamageStruct &damage) const{
        Room *room = mori->getRoom();
        if(!mori->isWeak())
            return;
        if(room->askForSkillInvoke(mori, objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = mori;

            room->judge(judge);
            if(judge.isGood()){
                foreach(ServerPlayer *tmp, room->getOtherPlayers(mori)){
                    if(mori->distanceTo(tmp) <= 1)
                        room->loseHp(tmp);
                }
            }
        }
    }
};

class SS: public TriggerSkill{
public:
    SS():TriggerSkill("ss"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isFemale();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xiang = room->findPlayerBySkillName(objectName());
        if(!xiang || !xiang->askForSkillInvoke(objectName()))
            return false;
        if(!room->askForDiscard(xiang, objectName(), 2, true, true))
            room->loseHp(xiang);
        return true;
    }
};

class EMFJ: public TriggerSkill{
public:
    EMFJ():TriggerSkill("emfj"){
        events << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getSuit() != Card::Club)
            return false;
        Room *room = lubu->getRoom();
        room->playSkillEffect(objectName());

        QString slasher = lubu->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@emfj-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, "jink", "@emfj-jink-2:" + slasher);

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

class DC: public ViewAsSkill{
public:
    DC():ViewAsSkill("dc"){
        frequency = Nirvana;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWeak();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        DCCard *card = new DCCard;
        card->addSubcards(cards);
        return card;
    }
};

class XZM: public OneCardViewAsSkill{
public:
    XZM():OneCardViewAsSkill("xzm"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class JM:public TriggerSkill{
public:
    JM():TriggerSkill("jm"){
        frequency = Nirvana;
        events << CardUsed << CardResponsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isWeak();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard") && card->isBlack()){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

class HQ:public TriggerSkill{
public:
    HQ():TriggerSkill("hq"){
        events << DamageDone << HpLost;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        int n;
        if(event == HpLost)
            n = data.toInt();
        else{
            DamageStruct damage = data.value<DamageStruct>();
            n = damage.damage;
        }
        if(n < 1)
            return false;
        Room *room = yueying->getRoom();
        for(int i = 0; i < n; i++){
            if(yueying->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.reason = objectName();
                judge.who = yueying;

                room->judge(judge);
                const Card *card = judge.card;
                bool canadd = false;
                foreach(int tmp, yueying->getPile("hq_pile")){
                    const Card *tmpcard = Sanguosha->getCard(tmp);
                    if(tmpcard->getSuit() == card->getSuit()){
                        canadd = true;
                        break;
                    }
                }
                if(!canadd){
                    yueying->addToPile("hq_pile", judge.card->getEffectiveId());
                    n --;
                }
            }
        }
        if(n < 1)
            return true;
        else if(event == HpLost)
            data = QVariant::fromValue(n);
        else{
            DamageStruct damage = data.value<DamageStruct>();
            damage.damage = n;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class JT: public OneCardViewAsSkill{
public:
    JT():OneCardViewAsSkill("jt"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

//lb

class HK: public TriggerSkill{
public:
    HK():TriggerSkill("hk"){
        events << CardDrawnDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *nicole = room->findPlayerBySkillName(objectName());
        if(nicole && nicole->askForSkillInvoke(objectName()))
            nicole->drawCards(1);
        return false;
    }
};

class TS: public TriggerSkill{
public:
    TS():TriggerSkill("ts"){
        events << CardEffected;
        frequency = Nirvana;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isWeak();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("AOE")){
            LogMessage log;
            log.from = player;
            log.type = "#TSNullify";
            log.arg = objectName();
            log.arg2 = effect.card->objectName();
            player->getRoom()->sendLog(log);

            return true;
        }
        return false;
    }
};

class YS: public OneCardViewAsSkill{
public:
    YS():OneCardViewAsSkill("ys"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach");
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

class CN: public OneCardViewAsSkill{
public:
    CN():OneCardViewAsSkill("cn"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ExNihilo *ex = new ExNihilo(first->getSuit(), first->getNumber());
        ex->addSubcard(first->getId());
        ex->setSkillName(objectName());
        return ex;
    }
};

class JDXJ: public ProhibitSkill{
public:
    JDXJ():ProhibitSkill("jdxj"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Indulgence");
    }
};

class SLX: public TriggerSkill{
public:
    SLX():TriggerSkill("slx"){
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        if(lubu->isWeak())
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = lubu->getRoom();
        if(lubu->isKongcheng() || !lubu->askForSkillInvoke(objectName()))
            return false;
        int nui = 0;
        while(room->askForDiscard(lubu, objectName(), 1, true))
            nui ++;
        QString slasher = lubu->objectName();

        const Card *first_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher);
        if(first_jink){
            for(int i = 0; i < nui ; i++){
                if(!room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher)){
                    room->slashResult(effect, NULL);
                    return true;
                }
            }
        }

        room->slashResult(effect, first_jink);
        return true;
    }
};

class BD: public PhaseChangeSkill{
public:
    BD():PhaseChangeSkill("bd"){
    }

    virtual bool onPhaseChange(ServerPlayer *ayumi) const{
        //Room *room = ayumi->getRoom();
        if(ayumi->getPhase() == Player::Finish && !ayumi->hasFlag("Bd")){
            if(ayumi->askForSkillInvoke(objectName())){
                ayumi->setFlags("Bd");
                ayumi->gainAnExtraTurn();
            }
        }
        return false;
    }
};

class HW: public OneCardViewAsSkill{
public:
    HW():OneCardViewAsSkill("hw"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(ClientInstance->getPattern() == "nullification")
            return to_select->getFilteredCard()->inherits("Jink") ||
                    to_select->getFilteredCard()->inherits("Slash");
        else
            return to_select->getFilteredCard()->inherits("Jink");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Slash(first->getSuit(), first->getNumber());
        if(ClientInstance->getPattern() == "nullification"){
            ncard = new Nullification(first->getSuit(), first->getNumber());
        }
        ncard->addSubcard(first);
        ncard->setSkillName("hw");

        return ncard;
    }
};

class BW: public TriggerSkill{
public:
    BW():TriggerSkill("bw"){
        events << CardLost;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *luxun, QVariant &data) const{
        if(!luxun->isWeak())
            return false;
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

class FX: public PhaseChangeSkill{
public:
    FX():PhaseChangeSkill("fx"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *mouri) const{
        Room *room = mouri->getRoom();
        if(mouri->getPhase() == Player::Start &&
           mouri->getHandcardNum() > mouri->getHp()){
            if(!room->askForDiscard(mouri, objectName(), 1, true))
                room->loseHp(mouri);
        }
        return false;
    }
};

class DLDViewAsSkill: public OneCardViewAsSkill{
public:
    DLDViewAsSkill():OneCardViewAsSkill("dld"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("dld") != 0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        int value = Self->getMark("dld");
        if(value == 1)
            return to_select->getFilteredCard()->isRed();
        else if(value == 2)
            return to_select->getFilteredCard()->isBlack();
        return false;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        DLDCard *card = new DLDCard;
        card->addSubcard(card_item->getCard());
        card->setSkillName(objectName());
        return card;
    }
};

class DLD: public TriggerSkill{
public:
    DLD():TriggerSkill("dld"){
        view_as_skill = new DLDViewAsSkill;
        frequency = Nirvana;
        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == PhaseChange){
            if(player->getPhase() == Player::Start){
                room->setPlayerMark(player, "dld", 0);
            }
            else if(player->getPhase() == Player::Draw){
                if(!player->isWeak())
                    return false;
                if(player->askForSkillInvoke(objectName())){
                    player->setFlags("dld");

                    JudgeStruct judge;
                    judge.pattern = QRegExp("(.*)");
                    judge.good = true;
                    judge.reason = objectName();
                    judge.who = player;

                    room->judge(judge);

                    room->setPlayerMark(player, "dld", judge.card->isRed() ? 1 : 2);
                    player->setFlags("-dld");

                    return true;
                }
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "dld"){
                player->obtainCard(judge->card);
                return true;
            }
        }
        return false;
    }
};

class DPH:public SlashBuffSkill{
public:
    DPH():SlashBuffSkill("dph"){
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *machao = effect.from;

        Room *room = machao->getRoom();
        if(effect.from->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());

            if(room->askForCard(machao, ".red", "@dph"))
                effect.from->setFlags("Dph");
        }
        return false;
    }
};

class LXHS: public TriggerSkill{
public:
    LXHS():TriggerSkill("lxhs"){
        events << Damage;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!zhurong->isWeak())
            return false;
        if(damage.card && damage.card->inherits("Slash")){
            Room *room = zhurong->getRoom();
            QList<ServerPlayer *>targets;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(damage.to))
                if(zhurong->inMyAttackRange(tmp))
                    targets << tmp;
            if(!targets.isEmpty() && room->askForSkillInvoke(zhurong, objectName(), data)){
                room->playSkillEffect(objectName(), 1);
                ServerPlayer *target = room->askForPlayerChosen(zhurong, targets, objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = false;
                judge.reason = objectName();
                judge.who = target;

                room->judge(judge);
                if(judge.isBad()){
                    DamageStruct dam;
                    dam.card = judge.card;
                    dam.from = zhurong;
                    dam.to = target;
                    room->damage(dam);
                }
            }
        }
        return false;
    }
};

class DCC: public TriggerSkill{
public:
    DCC():TriggerSkill("dcc"){
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

class JHZ:public MasochismSkill{
public:
    JHZ():MasochismSkill("jhz"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, objectName(), source)){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if(judge.isGood()){
                DamageStruct damage;
                damage.from = xiahou;
                damage.to = from;

                room->setEmotion(xiahou, "good");
                room->damage(damage);
            }else
                room->setEmotion(xiahou, "bad");
        }
    }
};

class JD: public OneCardViewAsSkill{
public:
    JD():OneCardViewAsSkill("jd"){
        frequency = Nirvana;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWeak();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Collateral *ollatera = new Collateral(first->getSuit(), first->getNumber());
        ollatera->addSubcard(first->getId());
        ollatera->setSkillName(objectName());
        return ollatera;
    }
};

class MX:public TriggerSkill{
public:
    MX():TriggerSkill("mx"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhenji, QVariant &data) const{
        if(event == PhaseChange && zhenji->getPhase() == Player::Finish){
            Room *room = zhenji->getRoom();
            while(zhenji->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                QString color = room->askForChoice(zhenji, objectName(), "mxr+mxb");
                JudgeStruct judge;
                judge.pattern = color == "mxb" ?
                                QRegExp("(.*):(spade|club):(.*)") :
                                QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = zhenji;

                room->judge(judge);
                if(judge.isGood())
                    zhenji->setFlags("getit");
                else{
                    zhenji->setFlags("-getit");
                    break;
                }
            }

        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(zhenji->hasFlag("getit")){
                    zhenji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
    }
};

class YZ: public OneCardViewAsSkill{
public:
    YZ():OneCardViewAsSkill("yz"){
        frequency = Nirvana;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(!Self->isWeak())
            return NULL;
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

class BCJAsSkill: public OneCardViewAsSkill{
public:
    BCJAsSkill():OneCardViewAsSkill("bcj"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@bcj";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;
        if(to_select->getCard()->getSuit() != Sanguosha->getCard(Self->property("Store").toInt())->getSuit())
            return false;
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *new_card = Sanguosha->cloneCard(Sanguosha->getCard(Self->property("Store").toInt())->objectName(), card->getSuit(), card->getNumber());
        new_card->addSubcard(card);
        new_card->setSkillName("bcj");
        return new_card;
    }
};

class BCJ: public TriggerSkill{
public:
    BCJ():TriggerSkill("bcj"){
        events << CardFinished;
        view_as_skill = new BCJAsSkill;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *s, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isNDTrick())
            return false;
        Room *room = s->getRoom();
        room->setPlayerProperty(s, "Store", use.card->getEffectiveId());
        room->askForUseCard(s, "@@bcj", "@bcj");
        return false;
    }
};

class GY: public TriggerSkill{
public:
    GY():TriggerSkill("gy"){
        events << SlashMissed;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(!player->isWeak())
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash && player->askForSkillInvoke(objectName()))
            player->obtainCard(effect.slash);
        return false;
    }
};

class BSJF: public WeaponSkill{
public:
    BSJF():WeaponSkill("bsjf"){
        events << SlashEffect;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(!player->isWeak())
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        effect.to->addMark("qinggang");
        return false;
    }
};

class ZQ: public SlashBuffSkill{
public:
    ZQ():SlashBuffSkill("zq"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gao = effect.from;
        if(gao->getWeapon())
            return false;
        Room *room = gao->getRoom();
        room->playSkillEffect(objectName());
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = gao;
        log.arg = objectName();
        room->sendLog(log);

        room->slashResult(effect, NULL);
        return true;
    }
};

class TK: public TriggerSkill{
public:
    TK():TriggerSkill("tk"){
        events << CardEffected;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(!player->isWeak())
            return false;
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#Xiangle";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);

            return !room->askForCard(effect.from, "slash", "@tk-discard", data);
        }
        return false;
    }
};

class PZ: public TriggerSkill{
public:
    PZ():TriggerSkill("pz"){
        events << SlashMissed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct slash_effect = data.value<SlashEffectStruct>();
        PlayerStar next = slash_effect.to->getNextAlive();
        if(next == player)
            next = next->getNextAlive();
        room->playSkillEffect(objectName());
        LogMessage log;
        log.type = "#PZEffect";
        log.from = player;
        log.to << next;
        log.arg = objectName();
        room->sendLog(log);

        slash_effect.to = next;
        room->slashEffect(slash_effect);
        return true;
    }
};

class HDY: public WeaponSkill{
public:
    HDY():WeaponSkill("hdy"){
        events << Predamage;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(!player->isWeak())
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash")){
            Room *room = damage.to->getRoom();
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(diamond):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = player;

            room->judge(judge);
            if(judge.isGood()){
                LogMessage log;
                log.type = "#HDYEffect";
                log.from = player;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                room->sendLog(log);

                damage.damage ++;
            }
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class QS: public TriggerSkill{
public:
    QS():TriggerSkill("qs"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(!killer || !killer->hasSkill(objectName()) || !damage->card->inherits("Slash"))
            return false;
        Room *room = killer->getRoom();
        if(killer->isAlive()){
            RecoverStruct tt;
            tt.who = killer;
            tt.recover = 2;
            room->recover(killer, tt);
        }
        return false;
    }
};

class SB: public ViewAsSkill{
public:
    SB():ViewAsSkill("sb"){
        frequency = Nirvana;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWeak();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return to_select->getCard()->isRed() == selected.last()->getCard()->isRed();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            SBCard *card = new SBCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class YLP: public TriggerSkill{
public:
    YLP():TriggerSkill("ylp"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gin, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
           damage.to->getNextAlive() != gin && gin->askForSkillInvoke(objectName())){
            Room *room = gin->getRoom();
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = gin;

            room->judge(judge);
            if(judge.isGood())
                room->cardEffect(damage.card, gin, damage.to->getNextAlive());
        }
        return false;
    }
};

class YYSD:public OneCardViewAsSkill{
public:
    YYSD():OneCardViewAsSkill("yysd"){
        frequency = Nirvana;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWeak();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YYSDCard *card = new YYSDCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class CKViewAsSkill:public OneCardViewAsSkill{
public:
    CKViewAsSkill():OneCardViewAsSkill("ck"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@ck";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() != Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new CKCard;
        card->setSuit(card_item->getFilteredCard()->getSuit());
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class CK: public TriggerSkill{
public:
    CK():TriggerSkill("ck"){
        view_as_skill = new CKViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@ck-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, "@ck", prompt, data);

        if(card){
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

            RecoverStruct dddddddd;
            dddddddd.who = player;
            room->recover(player, dddddddd);
        }
        return false;
    }
};

class HD: public ZeroCardViewAsSkill{
public:
    HD():ZeroCardViewAsSkill("hd"){

    }

    virtual const Card *viewAs() const{
        return new HDCard;
    }
};

class SJ: public TriggerSkill{
public:
    SJ():TriggerSkill("sj"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(!killer || !killer->hasSkill(objectName()) || !damage->card->inherits("Slash"))
            return false;
        if(killer->isAlive())
            killer->drawCards(2);
        return false;
    }
};
/*
class AS: public PhaseChangeSkill{
public:
    AS():PhaseChangeSkill("as"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(room->getCurrent()->hasSkill(objectName())){
            if;
        }
        return false;
    }
};
*/
class WQQViewAsSkill: public OneCardViewAsSkill{
public:
    WQQViewAsSkill():OneCardViewAsSkill("wqq"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@WQQ";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        WQQCard *card = new WQQCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class WQQ: public TriggerSkill{
public:
    WQQ():TriggerSkill("wqq"){
        events << Predamaged;
        view_as_skill = new WQQViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(!xiaoqiao->isKongcheng()){
            DamageStruct damage = data.value<DamageStruct>();
            Room *room = xiaoqiao->getRoom();

            xiaoqiao->tag["WQQDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(xiaoqiao, "@@WQQ", "@wqq"))
                return true;
        }

        return false;
    }
};

class TTGF: public ProhibitSkill{
public:
    TTGF():ProhibitSkill("ttgf"){
        frequency = Nirvana;
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Slash") || card->inherits("Duel"))
            return to->hasSkill(objectName()) && to->isWeak() &&
                    from->getGeneral()->isMale();
        else
            return false;
    }
};

class KSD: public TriggerSkill{
public:
    KSD():TriggerSkill("ksd"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *pangde, QVariant &data) const{
        if(pangde->getPhase() != Player::Play)
            return false;
        Room *room = pangde->getRoom();
        const Card *card = room->askForCard(pangde, ".red", "@ksd");
        ArcheryAttack *aa = new ArcheryAttack(card->getSuit(), card->getNumber());
        aa->setSkillName(objectName());
        aa->addSubcard(card);
        CardUseStruct use;
        use.card = aa;
        use.from = pangde;

        room->useCard(use);
        return false;
    }
};

class XY: public PhaseChangeSkill{
public:
    XY():PhaseChangeSkill("xy"){
        frequency = Nirvana;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->isWeak() && player->getPhase() == Player::Start){
           if(player->askForSkillInvoke(objectName())){
               Room *room = player->getRoom();
               player->skip(Player::Judge);
               player->skip(Player::Draw);
               ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
               target->drawCards(1);
               RecoverStruct uct;
               uct.who = player;
               room->recover(player, uct);
           }
        }
        return false;
    }
};

class MD: public TriggerSkill{
public:
    MD():TriggerSkill("md"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *caopi = room->findPlayerBySkillName(objectName());
        if(caopi && room->askForSkillInvoke(caopi, objectName(), data)){
            DummyCard *all_cards = player->wholeHandCards();
            if(all_cards){
                room->moveCardTo(all_cards, caopi, Player::Hand, false);
                delete all_cards;
            }
        }
        return false;
    }
};

class YTS: public TriggerSkill{
public:
    YTS():TriggerSkill("yts"){
        events << Predamaged;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(xiaoqiao->isWeak() && xiaoqiao->askForSkillInvoke(objectName())){
            Room *room = xiaoqiao->getRoom();
            if(room->askForCard(xiaoqiao, "..S", "@yts"))
                return true;
        }
        return false;
    }
};

class HH: public TriggerSkill{
public:
    HH():TriggerSkill("hh"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        bool caninvoke = player->hasSkill("hll") && player->isWeak() ?
                         effect.card->inherits("AOE") && effect.card->inherits("Duel") &&
                         effect.card->inherits("Snatch") && effect.card->inherits("Dismantlement") :
                         effect.card->inherits("Duel") && effect.card->inherits("Snatch") && effect.card->inherits("Dismantlement");
        if(!caninvoke)
            return false;
        Room *room = player->getRoom();
        if(!player->isKongcheng() && room->askForCard(player, ".black", "@hh")){
            QList<ServerPlayer *> players = effect.card->inherits("AOE")?
                                            room->getOtherPlayers(player) :room->getOtherPlayers(effect.from);
            effect.to = room->askForPlayerChosen(player, players, objectName());
            room->cardEffect(effect);
            return true;
        }
        return false;
    }
};

class SF: public ProhibitSkill{
public:
    SF():ProhibitSkill("sf"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Slash");
    }
};

class WL: public TriggerSkill{
public:
    WL():TriggerSkill("wl"){
        events << HpChanged;
        frequency = Nirvana;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isWeak())
            player->getRoom()->loseMaxHp(player, 2);
        return false;
    }
};

class XJ: public OneCardViewAsSkill{
public:
    XJ():OneCardViewAsSkill("xj"){
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

class YLZR: public TriggerSkill{
public:
    YLZR():TriggerSkill("ylzr"){
        events << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *toom = player->getRoom();
        ServerPlayer *me = toom->findPlayerBySkillName(objectName());
        if(!me)
            return false;
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->reason == "indulgence"){
           if(judge->isGood())
               me->drawCards(1);
           else{
               DamageStruct dmg;
               dmg.to = judge->who;
               dmg.from = NULL;
               toom->damage(dmg);
           }
        }
    }
};

class LH: public DistanceSkill{
public:
    LH():DistanceSkill("lh"){
        frequency = Nirvana;
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->isWeak() && to->hasSkill(objectName()) && !to->getDefensiveHorse() && !to->getOffensiveHorse())
            return +1;
        else
            return 0;
    }
};

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

void StandardPackage::addGenerals(){
    General *lufei = new General(this, "lufei", "red");
    lufei->addSkill(new XJQ);
    lufei->addSkill(new JYT);
    lufei->addSkill(new WQ);

    General *suolong = new General(this, "suolong", "red");
    suolong->addSkill(new FNF);
    suolong->addSkill(new SQSJ);

    General *weiwei = new General(this, "weiwei", "red", 3, false);
    weiwei->addSkill(new ZZ);
    weiwei->addSkill(new TM);

    General *xiangji = new General(this, "xiangji", "red");
    xiangji->addSkill(new SS);
    xiangji->addSkill(new EMFJ);
    xiangji->addSkill(new DC);

    General *namei = new General(this, "namei", "red", 3, false);
    namei->addSkill(new XZM);
    namei->addSkill(new JM);

    General *buluke = new General(this, "buluke", "red", 3);
    buluke->addSkill(new HQ);
    buluke->addSkill(new Skill("cs", Skill::Nirvana));

    General *fulanqi = new General(this, "fulanqi", "red");
    fulanqi->addSkill(new JT);
    fulanqi->addSkill(new Skill("lb"));

    General *luobin = new General(this, "luobin", "red", 3, false);
    luobin->addSkill(new HK);
    luobin->addSkill(new TS);

    General *qiaoba = new General(this, "qiaoba", "red", 3);
    qiaoba->addSkill(new YS);
    qiaoba->addSkill(new Skill("wny", Skill::Nirvana));

    General *wusuopu = new General(this, "wusuopu", "red", 3);
    wusuopu->addSkill(new CN);
    wusuopu->addSkill(new JDXJ);
    wusuopu->addSkill(new SLX);

    General *qingzhi = new General(this, "qingzhi", "blue", 3);
    qingzhi->addSkill(new BD);
    qiaoba->addSkill(new Skill("bhsd", Skill::Nirvana));

    General *zhantaowan = new General(this, "zhantaowan", "blue", 3);
    zhantaowan->addSkill(new HW);
    zhantaowan->addSkill(new BW);

    General *maizelun = new General(this, "maizelun", "blue");
    maizelun->addSkill(new FX);
    maizelun->addSkill(new DLD);

    General *chiquan = new General(this, "chiquan", "blue");
    chiquan->addSkill(new DPH);
    chiquan->addSkill(new LXHS);

    General *dasiqi = new General(this, "dasiqi", "blue", 3, false);
    dasiqi->addSkill(new DCC);
    dasiqi->addSkill(new JHZ);
    dasiqi->addSkill(new JD);

    General *kebi = new General(this, "kebi", "blue", 3);
    kebi->addSkill(new MX);
    kebi->addSkill(new YZ);

    General *huangyuan = new General(this, "huangyuan", "blue");
    huangyuan->addSkill(new BCJ);
    huangyuan->addSkill(new GY);

    General *simoge = new General(this, "simoge", "blue");
    simoge->addSkill(new Skill("bwn", Skill::Compulsory));
    simoge->addSkill(new Skill("yg", Skill::Compulsory));
    simoge->addSkill(new BSJF);

    General *luqi = new General(this, "luqi", "blue");
    luqi->addSkill(new ZQ);
    luqi->addSkill(new TK);

    General *yingyan = new General(this, "yingyan", "purple");
    yingyan->addSkill(new PZ);
    yingyan->addSkill(new HDY);

    General *keluokedaer = new General(this, "keluokedaer", "purple");
    keluokedaer->addSkill(new QS);
    keluokedaer->addSkill(new SB);

    General *xiong = new General(this, "xiong", "purple");
    xiong->addSkill(new YLP);
    xiong->addSkill(new YYSD);

    General *duofulangminge = new General(this, "duofulangminge", "purple", 3);
    duofulangminge->addSkill(new CK);
    duofulangminge->addSkill(new Skill("hm", Skill::Nirvana));

    General *diqi = new General(this, "diqi", "purple", 3);
    diqi->addSkill(new HD);
    diqi->addSkill(new SJ);
    diqi->addSkill(new Skill("as", Skill::Nirvana));

    General *hankuke = new General(this, "hankuke", "purple", 3, false);
    hankuke->addSkill(new WQQ);
    hankuke->addSkill(new TTGF);

    General *shenping = new General(this, "shenping", "purple");
    shenping->addSkill(new KSD);
    shenping->addSkill(new XY);

    General *moliya = new General(this, "moliya", "purple", 3);
    moliya->addSkill(new MD);
    moliya->addSkill(new Skill("zs"));
    moliya->addSkill(new YTS);

    General *lita = new General(this, "lita", "yellow", 4, false);
    lita->addSkill(new HH);
    lita->addSkill(new Skill("hll", Skill::Nirvana));

    General *baji = new General(this, "baji", "yellow", 3);
    baji->addSkill(new SF);
    baji->addSkill(new WL);

    General *mrkr = new General(this, "mrkr", "yellow");
    mrkr->addSkill(new Skill("ry"));
    mrkr->addSkill(new Skill("yr"));
    mrkr->addSkill(new Skill("yq"));

    General *peiluola = new General(this, "peiluola", "yellow", 3, false);
    peiluola->addSkill(new XJ);
    peiluola->addSkill(new YLZR);
    peiluola->addSkill(new LH);

    General *yiwankefu = new General(this, "yiwankefu", "yellow", 3);
    yiwankefu->addSkill(new Skill("ryw"));
    yiwankefu->addSkill(new Skill("js"));
    yiwankefu->addSkill(new Skill("qljs"));

    General *jide = new General(this, "jide", "yellow");
    //jide->addSkill(new JX);
    //jide->addSkill(new CL);
    //jide->addSkill(new JXH);

    General *luo = new General(this, "luo", "yellow", 3);
    //luo->addSkill(new LY);
    //luo->addSkill(new MZ);

    General *ainilu = new General(this, "ainilu", "yellow", 3);
    //ainilu->addSkill(new FD);
    //ainilu->addSkill(new DL);
    //ainilu->addSkill(new WL);

    // for skill cards
    addMetaObject<SQSJCard>();
    addMetaObject<DCCard>();
    addMetaObject<DLDCard>();
    addMetaObject<SBCard>();
    addMetaObject<YYSDCard>();
    addMetaObject<HDCard>();
    addMetaObject<WQQCard>();

    addMetaObject<CheatCard>();
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "uzumaki", "god", 5, true, true);
    new General(this, "haruno", "god", 5, false, true);
}

ADD_PACKAGE(Test)
