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

class Tieshan:public TriggerSkill{
public:
    Tieshan():TriggerSkill("tieshan"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->isNDTrick()){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            player->drawCards(1);
        }
        return false;
    }
};

CimuCard::CimuCard(){
    once = true;
}

bool CimuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;

    if(targets.isEmpty())
        return to_select->getKingdom() == "zhen" || to_select->getKingdom() == "woo";

    return false;
}

void CimuCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->acquireSkill(effect.to, "tieshan");
    effect.to->setMark("cimu", 1);
}

class CimuViewAsSkill: public OneCardViewAsSkill{
public:
    CimuViewAsSkill():OneCardViewAsSkill("cimu"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        QList<int> prime;
        prime << 2 << 3 << 5 << 7 << 11 << 13;
        return prime.contains(to_select->getFilteredCard()->getNumber());
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new CimuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("CimuCard");
    }
};

class Cimu: public PhaseChangeSkill{
public:
    Cimu():PhaseChangeSkill("cimu$"){
        view_as_skill = new CimuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            Room *room = player->getRoom();
            foreach(ServerPlayer *rtm, room->getAlivePlayers()){
                if(rtm->getMark("cimu") > 0){
                    room->detachSkillFromPlayer(rtm, "tieshan");
                    rtm->setMark("cimu", 0);
                }
            }
        }
        return false;
    }
};

RuoyuCard::RuoyuCard(){
}

bool RuoyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    if(to_select->isAllNude())
        return false;
    if(to_select == Self)
        return false;
    if(Self->distanceTo(to_select) > 1 &&
       !Self->hasSkill("shentou") && !Self->hasSkill("qicai"))
        return false;
    return true;
}

void RuoyuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->throwCard(this);
    room->loseHp(card_use.from);
    int card_id = getSubcards().first();
    Card::Suit suit = Sanguosha->getCard(card_id)->getSuit();
    int num = Sanguosha->getCard(card_id)->getNumber();

    CardUseStruct use;
    use.from = card_use.from;
    use.to = card_use.to;
    Snatch *sna = new Snatch(suit, num);
    sna->addSubcard(card_id);
    sna->setSkillName("ruoyu");
    use.card = sna;
    room->useCard(use);
}

class Ruoyu: public OneCardViewAsSkill{
public:
    Ruoyu():OneCardViewAsSkill("ruoyu"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        RuoyuCard *card = new RuoyuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

ZilianCard::ZilianCard(){
    target_fixed = true;
}

void ZilianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    if(source->isAlive())
        room->drawCards(source, subcards.length() + 1);
}

class ZilianViewAsSkill: public ViewAsSkill{
public:
    ZilianViewAsSkill():ViewAsSkill("zilian"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < Self->getHandcardNum();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zilian";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        ZilianCard *card = new ZilianCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zilian: public TriggerSkill{
public:
    Zilian():TriggerSkill("zilian"){
        events << Damaged << HpLost;
        view_as_skill = new ZilianViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(room->getCurrent() != player && !player->isNude())
            room->askForUseCard(player, "@@zilian", "@zilian");

        return false;
    }
};

class Lvbai: public TriggerSkill{
public:
    Lvbai():TriggerSkill("lvbai"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng())
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(!p->isKongcheng()){
                targets << p;
            }
        }
        if(!targets.isEmpty() && room->askForSkillInvoke(player, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            const Card *card = room->askForCard(player, ".", "@lvbai:" + target->objectName() + ":" + objectName());
            if(player->pindian(target, objectName(), card)){
                DamageStruct damage = data.value<DamageStruct>();
                damage.to = target;
                room->damage(damage);
                return true;
            }
        }
        return false;
    }
};

class Lvzhan: public TriggerSkill{
public:
    Lvzhan():TriggerSkill("lvzhan"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(player->isKongcheng() || damage.to == player || damage.to->isKongcheng())
            return false;
        Room *room = player->getRoom();

        if(room->askForSkillInvoke(player, objectName())){
            const Card *card = room->askForCard(player, ".", "@lvbai:" + damage.to->objectName() + ":" + objectName());
            if(player->pindian(damage.to, objectName(), card)){
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }
        return false;
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

    General *hattorishizuka = new General(this, "hattorishizuka$", "woo", 4, false);
    hattorishizuka->addSkill(new Tieshan);
    hattorishizuka->addSkill(new Cimu);
/*
    General *kudouyukiko = new General(this, "kudouyukiko", "yi", 3, false);
    General *kudouyuusaku = new General(this, "kudouyuusaku$", "yi", 3);
*/
    General *yamamuramisao = new General(this, "yamamuramisao", "jing", 3);
    yamamuramisao->addSkill(new Ruoyu);
    yamamuramisao->addSkill(new Zilian);

    General *otagiritoshirou = new General(this, "otagiritoshirou", "jing");
    otagiritoshirou->addSkill(new Skill("qinjian", Skill::Compulsory));

    General *suzukijirokichi = new General(this, "suzukijirokichi", "guai", 3);
    suzukijirokichi->addSkill(new Lvbai);
    suzukijirokichi->addSkill(new Lvzhan);
/*
    General *jiikounosuke = new General(this, "jiikounosuke", "guai", 3);
    General *chianti = new General(this, "chianti", "hei", 3, false);
    General *korn = new General(this, "korn", "hei");
    General *jamesblack = new General(this, "jamesblack$", "te");
*/
    General *hondoueisuke = new General(this, "hondoueisuke", "za");
    hondoueisuke->addSkill(new Mihu);
    hondoueisuke->addSkill(new Zhizhuo);

    addMetaObject<ChaidanCard>();
    addMetaObject<JulunCard>();
    addMetaObject<ConghuiCard>();
    addMetaObject<HongmengCard>();
    addMetaObject<CimuCard>();
    addMetaObject<RuoyuCard>();
    addMetaObject<ZilianCard>();
}

ADD_PACKAGE(Thicket)
