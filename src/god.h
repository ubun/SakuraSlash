#ifndef GODP_H
#define GODP_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class GodpPackage : public Package{
    Q_OBJECT

public:
    GodpPackage();
};

class XuanhuoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XuanhuoCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuihanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuihanCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LuanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JujianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JujianCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JieyinCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinCard();
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Feiying: public Skill{
public:
    Feiying();
};

class ChengxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChengxiangCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuangtianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuangtianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

#endif // GODP_H
