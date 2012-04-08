#ifndef THICKET_H
#define THICKET_H

#include "package.h"
#include "card.h"

class ThicketPackage: public Package{
    Q_OBJECT

public:
    ThicketPackage();
};

class JulunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JulunCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
/*
class HaoshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HaoshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DimengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DimengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LuanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};
*/
class ChaidanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChaidanCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

#endif // THICKET_H
