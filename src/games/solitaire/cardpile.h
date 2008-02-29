/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef CARD_PILE_H
#define CARD_PILE_H


#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qlist.h>
#include "cardmetrics.h"


enum ePileStackingType {
    pileCascades = 0, pileStacks, pileCascadesOrStacks
};


enum ePileFaceingType {
    pileFaceUp = 0, pileFaceDown, pileFaceUpOrDown
};


class Card;
class Config;


class CardPile : public QList<Card>
{
public:
    CardPile(QPoint p, QString name);
    virtual ~CardPile() { }

    QString name() const { return pileName; }
    void setName(QString n) { pileName = n; }

    QPoint pos() const { return curPosition; }
    void setPos(QPoint p) { curPosition = p; }

    QPoint nextPos() const { return nextPosition; }
    void setNextPos(QPoint p) { nextPosition = p; }

    QSize size() const { return QSize(CardMetrics::width(), CardMetrics::height()); }
    QRect pileRect() const { return QRect(pos(),size()); }

    void beginDealing() { dealing = true; }
    void endDealing() { dealing = false; }
    bool isDealing() const { return dealing; }

    void setDropTarget( bool t ) { target = t; }
    bool isDropTarget() const { return target; }
    
    int distanceFromPile(QPoint p) const;
    int distanceFromNextPos(QPoint p) const;
    
    Card *cardOnTop() const { return getLast(); }
    Card *cardOnBottom() const { return getFirst(); }
    static Card *cardInfront(Card *c);
    static Card *cardBehind(Card *c);
    bool kingOnTop() const;

    bool addCardToTop(Card *c);
    bool addCardToBottom(Card *c);
    bool removeCard(Card *c);
    
    virtual void cardAddedToTop(Card *) { }
    virtual void cardAddedToBottom(Card *) { }
    virtual void cardRemoved(Card *) { }
    virtual bool isAllowedOnTop(Card *) { return FALSE; }
    virtual bool isAllowedOnBottom(Card *) { return FALSE; }
    virtual bool isAllowedToBeMoved(Card *) { return FALSE; }
    virtual bool isSelectable(Card *) { return FALSE; }
    virtual QPoint cardPos(Card *) const { return pos(); }
    virtual QPoint hypertheticalNextCardPos() const { return pos(); }

    void readConfig(Config& cfg, CardPile *undealtCards);
    void writeConfig(Config& cfg);
   
private:
    bool dealing;
    bool target;
    QPoint curPosition, nextPosition;
    QString pileName;
};


#endif

