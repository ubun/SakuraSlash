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

class ConghuiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ConghuiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CimuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CimuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

typedef Skill SkillClass;
class LuanzhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuanzhenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HongmengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HongmengCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ChaidanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChaidanCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class RuoyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RuoyuCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class ZilianCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZilianCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhiquCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiquCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select) const;
};

#endif // THICKET_H
