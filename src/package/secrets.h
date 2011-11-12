#ifndef SECRETS_H
#define SECRETS_H

#include "package.h"
#include "standard.h"

class SecretsCard:public Card{
    Q_OBJECT

public:
    SecretsCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual CardType getTypeId() const;
};

class Tips: public SecretsCard{
    Q_OBJECT

public:
    Q_INVOKABLE Tips(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class SecretsPackage: public Package{
    Q_OBJECT

public:
    SecretsPackage();
};

#endif // SECRETS_H
