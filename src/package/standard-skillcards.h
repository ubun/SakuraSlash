#ifndef STANDARDSKILLCARDS_H
#define STANDARDSKILLCARDS_H

#include "skill.h"
#include "card.h"

class SQSJCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SQSJCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DCCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DCCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DLDCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DLDCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QingnangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class CKCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CKCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SBCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SBCard();

    virtual void use(Room *room, ServerPlayer *gin, const QList<ServerPlayer *> &) const;
};

class YYSDCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YYSDCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *vodka, const QList<ServerPlayer *> &targets) const;
};

class HDCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HDCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WQQCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WQQCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JXCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JXCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class CLCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CLCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LYCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LYCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MZCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MZCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YRCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YRCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YQCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YQCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARDSKILLCARDS_H
