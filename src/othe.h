#ifndef OTHE_H
#define OTHE_H

#include "package.h"
#include "card.h"

class GuicaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuicaiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GongxinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GongxinCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LexueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LexueCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShensuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShensuCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RendeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DimengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DimengCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TianyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianyiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QiaocaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiaocaiCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GanluCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GanluCard();
    void swapEquip(ServerPlayer *first, ServerPlayer *second, int index) const;

    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhihengCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class OthePackage: public Package{
    Q_OBJECT

public:
    OthePackage();
};

#endif // OTHE_H
