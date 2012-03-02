#ifndef DEVILFRUIT_H
#define DEVILFRUIT_H

#include "skill.h"
#include "card.h"

class DevilFruitPackage : public Package{
    Q_OBJECT

public:
    DevilFruitPackage();
};

class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TestPackage: public Package{
    Q_OBJECT

public:
    TestPackage();
};

#endif // DEVILFRUIT_H
