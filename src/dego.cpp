#include "dego.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "standard.h"

WushenSlash::WushenSlash(Card::Suit suit, int number)
    :Slash(suit, number)
{

}

class Wusheng:public OneCardViewAsSkill{
public:
    Wusheng():OneCardViewAsSkill("wusheng"){
    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(!card->isRed())
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow"){
            return ClientInstance->canSlashWithCrossbow();
        }else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        if (card->getSuit() == Card::Heart){
            WushenSlash *slash = new WushenSlash(card->getSuit(), card->getNumber());
            slash->addSubcard(card->getId());
            slash->setSkillName(objectName());
            return slash;
        } else
        {Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
    }
};

class Hurtslash:public MasochismSkill{
public:
    Hurtslash():MasochismSkill("hurtslash"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName())){
                room->playSkillEffect(objectName());
                room->askForUseCard(player, "slash", objectName());
            }
    }
};

class Guose: public OneCardViewAsSkill{
public:
    Guose():OneCardViewAsSkill("guose"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return (to_select->getCard()->getSuit() == Card::Diamond && !to_select->getFilteredCard()->inherits("TrickCard"));
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

LianliCard::LianliCard(){

}

bool LianliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
  if(!targets.isEmpty()){
      return false;
  }else {
      return to_select->getGeneral()->isMale() || (!to_select->getGeneral()->isMale() && to_select->getKingdom()=="ba");
  }}
//可以和男人或巴击女人

void LianliCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    LogMessage log;
    log.type = "#LianliConnection";
    log.from = effect.from;
    log.to << effect.to;
    room->sendLog(log);

    if(effect.from->getMark("@tied") == 0)
        effect.from->gainMark("@tied");

    if(effect.to->getMark("@tied") == 0){
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.from);
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0){
                player->loseMark("@tied");
                break;
            }
        }

        effect.to->gainMark("@tied");
    }
}

class LianliStart: public GameStartSkill{
public:
    LianliStart():GameStartSkill("#lianli-start") {

    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *player, players){
//            if(player->getGeneral()->isMale())
                room->attachSkillToPlayer(player, "lianli-slash");
        }
    }
};

LianliSlashCard::LianliSlashCard(){

}

bool LianliSlashCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void LianliSlashCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangfei = effect.from;
    Room *room = zhangfei->getRoom();

    ServerPlayer *xiahoujuan = room->findPlayerBySkillName("lianli");
    if(xiahoujuan){
        const Card *slash = room->askForCard(xiahoujuan, "slash", "@lianli-slash");
        if(slash){
            zhangfei->invoke("increaseSlashCount");
            room->cardEffect(slash, zhangfei, effect.to);
            return;
        }
    }
}

class LianliSlashViewAsSkill:public ZeroCardViewAsSkill{
public:
    LianliSlashViewAsSkill():ZeroCardViewAsSkill("lianli-slash"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@tied") > 0 && Slash::IsAvailable();
    }

    virtual const Card *viewAs() const{
        return new LianliSlashCard;
    }
};

class LianliSlash: public TriggerSkill{
public:
    LianliSlash():TriggerSkill("#lianli-slash"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@tied") > 0 && !target->hasSkill("lianli");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = player->getRoom();
        if(!player->askForSkillInvoke("lianli-slash", data))
            return false;

        ServerPlayer *xiahoujuan = room->findPlayerBySkillName("lianli");
        if(xiahoujuan){
            const Card *slash = room->askForCard(xiahoujuan, "slash", "@lianli-slash");
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class LianliJink: public TriggerSkill{
public:
    LianliJink():TriggerSkill("#lianli-jink"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@tied") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiahoujuan, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        if(!xiahoujuan->askForSkillInvoke("lianli-jink", data))
            return false;

        Room *room = xiahoujuan->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(xiahoujuan);
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0){
                ServerPlayer *zhangfei = player;

                const Card *jink = room->askForCard(zhangfei, "jink", "@lianli-jink");
                if(jink){
                    room->provide(jink);
                    return true;
                }

                break;
            }
        }

        return false;
    }
};

class LianliViewAsSkill: public ZeroCardViewAsSkill{
public:
    LianliViewAsSkill():ZeroCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@lianli";
    }

    virtual const Card *viewAs() const{
        return new LianliCard;
    }
};

class LianliClear: public TriggerSkill{
public:
    LianliClear():TriggerSkill("#lianli-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0)
                player->loseMark("@tied");
        }

        return false;
    }
};

class Lianli: public PhaseChangeSkill{
public:
    Lianli():PhaseChangeSkill("lianli$"){
        view_as_skill = new LianliViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            Room *room = target->getRoom();
            bool used = room->askForUseCard(target, "@lianli", "@@lianli-card");
            if(used){
//                if(target->getKingdom() != "shu")
//                    room->setPlayerProperty(target, "kingdom", "shu");
            }else{
//                if(target->getKingdom() != "wei")
//                    room->setPlayerProperty(target, "kingdom", "wei");

                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@tied") > 0)
                        player->loseMark("@tied");
                }
            }
        }

        return false;
    }
};

TianxiangCard::TianxiangCard()
{
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = effect.from->tag["TianxiangDamage"].value<DamageStruct>();
    damage.to = effect.to;
    damage.chain = true;
    room->damage(damage);

    if(damage.to->isAlive())
        damage.to->drawCards(damage.to->getLostHp());
}

class TianxiangViewAsSkill: public OneCardViewAsSkill{
public:
    TianxiangViewAsSkill():OneCardViewAsSkill("tianxiang"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@tianxiang";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->isEquipped() || to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TianxiangCard *card = new TianxiangCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Tianxiang: public TriggerSkill{
public:
    Tianxiang():TriggerSkill("tianxiang"){
        events << Predamaged;

        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(!xiaoqiao->isNude()){
            DamageStruct damage = data.value<DamageStruct>();
            Room *room = xiaoqiao->getRoom();

            xiaoqiao->tag["TianxiangDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(xiaoqiao, "@tianxiang", "@@tianxiang-card"))
                return true;
        }

        return false;
    }
};

class Lieren: public TriggerSkill{
public:
    Lieren():TriggerSkill("lieren"){
        events << GameStart << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhurong, QVariant &data) const{
        if(event == GameStart){
            zhurong->getRoom()->loseHp(zhurong);
        }
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !zhurong->isKongcheng() && !damage.to->isKongcheng() && damage.to != zhurong){
            Room *room = zhurong->getRoom();
            if(room->askForSkillInvoke(zhurong, objectName())){
                room->playSkillEffect(objectName(), 1);

                bool success = zhurong->pindian(damage.to, "lieren", NULL);
                if(success)
                    room->playSkillEffect(objectName(), 2);
                else{
                    room->playSkillEffect(objectName(), 3);
                    return false;
                }

                if(!damage.to->isNude()){
                    damage.to->turnOver();
                }
            }
        }

        return false;
    }
};
/*
class Undeadbird: public TriggerSkill{
public:
    Undeadbird():TriggerSkill("#undeadbird"){
        events << GameStart;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sumire, QVariant &data) const{
        Room *room = sumire->getRoom();
        QString busi = room->askForChoice(sumire,objectName(),"yes+no");
        if(busi=="yes"){
           sumire->getMark("@nirvana");
           room->loseHp(sumire);
        }
        return false;
    }
};
*/
class Niepan: public TriggerSkill{
public:
    Niepan():TriggerSkill("niepan"){
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *pangtong, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != pangtong)
            return false;
        Room *room = pangtong->getRoom();
        if(pangtong->askForSkillInvoke(objectName(), data)){
            room->broadcastInvoke("animate", "lightbox:$niepan");
            room->playSkillEffect(objectName());

            pangtong->loseMark("@nirvana");

            room->setPlayerProperty(pangtong, "hp", 3);
            pangtong->throwAllCards();
            pangtong->drawCards(5);

            if(pangtong->isChained()){
                if(dying_data.damage == NULL || dying_data.damage->nature == DamageStruct::Normal)
                    room->setPlayerProperty(pangtong, "chained", false);
            }
        }

        return false;
    }
};

class Liegong: public SlashBuffSkill{
public:
    Liegong():SlashBuffSkill("liegong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huangzhong = effect.from;
        Room *room = huangzhong->getRoom();
        if(room->getCurrent() != huangzhong)
            return false;

        int num = effect.to->getHandcardNum();
        if(num >= huangzhong->getHp() || num <= huangzhong->getAttackRange()){
            if(huangzhong->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                room->playSkillEffect(objectName());
                room->slashResult(effect, true);

                return true;
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

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "jink";
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << PhaseChange << CardUsed << CardResponsed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(event == PhaseChange){
            if(lumeng->getPhase() == Player::Start)
                lumeng->setMark("slash_count", 0);
        }else if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("Slash"))
                lumeng->addMark("slash_count");
        }else if(event == CardResponsed){
            CardStar card_star = data.value<CardStar>();
            if(card_star->inherits("Slash"))
                lumeng->addMark("slash_count");
        }

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
        if(lumeng->getPhase() == Player::Discard &&
           lumeng->getMark("slash_count") == 0 &&
           lumeng->askForSkillInvoke("keji"))
        {
            lumeng->getRoom()->playSkillEffect("keji");
            lumeng->skip(Player::Discard);

            return true;
        }

        return false;
    }
};

class Powl: public OneCardViewAsSkill{
public:
    Powl():OneCardViewAsSkill("powl"){
//K当桃园结义
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getNumber()== 13;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        GodSalvation *godsalvation = new GodSalvation(first->getSuit(), first->getNumber());
        godsalvation->addSubcard(first->getId());
        godsalvation->setSkillName(objectName());
        return godsalvation;
    }
};

class Jijiu: public OneCardViewAsSkill{
public:
    Jijiu():OneCardViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("peach") && Self->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Spade ||
                to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
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

HaoshiCard::HaoshiCard(){
    will_throw = false;
}

bool HaoshiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return to_select->getHandcardNum() == Self->getMark("haoshi");
}

void HaoshiCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *beggar = targets.first();

    room->moveCardTo(this, beggar, Player::Hand, false);
    room->setEmotion(beggar, "draw-card");
}

class HaoshiViewAsSkill: public ViewAsSkill{
public:
    HaoshiViewAsSkill():ViewAsSkill("haoshi"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        HaoshiCard *card = new HaoshiCard;        
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@haoshi!";
    }
};

class HaoshiGive: public PhaseChangeSkill{
public:
    HaoshiGive():PhaseChangeSkill("#haoshi-give"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *lusu) const{
        if(lusu->getPhase() == Player::Draw && lusu->hasFlag("haoshi")){
            lusu->setFlags("-haoshi");

            Room *room = lusu->getRoom();
            if(lusu->getHandcardNum() <= 5){
                room->playSkillEffect("haoshi");
                return false;
            }

            QList<ServerPlayer *> other_players = room->getOtherPlayers(lusu);
            int least = 1000;
            foreach(ServerPlayer *player, other_players)
                least = qMin(player->getHandcardNum(), least);
            room->setPlayerMark(lusu, "haoshi", least);
            bool used = room->askForUseCard(lusu, "@@haoshi!", "@haoshi");

            if(!used){
                // force lusu to give his half cards
                ServerPlayer *beggar = NULL;
                foreach(ServerPlayer *player, other_players){
                    if(player->getHandcardNum() == least){
                        beggar = player;
                        break;
                    }
                }

                int n = lusu->getHandcardNum()/2;
                QList<int> to_give = lusu->handCards().mid(0, n);
                HaoshiCard *haoshi_card = new HaoshiCard;
                foreach(int card_id, to_give)
                    haoshi_card->addSubcard(card_id);
                QList<ServerPlayer *> targets;
                targets << beggar;
                haoshi_card->use(room, lusu, targets);
                delete haoshi_card;
            }
        }

        return false;
    }
};

class Haoshi: public DrawCardsSkill{
public:
    Haoshi():DrawCardsSkill("#haoshi"){

    }

    virtual int getDrawNum(ServerPlayer *lusu, int n) const{
        Room *room = lusu->getRoom();
        if(room->askForSkillInvoke(lusu, "haoshi")){
            lusu->setFlags("haoshi");
            return n + 2;
        }else
            return n;
    }
};

JiemingCard::JiemingCard(){

}

bool JiemingCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    int upper = qMin(5, to_select->getMaxHP());
    return to_select->getHandcardNum() < upper;
}

void JiemingCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.to->getMaxHP() - effect.to->getHandcardNum();
    if(x <= 0)
        return;

    effect.to->drawCards(x);
}

class JiemingViewAsSkill: public ZeroCardViewAsSkill{
public:
    JiemingViewAsSkill():ZeroCardViewAsSkill("jieming"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@jieming";
    }

    virtual const Card *viewAs() const{
        return new JiemingCard;
    }
};

class Jieming: public MasochismSkill{
public:
    Jieming():MasochismSkill("jieming"){
        view_as_skill = new JiemingViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *xunyu, const DamageStruct &damage) const{
        Room *room = xunyu->getRoom();
        int x = damage.damage, i;
        for(i=0; i<x; i++){
            if(!room->askForUseCard(xunyu, "@@jieming", "@jieming"))
                break;
        }
    }
};

LeijiCard::LeijiCard(){

}

bool LeijiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void LeijiCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->setEmotion(target, "bad");

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade):(.*)");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 2;
        damage.from = zhangjiao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }else {
        judge.pattern = QRegExp("(.*):(club):(.*)");
        judge.good = false;
        judge.reason = "leiji";
        judge.who = target;
        if(judge.isBad()){
            DamageStruct damage;
            damage.card = NULL;
            damage.damage = 1;
            damage.from = zhangjiao;
            damage.to = target;
            damage.nature = DamageStruct::Thunder;

            room->damage(damage);
        }else
        room->setEmotion(zhangjiao, "bad");
    }
}

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponsed;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        Room *room = zhangjiao->getRoom();
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");

        return false;
    }
};

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
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

class Fankui:public MasochismSkill{
public:
    Fankui():MasochismSkill("fankui"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        int count = damage.damage;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)){
            int card_id;
          for(count ; count>0 && !from->isNude() ; count--){
            card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
            else room->obtainCard(simayi, card_id);
          }
            room->playSkillEffect(objectName());
        }
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

QiangxiCard::QiangxiCard(){
    once = true;
}

bool QiangxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(!subcards.isEmpty() && Self->getWeapon() == Sanguosha->getCard(subcards.first()))
        return Self->distanceTo(to_select) <= 1;

    return Self->inMyAttackRange(to_select);
}

void QiangxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(subcards.isEmpty())
        room->loseHp(effect.from);

    DamageStruct damage;
    damage.card = NULL;
    damage.from = effect.from;
    damage.to = effect.to;

    room->damage(damage);
}

class Qiangxi: public ViewAsSkill{
public:
    Qiangxi():ViewAsSkill("qiangxi"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("QiangxiCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return new QiangxiCard;
        else if(cards.length() == 1){
            QiangxiCard *card = new QiangxiCard;
            card->addSubcards(cards);

            return card;
        }else
            return NULL;
    }
};

class Qinyin: public TriggerSkill{
public:
    Qinyin():TriggerSkill("qinyin"){
        events << CardLost << PhaseChange;
        default_choice = "down";
    }

    void perform(ServerPlayer *shenzhouyu) const{
        Room *room = shenzhouyu->getRoom();
        QString result = room->askForChoice(shenzhouyu, objectName(), "up+down+eat");
        QList<ServerPlayer *> all_players = room->getAllPlayers();
        int vhs = result == "up"?1:(result=="down"?2:3);
        switch(vhs){
        case 1: {
                room->playSkillEffect(objectName(), 2);
                foreach(ServerPlayer *player, all_players){
                    RecoverStruct recover;
                    recover.who = shenzhouyu;
                    room->recover(player, recover);
                }break;
                }
        case 2: {
                foreach(ServerPlayer *player, all_players){
                    room->loseHp(player);
                }
                int index = 1;
                if(room->findPlayer("caocao+shencaocao+shencc"))
                    index = 3;
                room->playSkillEffect(objectName(), index);
                break;
            }
        case 3: {
                foreach(ServerPlayer *player, all_players){
                    player->drawCards(1);
                }break;
            }
        }
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shenzhouyu, QVariant &data) const{
        if(shenzhouyu->getPhase() != Player::Discard)
            return false;

        if(event == CardLost){
            CardMoveStruct move = data.value<CardMoveStruct>();
            if(move.to_place == Player::DiscardedPile){
                shenzhouyu->addMark("qinyin");
                if(shenzhouyu->getMark("qinyin") == 2){
                    if(shenzhouyu->askForSkillInvoke(objectName()))
                        perform(shenzhouyu);
                }
            }
        }else if(event == PhaseChange){
            shenzhouyu->setMark("qinyin", 0);
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

            room->doGuanxing(zhuge, room->getNCards(room->alivePlayerCount(), false), false);
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

            if(reason->inherits("Slash") || reason->inherits("Duel") || reason->inherits("FireAttack")){
                LogMessage log;
                log.type = "#LuoyiBuff";
                log.from = xuchu;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                xuchu->getRoom()->sendLog(log);
                damage.nature = DamageStruct::Thunder;
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

        Room *room = caocao->getRoom(); room->getAllPlayers();
        QList<ServerPlayer *> lieges = room->getSamesex("male", caocao);
        //QList<ServerPlayer *> lieges = room->getGeneral();
        //所有男性护驾
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->playSkillEffect(objectName());
        foreach(ServerPlayer *liege, lieges){
            QString result = room->askForChoice(liege, objectName(), "accept+ignore");
            if(result == "ignore")
                continue;

            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink");
            if(jink){
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class Slashtogether: public TriggerSkill{
public:
//    QString frient;
    Slashtogether():TriggerSkill("#slashtogether"){
       events << SlashProceed;
       default_choice = "no";
    }
    /*
    virtual void getFriend(const QString &me){
        if(me=="sakura") frient = "lihonglan";
        if(me=="sumire") frient="kanna";
        if(me=="maria") frient="lobelia";
        if(me=="alice") frient="reni";
        if(me=="lihonglan") frient="coquelicot";
        if(me=="kanna") frient="maria";
        if(me=="orhime") frient="hanabi";
        if(me=="reni") frient="orihime";
    }
    */
    virtual bool asked(ServerPlayer *f1,ServerPlayer *f2,ServerPlayer *f3)const{
        Room *room = f2->getRoom();
        if(f2!=f3 && f2->canSlash(f3)){
                if(f1->askForSkillInvoke(objectName())){
                    if (room->askForChoice(f2, "yoxi", "yes+no") == "yes")
                        return true;
                    else return false;
                }
            }
        else return false;
    }
    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
//        getFriend(player->getGeneralName());
        QString my=player->getGeneralName();
        QString myfriend="";
        if(my=="sakura") myfriend = "lihonglan";
        if(my=="sumire") myfriend="kanna";
        if(my=="maria") myfriend="lobelia";
        if(my=="alice") myfriend="reni";
        if(my=="lihonglan") myfriend="coquelicot";
        if(my=="kanna") myfriend="maria";
        if(my=="orhime") myfriend="hanabi";
        if(my=="reni") myfriend="orihime";
        if(my=="erica") myfriend = "sakura";
        if(my=="glycine") myfriend="sumire";
        if(my=="coquelicot") myfriend="alice";
        if(my=="lobelia") myfriend="erica";
        if(my=="hanabi") myfriend="glycine";

        if(!data.canConvert<SlashEffectStruct>() || myfriend=="")
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!player->isAlive())
            return false;
        ServerPlayer *player1 = effect.from;
        ServerPlayer *player2 = effect.to;
        Room *room = player1->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();

        foreach(ServerPlayer *p, players){
            if(p->getGeneralName() == myfriend)
            {player2 = p;
                break;}
        }
        if(asked(player1,player2,effect.to)){
                    room->playSkillEffect(objectName());

                    LogMessage log;
                    log.type = "#Slashok";
                    log.to << player1;
                    log.to << player2;
                    log.from = effect.to;
                    room->sendLog(log);

                    room->slashResult(effect, true);
                    return true;
                    }
        else  return false;
    }
};

DegoPackage::DegoPackage()
    :Package("dego")
{
    //帝国华击团·花组的女猪脚们
    General *sakura, *sumire, *maria, *alice, *lihonglan, *kanna, *orihime, *reni;
/*【樱花雾翔】（武圣）
【百花齐放】（国色）
【樱花天升】（连理）*/
    sakura = new General(this, "sakura$", "di");
    sakura->addSkill(new Slashtogether);
    sakura->addSkill(new Wusheng);
    sakura->addSkill(new Guose);
    sakura->addSkill(new Hurtslash);
    sakura->addSkill(new LianliStart);
    sakura->addSkill(new Lianli);
    sakura->addSkill(new LianliSlash);
    sakura->addSkill(new LianliJink);
    sakura->addSkill(new LianliClear);
//    sakura->addSkill(new Hujia);
/*【连雀之舞】（天香）
【蝴蝶之舞】（烈刃）
【凤凰之舞】（涅盘）*/
    sumire = new General(this, "sumire", "di");
    sumire->addSkill("#slashtogether");
    sumire->addSkill(new Tianxiang);
    sumire->addSkill(new Lieren);
//    sumire->addSkill(new Undeadbird);
    sumire->addSkill(new MarkAssignSkill("@nirvana", 1));
    sumire->addSkill(new Niepan);
/*【黑桃女皇】（烈弓）
【雪女】（倾国）
【胡桃夹子】（克己）*/
    maria = new General(this, "maria", "di");
    maria->addSkill("#slashtogether");
    maria->addSkill(new Liegong);
    maria->addSkill(new Qingguo);
    maria->addSkill(new Keji);
    maria->addSkill(new KejiSkip);
/*【漂亮的让保罗】（急救）
【超级让保罗】（洛神）
【爱丽丝长大成人】（好施）*/
    alice = new General(this, "alice", "di");
    alice->addSkill("#slashtogether");
    alice->addSkill(new Jijiu);
    alice->addSkill(new Powl); //桃园结义
    alice->addSkill(new Luoshen);
    alice->addSkill(new Haoshi);
    alice->addSkill(new HaoshiViewAsSkill);
    alice->addSkill(new HaoshiGive);
/*【雀牌机器人】（节命）
【球电机器人】（雷击）
【圣兽机器人】（奇袭）*/
    lihonglan = new General(this, "lihonglan", "di");
    lihonglan->addSkill("#slashtogether");
    lihonglan->addSkill(new Jieming);
    lihonglan->addSkill(new Leiji);
    lihonglan->addSkill(new Qixi);
/*【三进转掌】（连营）
【鹭牌五段】（咆哮）
【征远镇】（反馈）*/
    kanna = new General(this, "kanna", "di");
    kanna->addSkill("#slashtogether");
    kanna->addSkill(new Lianying);
    kanna->addSkill(new Skill("paoxiao"));
    kanna->addSkill(new Fankui);
/*【四季】（枭姬）
【野蔷薇】（强袭）
【魔笛】（琴音）*/
    orihime = new General(this, "orihime", "di");
    orihime->addSkill("#slashtogether");
    orihime->addSkill(new Xiaoji);
    orihime->addSkill(new Qiangxi);
    orihime->addSkill(new Qinyin);
/*【莱茵的黄金】（观星）
【女武神】（裸衣）
【齐格菲尔德】（奸雄）*/
    reni = new General(this, "reni", "di");
    reni->addSkill("#slashtogether");
    reni->addSkill(new Guanxing);
    reni->addSkill(new Luoyi);
    reni->addSkill(new LuoyiBuff);
    reni->addSkill(new Jianxiong);

    skills << new LianliSlashViewAsSkill;

    addMetaObject<LianliCard>();
    addMetaObject<LianliSlashCard>();
    addMetaObject<TianxiangCard>();
    addMetaObject<HaoshiCard>();
    addMetaObject<JiemingCard>();
    addMetaObject<LeijiCard>();
    addMetaObject<QiangxiCard>();
    addMetaObject<WushenSlash>();
}

ADD_PACKAGE(Dego)
