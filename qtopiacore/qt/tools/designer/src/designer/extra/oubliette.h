/****************************************************************************
**
** Copyright (C) 2005-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OUBLIETTE_H
#define OUBLIETTE_H

#include <QtGui/QWidget>
#include <oublietteplan.h>
#include "cursor.h"

class QPaintEvent;
class QKeyEvent;
class QListWidgetItem;
struct ItemEffect;

class Oubliette : public QWidget
{
    Q_OBJECT
public:
    Oubliette();
    ~Oubliette();
    const Cursor &character() const { return m_character; }
    QPoint visualCursorPos() const;

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void timerEvent(QTimerEvent *);
    void showEvent(QShowEvent *);

private slots:
    void showInventoryItem(QListWidgetItem *lwi);
    void showInstructions();
    void showVictory();

private:
    void showInventory();
    void animateItem(const Item *item, const QPoint &pos);
    bool tryMove(const QPoint &newPos);
    void updateExplored();
    void paintOubliette(QPainter *p, const QRect &rect);
    void fillTile(QPainter *p, int x, int y, Tile tile);
    inline void fillTile(QPainter *p, const QPoint &point, Tile tile)
    { fillTile(p, point.x(), point.y(), tile); }

signals:
    void characterMoved(const QPoint &pt);

private:
    OubliettePlan m_oubliettePlan;
    Cursor m_character;
    QPoint m_oldCursorPosition;
    int m_currentLevel;
    QList<ItemEffect *> m_effects;
    int m_timerID;
};

#endif
