/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef PICKBOARDPICKS_H
#define PICKBOARDPICKS_H

#include <qtopiaglobal.h>
#include <qframe.h>
#include <qlist.h>

// Internal stuff...
#include "pickboardcfg.h"

class PickboardPicks : public QFrame {
    Q_OBJECT
public:
    PickboardPicks(QWidget* parent=0, Qt::WFlags f=0);
    ~PickboardPicks();
    QSize sizeHint() const;
    void initialise();
    void setMode(int);
    int currentMode() const { return mode; }

    void mousePressEvent(QMouseEvent*);

    void resetState();

public slots:
    void doMenu();

protected:
    void drawContents( QPainter * );
    void mouseDoubleClickEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

protected:
    int mode;
    QList<PickboardConfig*> configs;

private:
    PickboardConfig* config() { return configs.at(mode); }
};


#endif
