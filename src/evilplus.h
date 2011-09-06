#ifndef EVIL2_H
#define EVIl2_H

#include "package.h"
#include "card.h"


class KurouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KurouCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class KuangfengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KuangfengCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XunleiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunleiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DawuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DawuCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class ShenfenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenfenCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WuqianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WuqianCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class XianzhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianzhenCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XianzhenSlashCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianzhenSlashCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class Evil2Package: public Package{
    Q_OBJECT

public:
    Evil2Package();
};

#endif // EVIL2_H
