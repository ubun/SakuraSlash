#ifndef MOUNTAINPACKAGE_H
#define MOUNTAINPACKAGE_H

#include "package.h"
#include "card.h"
#include "generaloverview.h"

class ShengongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShengongCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class FeitiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeitiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class CanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CanwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhaodaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaodaiCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JingshenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JingshenCard();

    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShehangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShehangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class MountainPackage : public Package
{
    Q_OBJECT

public:
    MountainPackage();
};

#endif // MOUNTAINPACKAGE_H
