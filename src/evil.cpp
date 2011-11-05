#include "evil.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "standard.h"
#include "maneuvering.h"

class Bazhen: public TriggerSkill{
public:
    Bazhen():TriggerSkill("bazhen"){
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getArmor() && target->getMark("qinggang") == 0;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *wolong, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        Room *room = wolong->getRoom();
        if(wolong->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wolong;

            room->judge(judge);

            if(judge.isGood()){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(wolong, "good");
                return true;
            }else
                room->setEmotion(wolong, "bad");
        }

        return false;
    }
};

class BuquRemove: public TriggerSkill{
public:
    BuquRemove():TriggerSkill("#buqu_remove"){
        events << HpRecover;
    }

    virtual int getPriority() const{
        return -1;
    }

    static void Remove(ServerPlayer *zhoutai){
        Room *room = zhoutai->getRoom();
        QList<int> &buqu = zhoutai->getPile("buqu");

        int need = 1 - zhoutai->getHp();
        if(need <= 0){
            // clear all the buqu cards
            while(!buqu.isEmpty()){
                zhoutai->removeCardFromPile("buqu", buqu.takeFirst());
            }
        }else{
            int to_remove = buqu.length() - need;

            room->fillAG(buqu);

            int i;
            for(i=0; i<to_remove; i++){
                int card_id = room->askForAG(zhoutai, buqu, false, "buqu");
                zhoutai->removeCardFromPile("buqu", card_id);
            }

            room->broadcastInvoke("clearAG");
        }
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhoutai, QVariant &) const{
        if(!zhoutai->hasFlag("dying"))
            Remove(zhoutai);

        return false;
    }
};

class Buqu: public TriggerSkill{
public:
    Buqu():TriggerSkill("buqu"){
        events << Dying << AskForPeachesDone;
        default_choice = "alive";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhoutai, QVariant &data) const{
        Room *room = zhoutai->getRoom();
        QList<int> &buqu = zhoutai->getPile("buqu");

        if(event == Dying){
            int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
            int n = need - buqu.length();
            if(n > 0){
                QList<int> card_ids = room->getNCards(n);
                foreach(int card_id, card_ids){
                    zhoutai->addCardToPile("buqu", card_id);
                }
            }
        }else if(event == AskForPeachesDone){
            BuquRemove::Remove(zhoutai);

            if(zhoutai->getHp() > 0)
                return false;

            QSet<int> numbers;
            foreach(int card_id, buqu){
                const Card *card = Sanguosha->getCard(card_id);
                numbers << card->getNumber();
            }

            bool duplicated =  numbers.size() < buqu.size();
            if(!duplicated){
                QString choice = room->askForChoice(zhoutai, objectName(), "alive+dead");
                if(choice == "alive"){
                    room->playSkillEffect(objectName());
                    return true;
                }
            }
        }

        return false;
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

YinghunCard::YinghunCard(){

}

void YinghunCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();
    Room *room = effect.from->getRoom();

    bool good = false;
    if(x == 1){
        effect.to->drawCards(1);
        room->askForDiscard(effect.to, "yinghun", 1, false, true);
        good = true;
    }else{
        QString choice = room->askForChoice(effect.from, "yinghun", "d1tx+dxt1");
        if(choice == "d1tx"){
            effect.to->drawCards(1);
            x = qMin(x, effect.to->getCardCount(true));
            room->askForDiscard(effect.to, "yinghun", x, false, true);
            good = false;
        }else{
            effect.to->drawCards(x);
            room->askForDiscard(effect.to, "yinghun", 1, false, true);
            good = true;
        }
    }

    if(good)
        room->setEmotion(effect.to, "good");
    else
        room->setEmotion(effect.to, "bad");
}

class YinghunViewAsSkill: public ZeroCardViewAsSkill{
public:
    YinghunViewAsSkill():ZeroCardViewAsSkill("yinghun"){
    }

    virtual const Card *viewAs() const{
        return new YinghunCard;        
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@yinghun";
    }
};

class Yinghun: public PhaseChangeSkill{
public:
    Yinghun():PhaseChangeSkill("yinghun"){
        default_choice = "d1tx";

        view_as_skill = new YinghunViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *sunjian) const{
        if(sunjian->getPhase() == Player::Start && sunjian->isWounded()){
            Room *room = sunjian->getRoom();
            room->askForUseCard(sunjian, "@@yinghun", "@yinghun");
        }

        return false;
    }
};

class Jiuchi: public OneCardViewAsSkill{
public:
    Jiuchi():OneCardViewAsSkill("jiuchi"){
    }

    virtual bool isEnabledAtPlay() const{
        return Analeptic::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("analeptic");
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

class Xuanfeng: public TriggerSkill{
public:
    Xuanfeng():TriggerSkill("xuanfeng"){
        events << CardLost;
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        /*

        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *p, players){
            if(AI::GetRelation(player, p) != AI::Enemy)
                continue;

            if(player->distanceTo(p) <= 1)
                return "damage";
            else
                return "slash";
        }

        */

        return "nothing";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStruct move = data.value<CardMoveStruct>();

        if(move.from_place == Player::Equip){
            Room *room = player->getRoom();

            QString choice = room->askForChoice(player, objectName(), "slash+damage+nothing");
            if(choice == "slash"){
                QList<ServerPlayer *> players = room->getOtherPlayers(player);

                ServerPlayer *target = room->askForPlayerChosen(player, players, "xuanfeng-slash");

                CardEffectStruct effect;
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                effect.card = slash;
                effect.from = player;
                effect.to = target;

                room->cardEffect(effect);
            }else if(choice == "damage"){
                QList<ServerPlayer *> players = room->getOtherPlayers(player), targets;
                foreach(ServerPlayer *p, players){
                    if(player->distanceTo(p) <= 1)
                        targets << p;
                }

                ServerPlayer *target = room->askForPlayerChosen(player, targets, "xuanfeng-damage");

                DamageStruct damage;
                damage.from = player;
                damage.to = target;
                room->damage(damage);
            }
        }

        return false;
    }
};

class Yizhong: public TriggerSkill{
public:
    Yizhong():TriggerSkill("yizhong"){
        events << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getArmor() == NULL;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->isBlack()){
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

class Duanliang: public OneCardViewAsSkill{
public:
    Duanliang():OneCardViewAsSkill("duanliang"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->getSuit()==Card::Club && !card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();

        SupplyShortage *shortage = new SupplyShortage(card->getSuit(), card->getNumber());
        shortage->setSkillName(objectName());
        shortage->addSubcard(card);

        return shortage;
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

static bool CompareBySuit(int card1, int card2){
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getSuit());
    int b = static_cast<int>(c2->getSuit());

    return a < b;
}

class Shelie: public PhaseChangeSkill{
public:
    Shelie():PhaseChangeSkill("shelie"){

    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() != Player::Draw)
            return false;

        Room *room = shenlumeng->getRoom();
        if(!shenlumeng->askForSkillInvoke(objectName()))
            return false;

        room->playSkillEffect(objectName());

        QList<int> card_ids = room->getNCards(5);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);        
        room->fillAG(card_ids);

        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(shenlumeng, card_ids, false, "shelie");
            card_ids.removeOne(card_id);
            room->takeAG(shenlumeng, card_id);

            // throw the rest cards that matches the same suit
            const Card *card = Sanguosha->getCard(card_id);
            Card::Suit suit = card->getSuit();
            QMutableListIterator<int> itor(card_ids);
            while(itor.hasNext()){
                const Card *c = Sanguosha->getCard(itor.next());
                if(c->getSuit() == suit){
                    itor.remove();

                    room->takeAG(NULL, c->getId());
                }
            }
        }

        room->broadcastInvoke("clearAG");

        return true;
    }
};

FanjianCard::FanjianCard(){
    once = true;
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->showCard(zhouyu, card_id);
    room->getThread()->delay();

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

        room->damage(damage);
    }

    if(target->isAlive()){
        target->obtainCard(card);
    }
}

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){

    }

    virtual bool isEnabledAtPlay() const{
        return !Self->isKongcheng() && ! Self->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class HuojiViewAsSkill: public OneCardViewAsSkill{
public:
    HuojiViewAsSkill():OneCardViewAsSkill("huoji"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit()==Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        //me->drawCards(1);
        //»ð¼Æ·¢¶¯ºóÃþÒ»ÕÅÅÆ
        return fire_attack;
    }
};

class Huoji:public TriggerSkill{
public:
    Huoji():TriggerSkill("huoji"){
        view_as_skill = new HuojiViewAsSkill;
        events << CardUsed << CardResponsed;
    }
virtual bool trigger(TriggerEvent event, ServerPlayer *huoche, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("FireAttack")){
            Room *room = huoche->getRoom();
            room->playSkillEffect(objectName());
            huoche->drawCards(1);
            }
        return false;
    }
};

void YeyanCard::damage(ServerPlayer *shenzhouyu, ServerPlayer *target, int point) const{
    DamageStruct damage;

    damage.card = NULL;
    damage.from = shenzhouyu;
    damage.to = target;
    damage.damage = point;
    damage.nature = DamageStruct::Fire;

    shenzhouyu->getRoom()->damage(damage);
}

GreatYeyanCard::GreatYeyanCard(){

}

bool GreatYeyanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void GreatYeyanCard::use(Room *room, ServerPlayer *shenzhouyu, const QList<ServerPlayer *> &targets) const{
    room->broadcastInvoke("animate", "lightbox:$greatyeyan");

//    shenzhouyu->loseMark("@flame");
    room->throwCard(this);
    room->loseHp(shenzhouyu, 3);

    damage(shenzhouyu, targets.first(), 3);
}

MediumYeyanCard::MediumYeyanCard(){

}

bool MediumYeyanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 2;
}

void MediumYeyanCard::use(Room *room, ServerPlayer *shenzhouyu, const QList<ServerPlayer *> &targets) const{
    room->broadcastInvoke("animate", "lightbox:$mediumyeyan");

    shenzhouyu->loseMark("@flame");
    room->throwCard(this);
    room->loseHp(shenzhouyu, 3);

    ServerPlayer *first = targets.first();
    ServerPlayer *second = targets.value(1, NULL);

    damage(shenzhouyu, first, 2);

    if(second)
        damage(shenzhouyu, second, 1);
}

SmallYeyanCard::SmallYeyanCard(){

}

bool SmallYeyanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 3;
}

void SmallYeyanCard::use(Room *room, ServerPlayer *shenzhouyu, const QList<ServerPlayer *> &targets) const{
    room->broadcastInvoke("animate", "lightbox:$smallyeyan");
    shenzhouyu->loseMark("@flame");

    foreach(ServerPlayer *target, targets)
        room->cardEffect(this, shenzhouyu, target);
}

void SmallYeyanCard::onEffect(const CardEffectStruct &effect) const{
    damage(effect.from, effect.to, 1);
}

class GreatYeyan: public ViewAsSkill{
public:
    GreatYeyan(): ViewAsSkill("greatyeyan"){
    }

    virtual bool isEnabledAtPlay() const{
        return true;
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
        if(cards.length() != 4)
            return NULL;

        GreatYeyanCard *card = new GreatYeyanCard;
        card->addSubcards(cards);

        return card;
    }
};

class MediumYeyan: public GreatYeyan{
public:
    MediumYeyan(){
        setObjectName("mediumyeyan");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        MediumYeyanCard *card = new MediumYeyanCard;
        card->addSubcards(cards);

        return card;
    }
};

class SmallYeyan: public ZeroCardViewAsSkill{
public:
    SmallYeyan():ZeroCardViewAsSkill("smallyeyan"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@flame") >= 1;
    }

    virtual const Card *viewAs() const{
        return new SmallYeyanCard;
    }
};

class Zonghuo: public TriggerSkill{
public:
    Zonghuo():TriggerSkill("zonghuo"){
        frequency = Compulsory;
         events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
            damage.nature = DamageStruct::Fire;
            data = QVariant::fromValue(damage);

            LogMessage log;
            log.type = "#Zonghuo";
            log.from = player;
            player->getRoom()->sendLog(log);
        return false;
    }
};

LiuliCard::LiuliCard()
    :is_weapon(false)
{
}

void LiuliCard::setSlashSource(const QString &slash_source){
    this->slash_source = slash_source;
}

void LiuliCard::setIsWeapon(bool is_weapon)
{
    this->is_weapon = is_weapon;
}

bool LiuliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select->objectName() == slash_source)
        return false;

    if(to_select == Self)
        return false;

    if(is_weapon)
        return Self->distanceTo(to_select) <= 1;
    else
        return Self->inMyAttackRange(to_select);
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "liuli_target");
}

class LiuliViewAsSkill: public OneCardViewAsSkill{
public:
    LiuliViewAsSkill():OneCardViewAsSkill("liuli"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@liuli");;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(!ClientInstance->card_pattern.startsWith("@@liuli-"))
            return NULL;

        QString slash_source = ClientInstance->card_pattern;
        slash_source.remove("@@liuli-");

        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->setSlashSource(slash_source);

        const Card *card = card_item->getCard();
        liuli_card->addSubcard(card->getId());
        liuli_card->setIsWeapon(Self->getWeapon() == card);

        return liuli_card;
    }
};

class Liuli: public TriggerSkill{
public:
    Liuli():TriggerSkill("liuli"){
        view_as_skill = new LiuliViewAsSkill;

        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *daqiao, QVariant &data) const{
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
                QString prompt = "@liuli-card:" + effect.from->getGeneralName();
                if(room->askForUseCard(daqiao, "@@liuli-" + effect.from->objectName(), prompt)){
                    foreach(ServerPlayer *player, players){
                        if(player->hasFlag("liuli_target")){
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

LijianCard::LijianCard(){
    once = true;
}

bool LijianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
//    if(!to_select->getGeneral()->isMale())
//        return false;
    if(to_select->getKingdom()!="di" && to_select->getKingdom()!="ba")
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

bool LijianCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

void LijianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("lijian");
    duel->setCancelable(false);

    CardEffectStruct effect;
    effect.card = duel;
    effect.from = from;
    effect.to = to;

    room->cardEffect(effect);
}

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("LijianCard");
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

class Dongcha: public PhaseChangeSkill{
public:
    Dongcha():PhaseChangeSkill("dongcha"){

    }

    virtual bool onPhaseChange(ServerPlayer *jiawenhe) const{
        switch(jiawenhe->getPhase()){
        case Player::Start:{
                if(jiawenhe->askForSkillInvoke(objectName())){
                    Room *room = jiawenhe->getRoom();
                    QList<ServerPlayer *> players = room->getOtherPlayers(jiawenhe);
                    ServerPlayer *dongchaee = room->askForPlayerChosen(jiawenhe, players, objectName());
                    room->setPlayerFlag(dongchaee, "dongchaee");
                    room->setTag("Dongchaee", dongchaee->objectName());
                    room->setTag("Dongchaer", jiawenhe->objectName());

                    room->showAllCards(dongchaee, jiawenhe);
                }

                break;
            }

        case Player::Finish:{
                Room *room = jiawenhe->getRoom();
                QString dongchaee_name = room->getTag("Dongchaee").toString();
                if(!dongchaee_name.isEmpty()){
                    ServerPlayer *dongchaee = room->findChild<ServerPlayer *>(dongchaee_name);
                    room->setPlayerFlag(dongchaee, "-dongchaee");

                    room->setTag("Dongchaee", QVariant());
                    room->setTag("Dongchaer", QVariant());
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Lianhuan: public OneCardViewAsSkill{
public:
    Lianhuan():OneCardViewAsSkill("lianhuan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
        chain->addSubcard(card);
        chain->setSkillName(objectName());
        return chain;
    }
};

FangzhuCard::FangzhuCard(){
    mute = true;
}

void FangzhuCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();

    effect.to->drawCards(x);

    Room *room = effect.to->getRoom();
    room->playSkillEffect("fangzhu", effect.to->faceUp() ? 1 : 2);

    effect.to->turnOver();
}

class FangzhuViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuViewAsSkill():ZeroCardViewAsSkill("fangzhu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@fangzhu";
    }

    virtual const Card *viewAs() const{
        return new FangzhuCard;
    }
};

class Fangzhu: public MasochismSkill{
public:
    Fangzhu():MasochismSkill("fangzhu"){
        view_as_skill = new FangzhuViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &damage) const{
        Room *room = caopi->getRoom();
        room->askForUseCard(caopi, "@@fangzhu", "@fangzhu");
    }
};

class Weimu: public ProhibitSkill{
public:
    Weimu():ProhibitSkill("weimu"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("TrickCard") && card->isBlack() && !card->inherits("Collateral");
    }
};

class Xueyi: public PhaseChangeSkill{
public:
    Xueyi():PhaseChangeSkill("xueyi$"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("xueyi");
    }

    virtual int getPriority() const{
        return 3;
        // promote the priority in order to avoid the conflict with Yongsi
    }

    virtual bool onPhaseChange(ServerPlayer *yuanshao) const{
        if(yuanshao->getPhase() == Player::Discard){
            Room *room = yuanshao->getRoom();
            int n = room->getLieges("e", yuanshao).length();
            int xueyi = n * 2;
            yuanshao->setXueyi(xueyi);
        }

        return false;
    }
};

FatherCard::FatherCard(){
    target_fixed = true;
}

void FatherCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->drawCards(5);

    QList<ServerPlayer *> players = room->getAlivePlayers();
/*    QStringList all_generals = Sanguosha->getLimitedGeneralNames();
    QStringList shu_generals;
    foreach(QString name, all_generals){
        const General *general = Sanguosha->getGeneral(name);
        if(general->getKingdom() != "shu")
            continue;

        bool duplicated = false;
        foreach(ServerPlayer *player, players){
            if(player->getGeneralName() == name){
                duplicated = true;
                break;
            }
        }

        if(!duplicated)
            shu_generals << name;
    }

    QString general = room->askForGeneral(source, shu_generals);

    room->transfigure(source, general, false);
    room->acquireSkill(source, "father", false);
    source->setFlags("father");*/
    room->transfigure(source,"kazuma",false);
    room->acquireSkill(source, "father", false);
    source->setFlags("father");
    foreach(ServerPlayer *player, players){
        if(player->getGeneralName() == "sakura" && !player->hasSkill("xuanfeng")){
            Room *romm = player->getRoom();
            romm->acquireSkill(player, "xuanfeng");
            break;
        }
    }
}

class FatherViewAsSkill: public ZeroCardViewAsSkill{
public:
    FatherViewAsSkill():ZeroCardViewAsSkill("#father"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getHp()<2 && Self->getGeneral()->hasSkill("father");
    }

    virtual const Card *viewAs() const{
        return new FatherCard;
    }
};

class Father: public PhaseChangeSkill{
public:
    Father():PhaseChangeSkill("father"){
        view_as_skill = new FatherViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish &&
           target->hasFlag("father"))
        {
            Room *room = target->getRoom();
            room->transfigure(target, parent()->objectName(), false);
            room->killPlayer(target);
        }

        return false;
    }
};


EvilPackage::EvilPackage()
    :Package("evil")
{
    //µÐ¶ÔÊÆÁ¦
    General *kyougoku, *ou, *king, *mushit, *train, *fox, *spider;
/*¾©¼«ÇìÎá 5Ñª
¡¾Ê¥Ä§·½Õó¡¿°ËÕó
¡¾Ô¹µÐÍËÉ¢¡¿²»Çü
¡¾º£µ×ÀÌÔÂ¡¿±ÕÔÂ*/
        kyougoku = new General(this, "kyougoku$", "e", 3, true);
    kyougoku->addSkill(new Bazhen);
    kyougoku->addSkill(new Buqu);
    kyougoku->addSkill(new BuquRemove);
    kyougoku->addSkill(new Biyue);
    kyougoku->addSkill(new Xueyi);
/*¹íÍõ 6Ñª
¡¾°µÉñÍþ¡¿Ó¢»ê
¡¾·ÅÄ§ÐÇ³½¡¿¾Æ³Ø
¡¾Ó£»¨·ÅÉñ¡¿Ðý·ç*/
        ou = new General(this, "ou", "e", 4, true);
        ou->addSkill(new Yinghun);
        ou->addSkill(new Jiuchi);
        ou->addSkill(new Xuanfeng);
        ou->addSkill(new Father);
/*½ð¸Õ 6Ñª
¡¾´óÈÕ½£¡¿ÒãÖØ
¡¾¹íÉñºäÌìÉ±¡¿ÌúÆï
¡¾»ÆÍ¯×Ó¡¿¶ÏÁ¸*/
        king = new General(this, "king", "e", 4, true);
    king->addSkill(new Yizhong);
    king->addSkill(new Tieji);
    king->addSkill(new Duanliang);
/*Ä¾Ê³ 5Ñª
¡¾ÖÇÈ­¡¿¼¯ÖÇ
¡¾ð©Ê¸ÄîÁÙÑÝÎè¡¿ÉæÁÔ
¡¾Ô²¿Õ¡¿·´¼ä*/
        mushit = new General(this, "mushit", "e", 5, true);
    mushit->addSkill(new Jizhi);
    mushit->addSkill(new Shelie);
    mushit->addSkill(new Fanjian);
/*»ð³µ 6Ñª
¡¾ÎåîÜ¡¿»ð¼Æ£¨·¢¶¯ºóÃþÒ»ÕÅÅÆ£©
¡¾ºìÁ«»ðÂÖË«¡¿ÒµÑ×(´óÒµÑ×·ÇÏÞ¶¨£©
¡¾»ðÂÒ¡¿×Ý»ð*/
        train = new General(this, "train", "e", 4, true);
    train->addSkill(new Huoji);
    train->addSkill(new MarkAssignSkill("@flame", 1));
    train->addSkill(new GreatYeyan);
    train->addSkill(new MediumYeyan);
    train->addSkill(new SmallYeyan);
    train->addSkill(new Zonghuo);
/*Ë®ºü 6Ñª
¡¾±¦ÐÎ¡¿Á÷Àë
¡¾Ñ©»¨²¨ÎÆÊ®¹ì¡¿Àë¼ä
¡¾Ë®·ÖÉí¡¿¶´²ì*/
        fox = new General(this, "fox", "e");
    fox->addSkill(new Dongcha);
    fox->addSkill(new Liuli);
    fox->addSkill(new Lijian);
/*ÍÁÖ©Öë 5Ñª
¡¾°ËÒ¶¡¿Á¬»·
¡¾¾ÅÓ¡ÂüÝ±ÂÞ¡¿·ÅÖð
¡¾¶¾Ë¿èÑ¡¿á¡Ä»*/
    spider = new General(this, "spider", "e");
    spider->addSkill(new Lianhuan);
    spider->addSkill(new Fangzhu);
    spider->addSkill(new Weimu);

    addMetaObject<YinghunCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<FatherCard>(); //´È¸¸¼¼
    addMetaObject<GreatYeyanCard>();
    addMetaObject<MediumYeyanCard>();
    addMetaObject<SmallYeyanCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<LijianCard>();
    addMetaObject<FangzhuCard>();
}

ADD_PACKAGE(Evil)
