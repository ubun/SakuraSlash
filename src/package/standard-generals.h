#ifndef STANDARDGENERALS_H
#define STANDARDGENERALS_H

#include "skill.h"
#include "card.h"

class AnshaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnshaCard();

    virtual void use(Room *room, ServerPlayer *gin, const QList<ServerPlayer *> &) const;
};

class MaixiongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MaixiongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *vodka, const QList<ServerPlayer *> &targets) const;
};

class GaizaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GaizaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YuandingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuandingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JingshenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JingshenCard();

    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARDGENERALS_H
