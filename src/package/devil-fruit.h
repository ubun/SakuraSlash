#ifndef DEVILFRUITPACKAGE_H
#define DEVILFRUITPACKAGE_H

#include "package.h"
#include "card.h"

class HoneymelonCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HoneymelonCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BananaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BananaCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class CherryCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CherryCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class DevilFruitPackage: public Package{
    Q_OBJECT

public:
    DevilFruitPackage();
};

// test
class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TestPackage: public Package{
    Q_OBJECT

public:
    TestPackage();
};

#endif // DEVILFRUIT_H
