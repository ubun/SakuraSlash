#ifndef FIREPACKAGE_H
#define FIREPACKAGE_H

#include "package.h"
#include "card.h"

class FirePackage : public Package{
    Q_OBJECT

public:
    FirePackage();
};

class IentouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE IentouCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
/*
class JiemingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiemingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QiangxiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiangxiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TianyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
*/
#endif // FIREPACKAGE_H
