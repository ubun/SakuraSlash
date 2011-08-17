#ifndef MANEUVERING_H
#define MANEUVERING_H

#include "standard.h"

class NatureSlash: public Slash{
    Q_OBJECT

public:
    NatureSlash(Suit suit, int number, DamageStruct::Nature nature);
    virtual bool match(const QString &pattern) const;
};

class ThunderSlash: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE ThunderSlash(Card::Suit suit, int number);
};

class FireSlash: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE FireSlash(Card::Suit suit, int number);
};

class Analeptic: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Analeptic(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;

    static bool IsAvailable();

    virtual bool isAvailable() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Fan: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Fan(Card::Suit suit, int number);
};

class GudingBlade: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE GudingBlade(Card::Suit suit, int number);
};

class Machine: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Machine(Card::Suit suit, int number);
};

class Doggy: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Doggy(Card::Suit suit, int number);
};

class IronChain: public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE IronChain(Card::Suit suit, int number);

    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FireAttack: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE FireAttack(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SupplyShortage: public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE SupplyShortage(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class Floriation: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Floriation(Card::Suit suit, int number);
};

class Yajiao: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Yajiao(Card::Suit suit, int number);
};

//sakura
class Pistol: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Pistol(Card::Suit suit, int number);
    virtual void onInstall(ServerPlayer *player) const;
};

class Muramasa: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Muramasa(Card::Suit suit, int number);
};

class Mazinka: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Mazinka(Card::Suit suit, int number);
    virtual void onUninstall(ServerPlayer *player) const;
    virtual void onMove(const CardMoveStruct &move) const;
};

class Cat: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Cat(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;
    virtual QString getEffectPath(bool is_male) const;

private:
    TriggerSkill *grab_peach;
};

class Ufo:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Ufo(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class ET:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE ET(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class Redalert:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE Redalert(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Renew: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Renew(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Hitself:public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Hitself(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class Turnover: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Turnover(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Bathroom: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Bathroom(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class ManeuveringPackage: public Package{
    Q_OBJECT

public:
    ManeuveringPackage();
};

#endif // MANEUVERING_H
