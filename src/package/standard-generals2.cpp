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

class Zhenxiang: public ZeroCardViewAsSkill{
   public:
   Zhenxiang():ZeroCardViewAsSkill("zhenxiang"){
//                default_choice = "discard";
            }
   virtual const Card *viewAs() const{
                return new ZhenxiangCard;
            }

            virtual bool isEnabledAtPlay() const{
                return !Self->hasUsed("ZhenxiangCard");
            }
        };

class Rexue:public MasochismSkill{
public:
    Rexue():MasochismSkill("rexue"){
    }

virtual void onDamaged(ServerPlayer *hatto, const DamageStruct &damage) const{
    Room *room = hatto->getRoom();
    if(!room->askForSkillInvoke(hatto, objectName())) return;
    QList<ServerPlayer *> Hurts;
    foreach(ServerPlayer *player, room->getOtherPlayers(hatto))
        if(player->isWounded())  Hurts << player;
        if (Hurts.length()<1) return;
        int x = damage.damage, i;
        for(i=0; i<x; i++){
            ServerPlayer *target = room->askForPlayerChosen(hatto, Hurts, "rexue");
            RecoverStruct recover;
            recover.card = NULL;
            recover.who = hatto;
            room->recover(target, recover);
        }
    }
};

class ChenshuiViewAsSkill: public ViewAsSkill{
public:
    ChenshuiViewAsSkill():ViewAsSkill("Chenshui"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    }

    virtual bool isEnabledAtResponse() const{
//        return ClientInstance->card_pattern.startsWith("@@Chenshui");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        SavageAssault *sa = new SavageAssault(Card::NoSuit, 0);
        sa->setSkillName("chenshui");
        return sa;
        }
};

class Chenshui: public PhaseChangeSkill{
public:
    Chenshui():PhaseChangeSkill("chenshui"){
        view_as_skill = new ChenshuiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *mourikogorou) const{
        Room *room = mourikogorou->getRoom();

        if(mourikogorou->getPhase() == Player::Start){
            if(room->askForUseCard(mourikogorou, "@@Chenshui", "@Chenshui")){
                mourikogorou->skip(Player::Judge);
                mourikogorou->skip(Player::Draw);
                mourikogorou->skip(Player::Play);
                return true;
            }
        }
        return false;
    }
};

class ZhinangClear: public TriggerSkill{
public:
    ZhinangClear():TriggerSkill("#zhinang-clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::NotActive)
            player->getRoom()->setTag("Zhinang", QVariant());

        return false;
    }
};

class Zhinang: public TriggerSkill{
public:
    Zhinang():TriggerSkill("zhinang"){
        events << CardLost;

        frequency = Frequent;
    }

virtual bool trigger(TriggerEvent, ServerPlayer *edogawaconan, QVariant &data) const{
        if(data.canConvert<CardMoveStruct>()){
            CardMoveStruct move = data.value<CardMoveStruct>();
            if(move.from_place != Player::Equip && edogawaconan->getPhase() == Player::NotActive){
                Room *room = edogawaconan->getRoom();
                if(room->askForSkillInvoke(edogawaconan, objectName())){
//                    room->playSkillEffect(objectName());
                    edogawaconan->drawCards(1);
                }
            }
        }

        return false;
    }
};

class ShiyanViewAsSkill: public OneCardViewAsSkill{
public:
    ShiyanViewAsSkill():OneCardViewAsSkill("shiyan"){

    }

    virtual bool isEnabledAtPlay() const{
        return !Self->hasUsed("ShiyanCard") && !Self->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ShiyanCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Shiyan: public PhaseChangeSkill{
public:
    Shiyan():PhaseChangeSkill("shiyan"){
        view_as_skill = new ShiyanViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
//        return false;
    }
};

class Lanman: public OneCardViewAsSkill{
public:
    Lanman():OneCardViewAsSkill("lanman"){

    }

    virtual bool isEnabledAtPlay(ServerPlayer *yoshidaayumi) const{
        bool trigger_this = false;
        Room *room = yoshidaayumi->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(yoshidaayumi);
            foreach(ServerPlayer *player, players){
                if(yoshidaayumi->getHp() > player->getHp()){
                    trigger_this = true;
                    break;
                 }}
 //       if(trigger_this = true){
        return trigger_this;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ExNihilo *exnihilo = new ExNihilo(first->getSuit(), first->getNumber());
        exnihilo->addSubcard(first->getId());
        exnihilo->setSkillName(objectName());
        return exnihilo;
    }
//  }
};

class DuorenViewAsSkill: public ZeroCardViewAsSkill{
public:
    DuorenViewAsSkill():ZeroCardViewAsSkill("duoren"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
//        return ClientInstance->card_pattern == "@@duoren";
    }

    virtual const Card *viewAs() const{
        return new DuorenCard;
    }
};

class Duoren: public TriggerSkill{
public:
    Duoren():TriggerSkill("duoren"){
        events << CardResponsed;
        view_as_skill = new DuorenViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *mouriran, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        Room *room = mouriran->getRoom();
        room->askForUseCard(mouriran, "@@duoren", "@duoren");

        return false;
    }
};

class Heqi: public TriggerSkill{
public:
    Heqi():TriggerSkill("heqi"){
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *mouriran, QVariant &data) const{
        return false;
    }
};

class Shenyong: public TriggerSkill{
public:
    Shenyong():TriggerSkill("shenyong"){
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *mouriran, QVariant &data) const{
        return false;
    }
};

class Shentou: public OneCardViewAsSkill{
public:
    Shentou():OneCardViewAsSkill("shentou"){

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
//                    room->playSkillEffect(objectName());
                    damage.damage ++;
                data = QVariant::fromValue(damage);
            }else{
                judge.pattern = QRegExp("(.*):(.*):([A])");
                judge.good = true;
                judge.reason = objectName();
                judge.who = aoyamagoushou;
             //   room->judge(judge);
                if(judge.isGood()){
                  damage.damage +=2;
                  data = QVariant::fromValue(damage);
              } else {aoyamagoushou->drawCards(1);return true;}
            }
            }
            return false;}
};




//conan
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
        foreach(ServerPlayer *liege, lieges){
            QString result = room->askForChoice(liege, objectName(), "accept+ignore");
            if(result == "ignore")
                continue;

            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink:" + caocao->objectName());
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
        int n = damage.damage * 2;
        guojia->drawCards(n);
        QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - n);

        while(room->askForYiji(guojia, yiji_cards))
            ; // empty loop
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

class LuoyiBuff: public TriggerSkill{
public:
    LuoyiBuff():TriggerSkill("#luoyi"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xuchu, QVariant &data) const{
        if(xuchu->hasFlag("luoyi")){
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
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@jijiang-slash");
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
                room->slashResult(effect, true);
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

class SuperGuanxing: public Guanxing{
public:
    SuperGuanxing():Guanxing(){
        setObjectName("super_guanxing");
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect("guanxing");

            room->doGuanxing(zhuge, room->getNCards(5, false), false);
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardMoveStruct move = data.value<CardMoveStruct>();
            if(move.from_place == Player::Hand)
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

        if(card->inherits("TrickCard") && !card->inherits("DelayedTrick")){
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
                room->playSkillEffect("jiuyuan", 1);
                break;
            }

        case CardEffected: {
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(effect.card->inherits("Peach") && effect.from->getKingdom() == "wu"
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
                lumeng->skip(Player::Discard);

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

    virtual bool trigger(TriggerEvent , ServerPlayer *luxun, QVariant &) const{
        if(luxun->isKongcheng()){
            Room *room = luxun->getRoom();
            if(room->askForSkillInvoke(luxun, objectName())){
                room->playSkillEffect(objectName());

                luxun->drawCards(1);
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

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return GameStartSkill::triggerable(target) && target->getGeneralName() == "sunshangxiang";
    }

    virtual void onGameStart(ServerPlayer *player) const{
        if(player->askForSkillInvoke(objectName())){
            player->getRoom()->transfigure(player, "sp_sunshangxiang", true, true);
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
        if(data.canConvert<CardMoveStruct>()){
            CardMoveStruct move = data.value<CardMoveStruct>();
            if(move.from_place == Player::Equip){
                Room *room = sunshangxiang->getRoom();
                if(room->askForSkillInvoke(sunshangxiang, objectName())){
                    room->playSkillEffect(objectName());
                    sunshangxiang->drawCards(2);
                }
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
        QString slasher = lubu->getGeneralName();
        bool jinked = false;
        room->playSkillEffect(objectName());
        const Card *first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher);
        if(first_jink && room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher))
            jinked = true;

        room->slashResult(effect, !jinked);

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

class Zhiba: public Zhiheng{
public:
    Zhiba(){
        setObjectName("zhiba");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("ZhihengCard") < (player->getLostHp() + 1);
    }
};

class Tuoqiao: public ZeroCardViewAsSkill{
public:
    Tuoqiao():ZeroCardViewAsSkill("tuoqiao"){
        huanzhuang_card = new HuanzhuangCard;
        huanzhuang_card->setParent(this);
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

void StandardPackage::addGenerals(){
	General *kudoushinichi, *hattoriheiji, *mourikogorou;
    kudoushinichi= new General(this,"kudoushinichi$","zhen");
    kudoushinichi->addSkill(new Zhenxiang);

    hattoriheiji= new General(this,"hattoriheiji","zhen",3);
    hattoriheiji->addSkill(new Rexue);

    mourikogorou= new General(this,"mourikogorou","zhen",3);
    mourikogorou->addSkill(new Chenshui);

    General *edogawaconan, *haibaraai, *yoshidaayumi;
    edogawaconan= new General(this,"edogawaconan$","shao",3);
    edogawaconan->addSkill(new Zhinang);
    edogawaconan->addSkill(new ZhinangClear);

    haibaraai= new General(this,"haibaraai","shao",3,false);
    haibaraai->addSkill(new Shiyan);

    yoshidaayumi= new General(this,"yoshidaayumi","shao",2,false);
    yoshidaayumi->addSkill(new Lanman);

    General *mouriran, *touyamakazuha, *kyougokumakoto;
    mouriran= new General(this,"mouriran","woo",4,false);
    mouriran->addSkill(new Duoren);

    touyamakazuha= new General(this,"touyamakazuha","woo",3,false);
    touyamakazuha->addSkill(new Heqi);

    kyougokumakoto= new General(this,"kyougokumakoto","woo");
    kyougokumakoto->addSkill(new Shenyong);

    General *kaitoukid, *sharon;
    kaitoukid= new General(this,"kaitoukid","yi",3);
    kaitoukid->addSkill(new Shentou);

    sharon= new General(this,"sharon","yi",3,false);
    sharon->addSkill(new Lijian);

    General *shiratorininzaburou, *matsumotokiyonaka, *otagiritoshirou;
    shiratorininzaburou= new General(this,"shiratorininzaburou","jing");
    shiratorininzaburou->addSkill(new Qingguo);

    matsumotokiyonaka= new General(this,"matsumotokiyonaka$","jing");
    matsumotokiyonaka->addSkill(new Jijiang);

    otagiritoshirou= new General(this,"otagiritoshirou","jing");
    otagiritoshirou->addSkill(new Guose);

    General *kurobakaitou, *nakamoriaoko;
    kurobakaitou= new General(this,"kurobakaitou","guai");
    kurobakaitou->addSkill(new Guanxing);

    nakamoriaoko= new General(this,"nakamoriaoko","guai",3,false);
    nakamoriaoko->addSkill(new Jizhi);

    General *gin, *vodka;
    gin= new General(this,"gin$","hei");
    gin->addSkill(new Zhiba);

    vodka= new General(this,"vodka","hei",3);
    vodka->addSkill(new Rende);

    General *akaishuichi;
    akaishuichi= new General(this,"akaishuichi","te");
    akaishuichi->addSkill(new Wushuang);

    General *agasahiroshi, *kobayashisumiko, *yamamuramisae, *aoyamagoushou;
    agasahiroshi= new General(this,"agasahiroshi$","za",3);
    agasahiroshi->addSkill(new Ganglie);

    kobayashisumiko= new General(this,"kobayashisumiko","za",3,false);
    kobayashisumiko->addSkill(new Jieyin);

    yamamuramisae= new General(this,"yamamuramisae","za",3,false);
    yamamuramisae->addSkill(new Yiji);

    aoyamagoushou= new General(this,"aoyamagoushou","za");
    aoyamagoushou->addSkill(new Long);



// conan

    // for skill cards
    addMetaObject<ZhihengCard>();
    addMetaObject<RendeCard>();
    addMetaObject<TuxiCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<KurouCard>();
    addMetaObject<LijianCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<GuicaiCard>();
    addMetaObject<QingnangCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<JijiangCard>();
    addMetaObject<HuanzhuangCard>();
    addMetaObject<CheatCard>();

    addMetaObject<ZhenxiangCard>();
}
