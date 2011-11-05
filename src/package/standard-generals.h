#ifndef STANDARDGENERALS_H
#define STANDARDGENERALS_H

#include "skill.h"
#include "card.h"

class ZhenxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhenxiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JiaojinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiaojinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *heiji, const QList<ServerPlayer *> &targets) const;
};

class MazuiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MazuiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShiyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShiyanCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShouqiuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouqiuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BaiyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaiyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DiaobingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiaobingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RenxingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RenxingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

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

#endif // STANDARDGENERALS_H
