/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef CHANGELISTENER_H
#define CHANGELISTENER_H

#include <QLabel>
#include <QContent>
#include <QFileInfo>

class ChangeListener : public QLabel
{
    Q_OBJECT

public:
    ChangeListener( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    ~ChangeListener();

private slots:
    void timeout();
    void changed( const QContentIdList &contentIds, QContent::ChangeType type );

private:
    int nextIndex;
    QContentId lastContentId;
    QString categoryId;
    QFileInfoList imageFiles;
};

#endif // CHANGELISTENER_H
