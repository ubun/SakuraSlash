#ifndef DEVILFRUITPACKAGE_H
#define DEVILFRUITPACKAGE_H

#include "package.h"
#include "card.h"

class DevilFruitPackage: public Package{
    Q_OBJECT

public:
    DevilFruitPackage();
};

// test
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
