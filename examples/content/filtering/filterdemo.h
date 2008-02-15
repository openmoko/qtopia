/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef FILTERDEMO_H
#define FILTERDEMO_H

#include <QListView>
#include <QContentSet>

class QKeyEvent;

class FilterDemo : public QListView
{
    Q_OBJECT
public:
    FilterDemo( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

protected:
    void keyPressEvent( QKeyEvent *event );

private:
    bool nextFilter();

    int index;
    QContentSet contentSet;
};

#endif
