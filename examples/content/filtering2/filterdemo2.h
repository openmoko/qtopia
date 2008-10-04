/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef FILTERDEMO_H
#define FILTERDEMO_H

#include <QListView>
#include <QContentSet>
#include <QMediaList>

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
    QContentSet  contentSet;
    QMediaList*  mediaList;
};

#endif
