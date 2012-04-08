#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

ChaidanCard::ChaidanCard(){
    once = true;
}

bool ChaidanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->getJudgingArea().isEmpty();
}

void ChaidanCard::onEffect(const CardEffectStruct &effect) const{
    QList<int> days;
    foreach(const Card *card, effect.to->getJudgingArea())
        days << card->getId();
    if(days.isEmpty())
        return;

    Room *room = effect.to->getRoom();
    int day = -1;
    if(days.count() > 1){
        room->fillAG(days, effect.from);
        day = room->askForAG(effect.from, days, true, "chaidan");
        effect.from->invoke("clearAG");
    }
    if(day == -1)
        day = days.first();

    const Card *delayCard = qobject_cast<const DelayedTrick *>(Sanguosha->getCard(day));
    const Card *delaycard = DelayedTrick::CastFrom(delayCard);

    JudgeStruct judge;
    judge.reason = "chaidan";
    judge.who = effect.from;
    room->judge(judge);

    bool chai = false;
    if(delaycard->inherits("Lightning")){
        if(judge.card->getSuit() == Card::Spade &&
           judge.card->getNumber() <= 9 && judge.card->getNumber() >= 2)
            chai = true;
    }
    else if(delaycard->inherits("Indulgence")){
        if(judge.card->getSuit() != Card::Heart)
            chai = true;
    }
    else if(delaycard->inherits("SupplyShortage")){
        if(judge.card->getSuit() != Card::Club)
            chai = true;
    }

    if(chai){
        effect.from->obtainCard(judge.card);
        effect.from->obtainCard(delayCard);
    }
}

class Chaidan: public ZeroCardViewAsSkill{
public:
    Chaidan():ZeroCardViewAsSkill("chaidan"){

    }

    virtual bool isEnabledAtPlay(const Player *matsuda) const{
        return !matsuda->hasUsed("ChaidanCard");
    }

    virtual const Card *viewAs() const{
        return new ChaidanCard;
    }
};

JulunCard::JulunCard(){
    once = true;
    target_fixed = true;
}

void JulunCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@circle");
    foreach(ServerPlayer *tmp, room->getAlivePlayers()){
        if(!tmp->getNextAlive()->isNude())
            room->obtainCard(tmp, room->askForCardChosen(tmp, tmp->getNextAlive(), "he", "julun"));
        else{
            DamageStruct damage;
            damage.from = tmp;
            damage.to = tmp->getNextAlive();
            room->damage(damage);
        }
        if(tmp->getNextAlive() == source)
            break;
    }
}

class Julun: public ZeroCardViewAsSkill{
public:
    Julun():ZeroCardViewAsSkill("julun"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@circle") >= 1;
    }

    virtual const Card *viewAs() const{
        return new JulunCard;
    }
};

ConghuiCard::ConghuiCard(){
    will_throw = false;
    target_fixed = true;
}

void ConghuiCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->addToPile("caidian", this->getSubcards().first(), false);
}

class ConghuiViewAsSkill: public OneCardViewAsSkill{
public:
    ConghuiViewAsSkill():OneCardViewAsSkill("conghui"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@conghui";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ConghuiCard *card = new ConghuiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Conghui:public PhaseChangeSkill{
public:
    Conghui():PhaseChangeSkill("conghui"){
        view_as_skill = new ConghuiViewAsSkill;
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        QStringList allnums = player->tag.value("Caidian").toStringList();
        int i = qrand() % allnums.count();
        return allnums.at(i);
    }

    virtual bool onPhaseChange(ServerPlayer *mitsu) const{
        if(mitsu->getPhase() == Player::Play){
            Room *room = mitsu->getRoom();
            if(room->askForUseCard(mitsu, "@@conghui", "@conghui")){
                QStringList allnums;
                QList<ServerPlayer *> chooses;
                for(int i = 1; i <= 13; i++)
                    allnums << QString::number(i);
                int card_id = mitsu->getPile("caidian").first();
                QList<ServerPlayer *> players = room->getOtherPlayers(mitsu);
                if(mitsu->hasSkill("qingyi") && mitsu->askForSkillInvoke("qingyi")){
                    ServerPlayer *extra = room->askForPlayerChosen(mitsu, room->getOtherPlayers(mitsu), objectName());
                    if(extra->getKingdom() == "shao"){
                        players.removeOne(extra);
                        extra->setFlags("ConghuiGet");
                    }
                }
                foreach(ServerPlayer *target, players){
                    target->tag["Caidian"] = QVariant::fromValue(allnums);
                    QString num = room->askForChoice(target, objectName(), allnums.join("+"));
                    LogMessage log;
                    log.type = "#Caidian";
                    log.from = target;
                    log.arg = num;
                    room->sendLog(log);

                    target->setMark("conghui", num.toInt());
                    chooses << target;
                    allnums.removeOne(num);
                    target->tag.remove("Caidian");
                }
                int x = Sanguosha->getCard(card_id)->getNumber();
                bool jiangli = false;
                QList<ServerPlayer *> chengfa;
                for(int i = 0; i < 14; i++){
                    if(!jiangli){
                        foreach(ServerPlayer *t, chooses){
                            if(qAbs(t->getMark("conghui") - x) == i){
                                t->drawCards(3);
                                jiangli = true;
                            }
                        }
                    }
                    else{
                        foreach(ServerPlayer *t, chooses){
                            if(qAbs(t->getMark("conghui") - x) == i)
                                chengfa << t;
                        }
                    }
                }
                foreach(ServerPlayer *t, chengfa){
                    int r = chengfa.indexOf(t);
                    if(r > chengfa.length() / 2 - 1)
                        room->loseHp(t);
                }

                room->throwCard(card_id);
                foreach(ServerPlayer *tmp, room->getLieges("shao", mitsu)){
                    if(tmp->hasFlag("ConghuiGet")){
                        room->obtainCard(tmp, card_id);
                        tmp->setFlags("-ConghuiGet");
                        break;
                    }
                }

                return true;
            }
        }
        return false;
    }
};

class Qingyi: public TriggerSkill{
public:
    Qingyi():TriggerSkill("qingyi"){
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *, QVariant &) const{
        return false;
    }
};

HongmengCard::HongmengCard(){
    target_fixed = true;
}

void HongmengCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->throwCard(this);
    int card_id = getSubcards().first();
    Card::Suit suit = Sanguosha->getCard(card_id)->getSuit();
    int num = Sanguosha->getCard(card_id)->getNumber();

    CardUseStruct use;
    use.from = card_use.from;
    AmazingGrace *amazingGrace = new AmazingGrace(suit, num);
    amazingGrace->addSubcard(card_id);
    amazingGrace->setSkillName("hongmeng");
    use.card = amazingGrace;
    room->useCard(use);
}

class Hongmeng: public OneCardViewAsSkill{
public:
    Hongmeng():OneCardViewAsSkill("hongmeng"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("HongmengCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HongmengCard *card = new HongmengCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Loli: public TriggerSkill{
public:
    Loli():TriggerSkill("loli"){
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->isRed()){
            player->getRoom()->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();

            player->getRoom()->sendLog(log);
            return true;
        }
        return false;
    }
};
/*
HaoshiCard::HaoshiCard(){
    will_throw = false;
}

bool HaoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@haoshi!";
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

DimengCard::DimengCard(){
    once = true;
}

bool DimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
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

bool DimengCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void DimengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    // make sure a is front of b
    if(room->getFront(a, b) != a){
        qSwap(a, b);
        qSwap(n1, n2);
    }

    int diff = qAbs(n1 - n2);
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("DimengCard");
    }
};

class Luanwu: public ZeroCardViewAsSkill{
public:
    Luanwu():ZeroCardViewAsSkill("luanwu"){
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@chaos") >= 1;
    }
};

LuanwuCard::LuanwuCard(){
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@chaos");
    room->broadcastInvoke("animate", "lightbox:$luanwu");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
            room->cardEffect(this, source, player);
    }
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach(ServerPlayer *player, players){
        int distance = effect.to->distanceTo(player);
        distance_list << distance;

        nearest = qMin(nearest, distance);
    }

    QList<ServerPlayer *> luanwu_targets;
    int i;
    for(i=0; i<distance_list.length(); i++){
        if(distance_list.at(i) == nearest && effect.to->canSlash(players.at(i))){
            luanwu_targets << players.at(i);
        }
    }

    const Card *slash = NULL;
    if(!luanwu_targets.isEmpty() && (slash = room->askForCard(effect.to, "slash", "@luanwu-slash"))){
        ServerPlayer *to_slash;
        if(luanwu_targets.length() == 1)
            to_slash = luanwu_targets.first();
        else
            to_slash = room->askForPlayerChosen(effect.to, luanwu_targets, "luanwu");
        room->cardEffect(slash, effect.to, to_slash);
    }else
        room->loseHp(effect.to);
}

class Weimu: public ProhibitSkill{
public:
    Weimu():ProhibitSkill("weimu"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("TrickCard") && card->isBlack() && !card->inherits("Collateral");
    }
};

class Jiuchi: public OneCardViewAsSkill{
public:
    Jiuchi():OneCardViewAsSkill("jiuchi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());

        return analeptic;
    }
};

class Roulin: public TriggerSkill{
public:
    Roulin():TriggerSkill("roulin"){
        events << SlashProceed;

        frequency = Compulsory;
    }

    const Card *askForDoubleJink(ServerPlayer *player, const QString &reason) const{
        Room *room = player->getRoom();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(player, "jink", QString("@%1-jink-1").arg(reason));
        if(first_jink)
            second_jink = room->askForCard(player, "jink", QString("@%1-jink-2").arg(reason));

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
        }

        return jink;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) || target->getGeneral()->isFemale();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.from->hasSkill(objectName()) && effect.to->getGeneral()->isFemale()){
            // dongzhuo slash female
            ServerPlayer *dongzhuo = effect.from;
            ServerPlayer *female = effect.to;
            Room *room = dongzhuo->getRoom();

            room->playSkillEffect(objectName(), 1);

            room->slashResult(effect, askForDoubleJink(female, "roulin1"));
            return true;

        }else if(effect.from->getGeneral()->isFemale() && effect.to->hasSkill(objectName())){
            // female slash dongzhuo

            ServerPlayer *female = effect.from;
            ServerPlayer *dongzhuo = effect.to;
            Room *room = female->getRoom();

            room->playSkillEffect(objectName(), 2);
            room->slashResult(effect, askForDoubleJink(dongzhuo, "roulin2"));

            return true;
        }

        return false;
    }
};

class Benghuai: public PhaseChangeSkill{
public:
    Benghuai():PhaseChangeSkill("benghuai"){
        frequency = Compulsory;
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getMaxHP() >= player->getHp() + 2)
            return "maxhp";
        else
            return "hp";
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const{
        bool trigger_this = false;
        Room *room = dongzhuo->getRoom();

        if(dongzhuo->getPhase() == Player::Finish){
            QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
            foreach(ServerPlayer *player, players){
                if(dongzhuo->getHp() > player->getHp()){
                    trigger_this = true;
                    break;
                }
            }
        }

        if(trigger_this){
            QString result = room->askForChoice(dongzhuo, "benghuai", "hp+max_hp");

            room->playSkillEffect(objectName());
            room->setEmotion(dongzhuo, "bad");

            LogMessage log;
            log.from = dongzhuo;
            if(result == "hp"){
                log.type = "#BenghuaiLoseHp";
                room->sendLog(log);
                room->loseHp(dongzhuo);
            }else{
                log.type = "#BenghuaiLoseMaxHp";
                room->sendLog(log);
                room->loseMaxHp(dongzhuo);
            }
        }

        return false;
    }
};

class Baonue: public TriggerSkill{
public:
    Baonue():TriggerSkill("baonue$"){
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "qun";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> dongzhuos;
        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *p, players){
            if(p->hasLordSkill("baonue")){
                dongzhuos << p;
            }
        }

        foreach(ServerPlayer *dongzhuo, dongzhuos){
            QVariant who = QVariant::fromValue(dongzhuo);
            if(player->askForSkillInvoke(objectName(), who)){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = true;
                judge.reason = "baonue";
                judge.who = player;

                room->judge(judge);

                if(judge.isGood()){
                    room->playSkillEffect(objectName());

                    RecoverStruct recover;
                    recover.who = player;
                    room->recover(dongzhuo, recover);
                }
            }
        }

        return false;
    }
};
*/
//conan
class Guilin: public MasochismSkill{
public:
    Guilin():MasochismSkill("guilin"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        const Card *card = damage.card;
        if(room->obtainable(card, player) && card->getSubcards().length() < 2
            && room->askForSkillInvoke(player, objectName())){
            player->obtainCard(card);
            QList<ServerPlayer *> targets;
            if(damage.from && !damage.from->isNude())
                targets << damage.from;
            if(room->getCurrent() && !room->getCurrent()->isNude() && room->getCurrent() != damage.from)
                targets << room->getCurrent();
            if(!targets.isEmpty()){
                ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                int card2_id = room->askForCardChosen(player, target, "he", objectName());
                room->obtainCard(player, card2_id);
                if(card->getSuit() != Sanguosha->getCard(card2_id)->getSuit())
                    room->askForDiscard(player, objectName(), 1, false, true);
            }
        }
    }
};

class Mihu: public TriggerSkill{
public:
    Mihu():TriggerSkill("mihu"){
        frequency = Compulsory;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Play)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getSkillName() != "zhizhuo")
            return false;
        int frog = qrand() % 4;
        Room *room = player->getRoom();
        if(frog == 0){
            if(use.card->subcardsLength() > 0)
                room->throwCard(use.card);
            else
                room->throwCard(use.card->getId());

            LogMessage log;
            log.type = "#Mihu_cup";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            return true;
        }
        return false;
    }
};

class ZhizhuoViewAsSkill: public OneCardViewAsSkill{
public:
    ZhizhuoViewAsSkill():OneCardViewAsSkill("zhizhuo"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->property("Store").toInt() > -1;
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
        new_card->setSkillName("zhizhuo");
        return new_card;
    }
};

class Zhizhuo: public TriggerSkill{
public:
    Zhizhuo():TriggerSkill("zhizhuo"){
        view_as_skill = new ZhizhuoViewAsSkill;
        events << CardUsed << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(!player)
            return false;
        if(event == PhaseChange){
            if(player->getPhase() == Player::Start)
                player->getRoom()->setPlayerProperty(player, "Store", -1);
            return false;
        }
        CardUseStruct card = data.value<CardUseStruct>();
        if(card.card->subcardsLength() > 0 || card.card->getNumber() < 1)
            return false;
        if(card.card->isNDTrick() || card.card->inherits("BasicCard"))
            player->getRoom()->setPlayerProperty(player, "Store", card.card->getEffectiveId());

        return false;
    }
};

ThicketPackage::ThicketPackage()
    :Package("thicket")
{
    General *matsudajinpei = new General(this, "matsudajinpei", "zhen");
    matsudajinpei->addSkill(new Chaidan);
    matsudajinpei->addSkill(new Julun);
    matsudajinpei->addSkill(new MarkAssignSkill("@circle", 1));
    related_skills.insertMulti("julun", "#@circle-1");

    General *tsuburayamitsuhiko = new General(this, "tsuburayamitsuhiko", "shao");
    tsuburayamitsuhiko->addSkill(new Conghui);
    tsuburayamitsuhiko->addSkill(new Qingyi);

    General *tabuseharuna = new General(this, "tabuseharuna", "shao", 3, false);
    tabuseharuna->addSkill(new Hongmeng);
    tabuseharuna->addSkill(new Loli);

    General *shiratorininzaburou = new General(this, "shiratorininzaburou", "jing");
    shiratorininzaburou->addSkill(new Guilin);

    General *hondoueisuke = new General(this, "hondoueisuke", "za");
    hondoueisuke->addSkill(new Mihu);
    hondoueisuke->addSkill(new Zhizhuo);

    addMetaObject<ChaidanCard>();
    addMetaObject<JulunCard>();
    addMetaObject<ConghuiCard>();
    addMetaObject<HongmengCard>();
}

ADD_PACKAGE(Thicket)
