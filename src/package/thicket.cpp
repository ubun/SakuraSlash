#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

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
    source->addToPile("caidian", getSubcards().first(), false);
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
                    ServerPlayer *extra = room->askForPlayerChosen(mitsu, room->getOtherPlayers(mitsu), "qingyi");
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
                LogMessage log;
                log.type = "#CaidianShow";
                log.to << mitsu;
                log.arg = QString::number(x);
                room->sendLog(log);

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
    mute = true;
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
            player->playSkillEffect(objectName());

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
            room->playSkillEffect(objectName());
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

LuanzhenCard::LuanzhenCard(){
}

bool LuanzhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select != Self;
}

void LuanzhenCard::onEffect(const CardEffectStruct &effect) const{
    QStringList skillist;
    Room *room = effect.from->getRoom();
    foreach(const SkillClass *skill, effect.to->getVisibleSkillList()){
        if(skill->getLocation() == Skill::Right &&
           skill->getFrequency() != Skill::Limited &&
           skill->getFrequency() != Skill::Wake &&
           !skill->isLordSkill()){
            skillist << skill->objectName();
        }
    }
    if(!skillist.isEmpty()){
        QString ski = room->askForChoice(effect.from, "luanzhen", skillist.join("+"));
        room->acquireSkill(effect.from, ski);
        effect.from->tag["Luanzhen"] = QVariant::fromValue(ski);
    }
}

class LuanzhenViewAsSkill: public ZeroCardViewAsSkill{
public:
    LuanzhenViewAsSkill():ZeroCardViewAsSkill("luanzhen"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@luanzhen";
    }

    virtual const Card *viewAs() const{
        return new LuanzhenCard;
    }
};

class Luanzhen: public PhaseChangeSkill{
public:
    Luanzhen():PhaseChangeSkill("luanzhen"){
        view_as_skill = new LuanzhenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            Room *room = player->getRoom();
            QString lzskill = player->tag["Luanzhen"].toString();
            room->detachSkillFromPlayer(player, lzskill);
            room->askForUseCard(player, "@@luanzhen", "@luanzhen");
        }
        return false;
    }
};

class Yinv:public TriggerSkill{
public:
    Yinv():TriggerSkill("yinv"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *yinv, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        Room *room = yinv->getRoom();
        if(use.card->inherits("SingleTargetTrick")
            && !use.card->inherits("Collateral")
            && !use.card->inherits("Nullification")){
            if(use.to.isEmpty())
                use.to << use.from;

            QList<ServerPlayer *> extras;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(use.to.first())){
                if(yinv->hasSkill("jaguarE")){
                    extras << tmp;
                    continue;
                }
                if(use.from->inMyAttackRange(tmp))
                    extras << tmp;
            }

            if(extras.isEmpty() || use.to.length() != 1 || !yinv->askForSkillInvoke(objectName(), data))
                return false;
            ServerPlayer *extra = room->askForPlayerChosen(yinv, extras, objectName());
            use.to << extra;

            data = QVariant::fromValue(use);
            return false;
        }
        return false;
    }
};

class Anye: public ProhibitSkill{
public:
    Anye():ProhibitSkill("anye"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return to->getPhase() == Player::NotActive && card->inherits("TrickCard") && card->isRed() && !card->inherits("Collateral");
    }
};

class Panguan: public TriggerSkill{
public:
    Panguan():TriggerSkill("panguan"){
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        if(player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
            room->obtainCard(player, judge->card);
            room->playSkillEffect(objectName());
            int card_id = room->drawCard();
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
            room->getThread()->delay();

            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->sendJudgeResult(judge);
            return true;
        }
        return false;
    }
};

FenbiCard::FenbiCard(){
    once = true;
    target_fixed = true;
}

void FenbiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@pen");
    QString name = Sanguosha->getCard(getSubcards().first())->objectName();
    room->throwCard(this);
    foreach(ServerPlayer *tmp, room->getOtherPlayers(source)){
        const Card *card = room->askForCard(tmp, "..", "@fenbi:" + source->objectName() + ":" + name);
        if(!card || card->objectName() != name)
            room->loseHp(tmp);
    }
}

class Fenbi: public OneCardViewAsSkill{
public:
    Fenbi():OneCardViewAsSkill("fenbi"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@pen") > 0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FenbiCard *card = new FenbiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Wuji: public TriggerSkill{
public:
    Wuji():TriggerSkill("wuji$"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage < 1 || !player->hasLordSkill(objectName()))
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> kudous;
        QStringList names;
        names << "kudoushinichi" << "edogawaconan" << "kudouyuusaku" << "kudouyukiko";
        foreach(ServerPlayer *kudou, room->getOtherPlayers(player)){
            if(kudou->getKingdom() == "yi" || names.contains(kudou->getGeneralName()))
                kudous << kudou;
        }

        if(kudous.isEmpty() || !player->askForSkillInvoke(objectName()))
            return false;
        foreach(ServerPlayer *kudou, kudous){
            if(room->askForCard(kudou, "Slash", "@wuji:" + player->objectName(), data))
                return true;
        }
        return false;
    }
};

RuoyuCard::RuoyuCard(){
    mute = true;
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
            room->playSkillEffect(objectName());
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

        if(room->askForSkillInvoke(player, objectName(), data)){
            room->playSkillEffect(objectName());
            const Card *card = room->askForCard(player, ".", "@lvbai:" + damage.to->objectName() + ":" + objectName());
            if(player->pindian(damage.to, objectName(), card)){
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Zhongpu:public MasochismSkill{
public:
    Zhongpu():MasochismSkill("zhongpu"){
    }

    virtual void onDamaged(ServerPlayer *jii, const DamageStruct &damage) const{
        Room *room = jii->getRoom();
        if(damage.damage < 1)
            return;
        if(jii->askForSkillInvoke(objectName())){
            ServerPlayer *target = room->askForPlayerChosen(jii, room->getOtherPlayers(jii), objectName());
            room->playSkillEffect(objectName());
            QString to = room->askForChoice(jii, objectName(), "draw+recover");
            if(to == "draw")
                target->drawCards(2);
            else{
                RecoverStruct r;
                r.who = jii;
                room->recover(target, r, true);
            }
            room->setTag("ZProject", jii->objectName());
        }
    }
};

class ZhongpuEffect: public TriggerSkill{
public:
    ZhongpuEffect():TriggerSkill("#zhongpu-effect"){
        events << PhaseChange << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        if(event == Predamaged){
            QString zp = room->getTag("ZProject").toString();
            return player->hasSkill("zhongpu") && player->objectName() == zp;
        }
        if(player->getPhase() == Player::NotActive)
            room->removeTag("ZProject");

        return false;
    }
};

ZhiquCard::ZhiquCard(){
    once = true;
}

bool ZhiquCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->canSlash(Self);
}

void ZhiquCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    const Card *slash = room->askForCard(effect.to, "slash", "@zhiqu-slash:" + effect.from->objectName());
    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.to << effect.from;
        use.from = effect.to;
        room->useCard(use);
    }else if(!effect.to->isNude())
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", skill_name));
}

class Zhiqu: public ZeroCardViewAsSkill{
public:
    Zhiqu():ZeroCardViewAsSkill("zhiqu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhiquCard");
    }

    virtual const Card *viewAs() const{
        return new ZhiquCard;
    }
};

class Shanjing:public SlashBuffSkill{
public:
    Shanjing():SlashBuffSkill("shanjing"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *chianti = effect.from;

        Room *room = chianti->getRoom();
        if(chianti->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());
            QString ichi, me;

            JudgeStruct judge;
            judge.reason = "shanjing1";
            judge.who = chianti;
            room->judge(judge);
            ichi = judge.card->getSuitString();
            me = judge.card->getNumberString();

            judge.reason = "shanjing2";
            room->judge(judge);

            if(me != judge.card->getNumberString() && ichi != judge.card->getSuitString()){
                LogMessage log;
                log.type = "#Shanjing";
                log.from = chianti;
                log.to << effect.to;
                log.arg = objectName();
                room->sendLog(log);

                room->slashResult(effect, NULL);
                return true;
            }
            else{
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->slashResult(effect, jink);
                return true;
            }
        }
        return false;
    }
};

class Mangju:public SlashBuffSkill{
public:
    Mangju():SlashBuffSkill("mangju"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *korn = effect.from;

        Room *room = korn->getRoom();
        if(korn->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.reason = objectName();
            judge.pattern = "BasicCard|.|.|.|black";
            judge.good = true;
            judge.who = korn;
            room->judge(judge);

            if(judge.isGood()){
                LogMessage log;
                log.type = "#Shanjing";
                log.from = korn;
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

AnyongCard::AnyongCard(){
    target_fixed = true;
}

void AnyongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    PlayerStar target = room->getCurrent();
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
    judge.good = false;
    judge.reason = "anyong";
    judge.who = source;
    room->judge(judge);
    if(judge.isGood())
        target->swap2Phases(Player::Judge, Player::Discard);
}

class AnyongViewAsSkill:public OneCardViewAsSkill{
public:
    AnyongViewAsSkill():OneCardViewAsSkill("anyong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        AnyongCard *card = new AnyongCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Anyong: public PhaseChangeSkill{
public:
    Anyong():PhaseChangeSkill("anyong"){
        view_as_skill = new AnyongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() != Player::RoundStart)
            return false;
        ServerPlayer *jams = room->findPlayerBySkillName(objectName());
        if(jams && jams != player && !jams->isKongcheng())
            room->askForUseCard(jams, "@@anyong", "@anyong:" + player->objectName());
        return false;
    }
};

class Panda: public TriggerSkill{
public:
    Panda():TriggerSkill("panda$"){
        events << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();
        if(!judge->who->hasLordSkill(objectName()) && judge->who->getKingdom() != "hei")
            return false;

        if(player->askForSkillInvoke(objectName(), data)){
            QList<ServerPlayer *> lieges;
            foreach(ServerPlayer *liege, room->getAllPlayers()){
                if(liege->getKingdom() == "te")
                    lieges << liege;
            }
            const Card *jud = NULL;
            ServerPlayer *who = NULL;
            foreach(ServerPlayer *liege, lieges){
                const Card *judo = room->askForCard(liege, ".", "@panda:" + player->objectName(), QVariant::fromValue((PlayerStar)player));
                if(judo){
                    who = liege;
                    jud = judo;
                }
            }
            if(!jud)
                return false;
            room->getThread()->delay();
            room->throwCard(judge->card);

            judge->card = jud;
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = who;
            log.to << judge->who;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }
        return false;
    }
};

class Mihu: public TriggerSkill{
public:
    Mihu():TriggerSkill("mihu"){
        frequency = Compulsory;
        events << DrawNCards << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == DrawNCards){
            int n = data.toInt();
            data = n - 1;
            return false;
        }
        else if(player->getPhase() == Player::NotActive)
            player->drawCards(1);
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
    matsudajinpei->addSkill(new Skill("chaidan", Skill::Compulsory));
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

    General *kudouyukiko = new General(this, "kudouyukiko", "yi", 3, false);
    kudouyukiko->addSkill(new Luanzhen);
    kudouyukiko->addSkill(new Yinv);

    General *kudouyuusaku = new General(this, "kudouyuusaku$", "yi", 3);
    kudouyuusaku->addSkill(new Anye);
    kudouyuusaku->addSkill(new Panguan);
    kudouyuusaku->addSkill(new Fenbi);
    kudouyuusaku->addSkill(new Wuji);
    kudouyuusaku->addSkill(new MarkAssignSkill("@pen", 1));
    related_skills.insertMulti("fenbi", "#@pen-1");

    General *yamamuramisao = new General(this, "yamamuramisao", "jing", 3);
    yamamuramisao->addSkill(new Ruoyu);
    yamamuramisao->addSkill(new Zilian);

    General *suzukijirokichi = new General(this, "suzukijirokichi", "guai", 3);
    suzukijirokichi->addSkill(new Lvbai);
    suzukijirokichi->addSkill(new Lvzhan);

    General *jiikounosuke = new General(this, "jiikounosuke", "guai", 3);
    jiikounosuke->addSkill(new Zhongpu);
    jiikounosuke->addSkill(new ZhongpuEffect);
    related_skills.insertMulti("zhongpu", "#zhongpu-effect");
    jiikounosuke->addSkill(new Zhiqu);

    General *chianti = new General(this, "chianti", "hei", 4, false);
    chianti->addSkill(new Skill("weiju", Skill::Compulsory));
    chianti->addSkill(new Shanjing);

    General *korn = new General(this, "korn", "hei");
    korn->addSkill(new Skill("baotai", Skill::Compulsory));
    korn->addSkill(new Mangju);

    General *jamesblack = new General(this, "jamesblack$", "te");
    jamesblack->addSkill(new Anyong);
    jamesblack->addSkill(new Panda);

    General *hondoueisuke = new General(this, "hondoueisuke", "te");
    hondoueisuke->addSkill(new Mihu);
    hondoueisuke->addSkill(new Zhizhuo);

    General *miyamotoyumi = new General(this, "miyamotoyumi", "za", 3, false);
    miyamotoyumi->addSkill(new Skill("xuncha", Skill::Compulsory));
    miyamotoyumi->addSkill(new Skill("chuanyao", Skill::Compulsory));

    addMetaObject<JulunCard>();
    addMetaObject<ConghuiCard>();
    addMetaObject<HongmengCard>();
    addMetaObject<CimuCard>();
    addMetaObject<LuanzhenCard>();
    addMetaObject<FenbiCard>();
    addMetaObject<RuoyuCard>();
    addMetaObject<ZilianCard>();
    addMetaObject<ZhiquCard>();
    addMetaObject<AnyongCard>();
}

ADD_PACKAGE(Thicket)
