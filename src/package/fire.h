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

class FangxinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangxinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MoguaCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MoguaCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShendieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShendieCard();

    //virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YingyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YingyanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ChunbaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChunbaiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ManmiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ManmiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MijiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MijiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // FIREPACKAGE_H
