#ifndef EVIL_H
#define EVIl_H

#include "package.h"
#include "card.h"

class YinghunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YinghunCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FanjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YeyanCard: public SkillCard{
    Q_OBJECT

public:
    void damage(ServerPlayer *shenzhouyu, ServerPlayer *target, int point) const;
};

class GreatYeyanCard: public YeyanCard{
    Q_OBJECT

public:
    Q_INVOKABLE GreatYeyanCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MediumYeyanCard: public YeyanCard{
    Q_OBJECT

public:
    Q_INVOKABLE MediumYeyanCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SmallYeyanCard: public YeyanCard{
    Q_OBJECT

public:
    Q_INVOKABLE SmallYeyanCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LiuliCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiuliCard();
    void setSlashSource(const QString &slash_source);
    void setIsWeapon(bool is_weapon);
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

private:
    QString slash_source;
    bool is_weapon;
};

class LijianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LijianCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FangzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangzhuCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FatherCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FatherCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class EvilPackage: public Package{
    Q_OBJECT

public:
    EvilPackage();
};

#endif // EVIL_H
