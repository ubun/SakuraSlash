#include "standard.h"
#include "serverplayer.h"
#include "room.h"
#include "skill.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

QString BasicCard::getType() const{
    return "basic";
}

Card::CardType BasicCard::getTypeId() const{
    return Basic;
}

TrickCard::TrickCard(Suit suit, int number, bool aggressive)
    :Card(suit, number), aggressive(aggressive),
    cancelable(true)
{
}

bool TrickCard::isAggressive() const{
    return aggressive;
}

void TrickCard::setCancelable(bool cancelable){
    this->cancelable = cancelable;
}

QString TrickCard::getType() const{
    return "trick";
}

Card::CardType TrickCard::getTypeId() const{
    return Trick;
}

bool TrickCard::isCancelable(const CardEffectStruct &effect) const{
    return cancelable;
}

TriggerSkill *EquipCard::getSkill() const{
    return skill;
}

QString EquipCard::getType() const{
    return "equip";
}

Card::CardType EquipCard::getTypeId() const{
    return Equip;
}

QString EquipCard::getEffectPath(bool is_male) const{
    return "audio/card/common/equip.ogg";
}

void EquipCard::onUse(Room *room, const CardUseStruct &card_use) const{
    if(card_use.to.isEmpty()){
        ServerPlayer *player = card_use.from;

        QVariant data = QVariant::fromValue(card_use);
        RoomThread *thread = room->getThread();
        thread->trigger(CardUsed, player, data);

        thread->trigger(CardFinished, player, data);
    }else
        Card::onUse(room, card_use);
}

void EquipCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const EquipCard *equipped = NULL;
    switch(location()){
    case WeaponLocation: equipped = source->getWeapon(); break;
    case ArmorLocation: equipped = source->getArmor(); break;
    case DefensiveHorseLocation: equipped = source->getDefensiveHorse(); break;
    case OffensiveHorseLocation: equipped = source->getOffensiveHorse(); break;
    }

    if(equipped)
        room->throwCard(equipped);

    LogMessage log;
    log.from = source;
    log.type = "$Install";
    log.card_str = QString::number(getEffectiveId());
    room->sendLog(log);

    room->moveCardTo(this, source, Player::Equip, true);
}

void EquipCard::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void EquipCard::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->removeTriggerSkill(skill);
}

QString GlobalEffect::getSubtype() const{
    return "global_effect";
}

void GlobalEffect::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> all_players = room->getAllPlayers();
    TrickCard::use(room, source, all_players);
}

QString AOE::getSubtype() const{
    return "aoe";
}

bool AOE::isAvailable() const{
    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    int count = 0;
    foreach(const ClientPlayer *player, players){
        if(player == Self)
            continue;

        if(player->isDead())
            continue;

        if(ClientInstance->isProhibited(player, this))
            continue;

        count ++;
    }

    return count > 0;
}

void AOE::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> targets;
    QList<ServerPlayer *> other_players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, other_players){
        const ProhibitSkill *skill = room->isProhibited(source, player, this);
        if(skill){
            LogMessage log;
            log.type = "#SkillAvoid";
            log.from = player;
            log.arg = skill->objectName();
            log.arg2 = objectName();
            room->sendLog(log);

            room->playSkillEffect(skill->objectName());
        }else
            targets << player;
    }

    TrickCard::use(room, source, targets);
}

QString SingleTargetTrick::getSubtype() const{
    return "single_target_trick";
}

bool SingleTargetTrick::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return true;
}

DelayedTrick::DelayedTrick(Suit suit, int number, bool movable)
    :TrickCard(suit, number, true), movable(movable)
{
}

void DelayedTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(target_fixed)
        room->moveCardTo(this, source, Player::Judging, true);
    else{
        ServerPlayer *target = targets.first();
        room->moveCardTo(this, target, Player::Judging, true);
    }
}

QString DelayedTrick::getSubtype() const{
    return "delayed_trick";
}

void DelayedTrick::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(!movable)
        room->throwCard(this);

    LogMessage log;
    log.from = effect.to;
    log.type = "#DelayedTrick";
    log.arg = effect.card->objectName();
    room->sendLog(log);

    JudgeStruct judge_struct = judge;
    judge_struct.who = effect.to;
    room->judge(judge_struct);

    if(judge_struct.isBad()){
        room->throwCard(this);
        takeEffect(effect.to);
    }else if(movable){
        onNullified(effect.to);
    }
}

void DelayedTrick::onNullified(ServerPlayer *target) const{
    Room *room = target->getRoom();
    if(movable){
        QList<ServerPlayer *> players = room->getOtherPlayers(target);
        players << target;

        foreach(ServerPlayer *player, players){            
            if(player->containsTrick(objectName()))
                continue;

            if(room->isProhibited(target, player, this))
                continue;

            room->moveCardTo(this, player, Player::Judging, true);
            break;
        }
    }else
        room->throwCard(this);
}

const DelayedTrick *DelayedTrick::CastFrom(const Card *card){
    DelayedTrick *trick = NULL;
    Card::Suit suit = card->getSuit();
    int number = card->getNumber();
    if(card->inherits("DelayedTrick"))
        return qobject_cast<const DelayedTrick *>(card);
    else if(card->getSuit() == Card::Diamond && (card->inherits("BasicCard") || card->inherits("EquipCard"))){
        trick = new Indulgence(suit, number);
        trick->addSubcard(card->getId());
    }else if(card->getSuit() == Card::Club && (card->inherits("BasicCard") || card->inherits("EquipCard"))){
        trick = new SupplyShortage(suit, number);
        trick->addSubcard(card->getId());
    }else if(card->getSuit() == Card::Heart && (card->inherits("BasicCard") || card->inherits("EquipCard"))){
        trick = new Hitself(suit, number);
        trick->addSubcard(card->getId());
    }

    return trick;
}

Weapon::Weapon(Suit suit, int number, int range)
    :EquipCard(suit, number), range(range), attach_skill(false)
{
}

int Weapon::getRange() const{
    return range;
}

QString Weapon::getSubtype() const{
    return "weapon";
}

EquipCard::Location Weapon::location() const{
    return WeaponLocation;
}

QString Weapon::label() const{
    return QString("%1(%2)").arg(getName()).arg(range);
}

void Weapon::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    Room *room = player->getRoom();
    room->setPlayerProperty(player, "atk", range);

    if(attach_skill)
        room->attachSkillToPlayer(player, objectName());
}

void Weapon::onUninstall(ServerPlayer *player) const{
    EquipCard::onUninstall(player);
    Room *room = player->getRoom();
    room->setPlayerProperty(player, "atk", 1);

    if(attach_skill)
        room->detachSkillFromPlayer(player, objectName());
}

QString Armor::getSubtype() const{
    return "armor";
}

EquipCard::Location Armor::location() const{
    return ArmorLocation;
}

QString Armor::label() const{
    return getName();
}

Horse::Horse(Suit suit, int number, int correct)
    :EquipCard(suit, number), correct(correct)
{
}

QString Horse::getEffectPath(bool) const{
    return "audio/card/common/horse.ogg";
}

void Horse::onInstall(ServerPlayer *) const{

}

void Horse::onUninstall(ServerPlayer *) const{

}

QString Horse::label() const{
    QString format;

    if(correct > 0)
        format = "%1(+%2)";
    else
        format = "%1(%2)";

    return format.arg(getName()).arg(correct);
}

OffensiveHorse::OffensiveHorse(Card::Suit suit, int number)
    :Horse(suit, number, -1)
{

}

QString OffensiveHorse::getSubtype() const{
    return "offensive_horse";
}

DefensiveHorse::DefensiveHorse(Card::Suit suit, int number)
    :Horse(suit, number, +1)
{

}

QString DefensiveHorse::getSubtype() const{
    return "defensive_horse";
}

EquipCard::Location Horse::location() const{
    if(correct > 0)
        return DefensiveHorseLocation;
    else
        return OffensiveHorseLocation;
}

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

TuxiCard::TuxiCard(){
}

bool TuxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "h", "tuxi");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class TuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@tuxi";
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

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();
        if(guojia->getHp()<guojia->getHandcardNum())
            return;
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

class SavageAssaultAvoid: public TriggerSkill{
public:
    SavageAssaultAvoid(const QString &avoid_skill)
        :TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill)
    {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("SavageAssault")){
            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = avoid_skill;
            log.arg2 = "savage_assault";
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }

private:
    QString avoid_skill;
};

class Huoshou: public TriggerSkill{
public:
    Huoshou():TriggerSkill("huoshou"){
        events << Predamage << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("SavageAssault")){
            Room *room = player->getRoom();
            ServerPlayer *menghuo = room->findPlayerBySkillName(objectName());
            if(menghuo){
                room->playSkillEffect(objectName());

                damage.from = menghuo;
                room->damage(damage);
                return true;
            }
        }

        return false;
    }
};

JijiangCard::JijiangCard(){

}

bool JijiangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void JijiangCard::use(Room *room, ServerPlayer *liubei, const QList<ServerPlayer *> &targets) const{
    QString resul = room->askForChoice(liubei, "zhaohuan", "yes+no");
    QList<ServerPlayer *> lieges = resul == "yes" ? room->getLieges("di", liubei) : room->getLieges("ba", liubei);

    const Card *slash = NULL;
    foreach(ServerPlayer *liege, lieges){
        QString result = room->askForChoice(liege, "jijiang", "accept+ignore");
        if(result == "ignore")
            continue;

        slash = room->askForCard(liege, "slash", "@jijiang-slash");
        if(slash){
            liubei->invoke("increaseSlashCount");
            room->cardEffect(slash, liubei, targets.first());
            return;
        }
    }
}

class JijiangViewAsSkill:public ZeroCardViewAsSkill{
public:
    JijiangViewAsSkill():ZeroCardViewAsSkill("jijiang$"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->hasLordSkill("jijiang") && Slash::IsAvailable();
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
        QList<ServerPlayer *> lieges = room->getLieges("di+ba", liubei);
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

StandardPackage::StandardPackage()
    :Package("standard")
{
    addCards();
    General *ogami = new General(this, "ogami$", "god", 4, true); //大神一郎
    ogami->addSkill(new Tuxi);//突袭
    ogami->addSkill(new Yiji);//遗计
    ogami->addSkill(new SavageAssaultAvoid("huoshou"));
    ogami->addSkill(new Huoshou);//祸首
    ogami->addSkill(new Jijiang); //激将
/*    General *liubei;
    liubei = new General(this, "liubei$", "shu");
*/
    addMetaObject<TuxiCard>();
    addMetaObject<JijiangCard>();
    addMetaObject<CheatCard>();

}

ADD_PACKAGE(Standard)
