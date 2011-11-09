#ifndef WIND_H
#define WIND_H

#include "package.h"
#include "card.h"

class FatingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FatingCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TuanzhangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuanzhangCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class HuachiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuachiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YunchouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YunchouCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WeijiaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeijiaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WeixiaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeixiaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShuangyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShuangyuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhiyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiyuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WindPackage: public Package{
    Q_OBJECT

public:
    WindPackage();
};

#endif // WIND_H
