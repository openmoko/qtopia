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

#ifndef __TASKMANAGERENTRY_H__
#define __TASKMANAGERENTRY_H__

#include <QObject>

class TaskManagerEntryPrivate;

class TaskManagerEntry : public QObject
{
    Q_OBJECT
public:
    TaskManagerEntry(const QString &description, const QString &iconPath, QObject *parent = 0);

public slots:
    void show();
    void hide();

signals:
    void activated();

private:
    friend class TaskManagerEntryPrivate;
    TaskManagerEntryPrivate *m_data;
};

#endif
