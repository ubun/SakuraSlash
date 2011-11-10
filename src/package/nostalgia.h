#ifndef NOSTALGIA_H
#define NOSTALGIA_H

#include "package.h"
#include "card.h"
#include "standard.h"

class YitianSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE YitianSword(Card::Suit suit = Spade, int number = 6);

    virtual void onMove(const CardMoveStruct &move) const;
};

class NostalgiaPackage: public Package{
    Q_OBJECT

public:
    NostalgiaPackage();
};

#endif // NOSTALGIA_H
