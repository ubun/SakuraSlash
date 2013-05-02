#ifndef JOYPACKAGE_H
#define JOYPACKAGE_H

#include "package.h"
#include "standard.h"

class JoyPackage: public Package{
    Q_OBJECT

public:
    JoyPackage();
};

class JoyEquipPackage: public Package{
    Q_OBJECT

public:
    JoyEquipPackage();
};

class Shit:public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Shit(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onMove(const CardMoveStruct &move) const;

    static bool HasShit(const Card *card);
};

class Stink: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Stink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Monkey: public OffensiveCar{
    Q_OBJECT

public:
    Q_INVOKABLE Monkey(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;
    virtual QString getEffectPath(bool is_male) const;

private:
    TriggerSkill *grab_peach;
};

class GaleShell:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE GaleShell(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ThunderShell:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE ThunderShell(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif // JOYPACKAGE_H
