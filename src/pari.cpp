#include "pari.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "maneuvering.h"

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

QingnangCard::QingnangCard(){
    once = true;
}

bool QingnangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    if(targets.length() > 1)
        return false;

    const ClientPlayer *to_cure = targets.isEmpty() ? Self : targets.first();
    return to_cure->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target;
    if(!targets.isEmpty())
        target = targets.first();
    else
        target = source;

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

class Qingnang: public OneCardViewAsSkill{
public:
    Qingnang():OneCardViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("QingnangCard");
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

JuejiCard::JuejiCard(){
    once = true;
}

bool JuejiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 2 && to_select != Self;
}

void JuejiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@jueji");
}

class JuejiViewAsSkill: public ViewAsSkill{
public:
    JuejiViewAsSkill():ViewAsSkill("jueji"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("JuejiCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return true;
        else if(selected.length() == 1){
            const Card *first = selected.first()->getFilteredCard();
            return first->sameColorWith(to_select->getFilteredCard());
        }

        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            JuejiCard *card = new JuejiCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Jueji: public DrawCardsSkill{
public:
    Jueji():DrawCardsSkill("jueji"){
        view_as_skill = new JuejiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@jueji") > 0;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        player->loseMark("@jueji");

        // find zhanghe
        ServerPlayer *zhanghe = room->findPlayerBySkillName(objectName());
        if(zhanghe){
            zhanghe->drawCards(1);
        }

        return n - 1;
    }
};

class JuejiClear: public PhaseChangeSkill{
public:
    JuejiClear():PhaseChangeSkill("#jueji-clear"){

    }

    virtual bool onPhaseChange(ServerPlayer *zhanghe) const{
        if(zhanghe->getPhase() == Player::Start){
            Room *room = zhanghe->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(zhanghe);
            foreach(ServerPlayer *player, players){
                if(player->getMark("@jueji") > 0){
                    player->loseMark("@jueji");
                }
            }
        }

        return false;
    }
};
/*
AngleCard::AngleCard(){
    target_fixed = true;
}

void AngleCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{

QList<ServerPlayer *> players = room->getAllPlayers();
foreach(const ServerPlayer *player, players){
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}
}

class Angle: public ZeroCardViewAsSkill{
public:
    Angle():ZeroCardViewAsSkill("angle"){
    }

    virtual const Card *viewAs() const{
        return new AngleCard;
    }
};
*/

MingceCard::MingceCard(){
    once = true;
    will_throw = false;
}

void MingceCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.to->getRoom();
    QString choice = room->askForChoice(effect.to, "mingce", "use+draw");
    if(choice == "use"){
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.to), targets;
        foreach(ServerPlayer *player, players){
            if(effect.to->canSlash(player))
                targets << player;
        }

        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(effect.to, targets, "mingce");
            room->cardEffect(new Slash(Card::NoSuit, 0), effect.to, target);
        }
    }else if(choice == "draw"){
        effect.to->drawCards(1, true);
    }
}

class Mingce: public OneCardViewAsSkill{
public:
    Mingce():OneCardViewAsSkill("mingce"){
        default_choice = "draw";
    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("MingceCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *c = to_select->getCard();
        return c->getTypeId() == Card::Equip || c->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MingceCard *card = new MingceCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Wushuang: public TriggerSkill{
public:
    Wushuang():TriggerSkill("wushuang"){
        events << SlashProceed;
//        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = lubu->getRoom();
        if(!lubu->askForSkillInvoke(objectName()))
            return false;
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

class Mashu: public Skill{
public:
    Mashu():Skill("mashu", Skill::Compulsory)
    {
    }
};

QuhuCard::QuhuCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool QuhuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    return true;
}

void QuhuCard::use(Room *room, ServerPlayer *xunyu, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *tiger = targets.first();

    room->playSkillEffect("quhu", 1);

    bool success = xunyu->pindian(tiger, "quhu", this);
    if(success){
        room->playSkillEffect("quhu", 2);

        QList<ServerPlayer *> players = room->getOtherPlayers(tiger), wolves;
        foreach(ServerPlayer *player, players){
            if(tiger->inMyAttackRange(player))
                wolves << player;
        }

        if(wolves.isEmpty())
            return;

        room->playSkillEffect("#tunlang");
        ServerPlayer *wolf = room->askForPlayerChosen(xunyu, wolves, "quhu");

        DamageStruct damage;
        damage.from = tiger;
        damage.to = wolf;

        room->damage(damage);

    }else{
        DamageStruct damage;
        damage.card = NULL;
        damage.from = tiger;
        damage.to = xunyu;

        room->damage(damage);
    }
}

class Quhu: public OneCardViewAsSkill{
public:
    Quhu():OneCardViewAsSkill("quhu"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("QuhuCard") && !Self->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new QuhuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Enyuan: public TriggerSkill{
public:
    Enyuan():TriggerSkill("enyuan"){
        events << HpRecover << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who != player){
                recover.who->drawCards(recover.recover);

                LogMessage log;
                log.type = "#EnyuanRecover";
                log.from = player;
                log.to << recover.who;
                log.arg = QString::number(recover.recover);

                room->sendLog(log);
            }
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *source = damage.from;
            if(source && source != player){
                const Card *card = room->askForCard(source, ".H", "@enyuan", false);
                if(card){
                    room->showCard(source, card->getEffectiveId());
                    player->obtainCard(card);
                }else{
                    room->loseHp(source);
                }
            }
        }

        return false;
    }
};

XinzhanCard::XinzhanCard(){
    target_fixed = true;
    once = true;
}

void XinzhanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(3), left;
    left = cards;

    QList<int> hearts;
    foreach(int card_id, cards){
        const Card *card = Sanguosha->getCard(card_id);
        if(card->getSuit() == Card::Heart)
            hearts << card_id;
    }

    if(!hearts.isEmpty()){
        room->fillAG(cards, source);

        while(!hearts.isEmpty()){
            int card_id = room->askForAG(source, hearts, true, "xinzhan");
            if(card_id == -1)
                break;

            if(!hearts.contains(card_id))
                continue;

            hearts.removeOne(card_id);
            left.removeOne(card_id);

            source->obtainCard(Sanguosha->getCard(card_id));
            room->showCard(source, card_id);
        }

        source->invoke("clearAG");
    }

    if(left.length() >= 2)
        room->doGuanxing(source, left, true);
 }

class Xinzhan: public ZeroCardViewAsSkill{
public:
    Xinzhan():ZeroCardViewAsSkill("xinzhan"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("XinzhanCard") && Self->getHandcardNum() > Self->getMaxHP();
    }

    virtual const Card *viewAs() const{
        return new XinzhanCard;
    }
};

class Jiushi: public ZeroCardViewAsSkill{
public:
    Jiushi():ZeroCardViewAsSkill("jiushi"){

    }

    virtual bool isEnabledAtPlay() const{
        return Analeptic::IsAvailable() && Self->faceUp();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("analeptic") && Self->faceUp();
    }

    virtual const Card *viewAs() const{
        Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
        analeptic->setSkillName("jiushi");
        return analeptic;
    }
};

class JiushiFlip: public TriggerSkill{
public:
    JiushiFlip():TriggerSkill("#jiushi-flip"){
        events << CardUsed << Predamaged << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == "jiushi")
                player->turnOver();
        }else if(event == Predamaged){
            player->tag["PredamagedFace"] = player->faceUp();
        }else if(event == Damaged){
            bool faceup = player->tag.value("PredamagedFace").toBool();
            if(!faceup && player->askForSkillInvoke("jiushi", data))
                player->turnOver();
        }

        return false;
    }
};

GuidaoCard::GuidaoCard(){
    target_fixed = true;
}

void GuidaoCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{
}

class GuidaoViewAsSkill:public ViewAsSkill{
public:
    GuidaoViewAsSkill():ViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guidao";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getFilteredCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        GuidaoCard *card = new GuidaoCard;
        card->addSubcard(cards.first()->getFilteredCard());

        return card;
    }
};

class Guidao: public TriggerSkill{
public:
    Guidao():TriggerSkill("guidao"){
        view_as_skill = new GuidaoViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isNude();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        const Card *card = room->askForCard(player, "@guidao", prompt);

        if(card){
            // the only difference for Guicai & Guidao
            player->obtainCard(judge->card);

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

class Shaoying: public TriggerSkill{
public:
    Shaoying():TriggerSkill("shaoying"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        ServerPlayer *luboyan = damage.from;
        if(!damage.to->isChained() && damage.nature == DamageStruct::Fire
           && luboyan->askForSkillInvoke(objectName(), data))
        {
            Room *room = luboyan->getRoom();

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = luboyan;

            room->judge(judge);

            if(judge.isGood()){
                DamageStruct shaoying_damage;
                shaoying_damage.nature = DamageStruct::Fire;
                shaoying_damage.from = luboyan;
                shaoying_damage.to = player->getNextAlive();

                room->damage(shaoying_damage);
            }
        }

        return false;
    }
};

class Theft: public OneCardViewAsSkill{
public:
    Theft():OneCardViewAsSkill("theft"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return (card->inherits("TrickCard") && card->isBlack());
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Indulgence");
    }
};

class Luoying: public TriggerSkill{
public:
    Luoying():TriggerSkill("luoying"){
        events << CardDiscarded << CardUsed << FinishJudge;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    QList<const Card *> getClubs(const Card *card) const{
        QList<const Card *> clubs;

        if(!card->isVirtualCard()){
            if(card->getSuit() == Card::Club)
                clubs << card;

            return clubs;
        }

        foreach(int card_id, card->getSubcards()){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->getSuit() == Card::Club)
                clubs << c;
        }

        return clubs;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<const Card *> clubs;

        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            const SkillCard *skill_card = qobject_cast<const SkillCard *>(use.card);
            if(skill_card && skill_card->subcardsLength() > 0 && skill_card->willThrow()){
                clubs = getClubs(skill_card);
            }
        }else if(event == CardDiscarded){
            const Card *card = data.value<CardStar>();
            if(card->subcardsLength() == 0)
                return false;

            clubs = getClubs(card);
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile
               && judge->card->getSuit() == Card::Club)
               clubs << judge->card;
        }

        if(clubs.isEmpty())
            return false;

        ServerPlayer *caozhi = room->findPlayerBySkillName(objectName());
        if(caozhi && caozhi->askForSkillInvoke(objectName(), data)){
            foreach(const Card *club, clubs)
                caozhi->obtainCard(club);
        }

        return false;
    }
};

class Ktajhnb: public SlashBuffSkill{
public:
    Ktajhnb():SlashBuffSkill("ktajhnb"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huangzhong = effect.from;
        Room *room = huangzhong->getRoom();
//可以回合外发动
        int num = effect.to->getHandcardNum();
        int nmu = effect.to->getHp();
        if(nmu == huangzhong->getHp() || num == huangzhong->getHandcardNum()){
            if(huangzhong->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                room->playSkillEffect(objectName());
                room->slashResult(effect, true);
                return true;
            }
        }
        return false;
    }
};

class Songwei: public TriggerSkill{
public:
    Songwei():TriggerSkill("songwei$"){
        events << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "ba";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        if(!card->inherits("Spade")){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                QVariant who = QVariant::fromValue(p);
                if(p->hasLordSkill("songwei") && player->askForSkillInvoke("songwei", who)){
                    room->playSkillEffect(objectName(), 1);
                    p->drawCards(1);
                }
            }
        }

        return false;
    }
};

NonoCard::NonoCard(){
    will_throw = false;
}

void NonoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target;
    if(!targets.isEmpty())
        target = targets.first();
    else
        target = source;
    const Card *c = Sanguosha->getCard(this->getSubcards().first());
    const EquipCard *equipped = qobject_cast<const EquipCard *>(c);
    equipped->use(room,target,targets);

    LogMessage log;
    log.type = "#Nonos";
    log.to << target;
    log.from = source;
    log.arg = c->objectName();
    room->sendLog(log);
}


class Nono3: public OneCardViewAsSkill{
public:
    Nono3():OneCardViewAsSkill("nono3"){
//装备牌为人作嫁衣裳
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        NonoCard *nonocd = new NonoCard;
        nonocd->addSubcard(card_item->getCard());
        return nonocd;
    }
};


PariPackage::PariPackage()
    :Package("pari")
{
    //巴黎华击团·花组的女猪脚们
    General *erica, *glycine, *coquelicot, *lobelia, *hanabi;
/*艾莉卡
热血，青囊，绝汲*/
    erica = new General(this, "erica$", "ba");
    erica->addSkill("#slashtogether");
    erica->addSkill(new Rexue);
    erica->addSkill(new Qingnang);
    erica->addSkill(new Jueji);
    erica->addSkill(new JuejiClear);
    erica->addSkill(new Songwei); //主公技颂威
/*库里息怒
陷阵，明策，无双*/
	glycine = new General(this, "glycine", "ba");
        glycine->addSkill("#slashtogether");
        glycine->addSkill(new Mingce);
        glycine->addSkill(new Wushuang);
        glycine->addSkill(new Mashu);//马术
/*寇库里克
驱虎，挪移，恩怨，心战*/
	coquelicot = new General(this, "coquelicot", "ba");
        coquelicot->addSkill("#slashtogether");
        coquelicot->addSkill(new Quhu);
        coquelicot->addSkill(new Nono3);
        coquelicot->addSkill(new Enyuan);
        coquelicot->addSkill(new Xinzhan);
/*罗贝利亚
鬼道，酒诗，烧营，窃贼（黑锦囊当顺牵）*/
	lobelia = new General(this, "lobelia", "ba");
        lobelia->addSkill("#slashtogether");
        lobelia->addSkill(new Jiushi);
        lobelia->addSkill(new JiushiFlip);
        lobelia->addSkill(new Guidao);
        lobelia->addSkill(new Shaoying);
        lobelia->addSkill(new Theft);
/*花火
落英，谦逊，烈弓改*/
	hanabi = new General(this, "hanabi", "ba");
        hanabi->addSkill("#slashtogether");
        hanabi->addSkill(new Luoying);
        hanabi->addSkill(new Qianxun);
        hanabi->addSkill(new Ktajhnb);

    addMetaObject<QingnangCard>();
    addMetaObject<JuejiCard>();
    addMetaObject<MingceCard>();
    addMetaObject<QuhuCard>();
    addMetaObject<XinzhanCard>();
    addMetaObject<GuidaoCard>();
    addMetaObject<NonoCard>();
}

ADD_PACKAGE(Pari)
