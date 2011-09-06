#ifndef DEGO_H
#define DEGO_H

#include "package.h"
#include "card.h"
#include "standard.h"

class LianliCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianliCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class LianliSlashCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianliSlashCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TianxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianxiangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HaoshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HaoshiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiemingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiemingCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LeijiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LeijiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QiangxiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiangxiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WushenSlash: public Slash{
    Q_OBJECT

public:
    Q_INVOKABLE WushenSlash(Card::Suit suit, int number);
};

class DegoPackage: public Package{
    Q_OBJECT

public:
    DegoPackage();
};

#endif // DEGO_H
