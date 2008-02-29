/****************************************************************************
** $Id: qt/examples/winmanager/minimal.h   2.3.12   edited 2005-10-27 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qwsmanager_qws.h"

class QPixmap;
class QWidget;
class QRegion;

class QWSMinimalDecoration : public QWSDecoration
{
public:
    QWSMinimalDecoration();
    virtual ~QWSMinimalDecoration();
    
    virtual QRegion region(const QWidget *, const QRect &, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);

protected:
    QPixmap *closePixmap;
    QPixmap *minimizePixmap;
    QPixmap *maximizePixmap;
    QPixmap *normalizePixmap;
};

