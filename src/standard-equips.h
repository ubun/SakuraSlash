#ifndef STANDARDEQUIPS_H
#define STANDARDEQUIPS_H

#include "standard.h"

class Crossbow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Crossbow(Card::Suit suit, int number = 1);
};

class DoubleSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE DoubleSword(Card::Suit suit = Spade, int number = 2);
};

class QinggangSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE QinggangSword(Card::Suit suit = Spade, int number = 6);
};

class Blade:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Blade(Card::Suit suit = Spade, int number = 5);
};

class Spear:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Spear(Card::Suit suit = Spade, int number = 12);
};

class Axe:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Axe(Card::Suit suit = Diamond, int number = 5);
};

class Halberd:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Halberd(Card::Suit suit = Diamond, int number = 12);
};

class KylinBow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE KylinBow(Card::Suit suit = Heart, int number = 5);
};

class EightDiagram:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE EightDiagram(Card::Suit suit, int number = 2);
};

class IceSword: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE IceSword(Card::Suit suit, int number);
};

class Koubu: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Koubu(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    TriggerSkill *jue_ying;
};

class Koubux: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Koubux(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    TriggerSkill *di_lu;
};

class Isenclight: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Isenclight(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    TriggerSkill *chi_tu;
};

class KoubuF: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE KoubuF(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    TriggerSkill *da_yuan;
};

class Mocao: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Mocao(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    TriggerSkill *zhua_h;
};

class Watkemo: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Watkemo(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    TriggerSkill *zi_xing;
};


#endif // STANDARDEQUIPS_H
