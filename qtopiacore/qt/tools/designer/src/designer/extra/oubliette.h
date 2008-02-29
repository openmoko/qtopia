/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
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
