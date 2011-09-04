#ifndef PARI_H
#define PARI_H

#include "package.h"
#include "card.h"

class JuejiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JuejiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QingnangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MingceCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MingceCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QuhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuhuCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XinzhanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XinzhanCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GuidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuidaoCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class NonoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NonoCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class PariPackage: public Package{
    Q_OBJECT

public:
    PariPackage();
};

#endif // PARI_H
