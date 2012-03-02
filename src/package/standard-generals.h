#ifndef STANDARDGENERALS_H
#define STANDARDGENERALS_H

#include "skill.h"
#include "card.h"

class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARDGENERALS_H
