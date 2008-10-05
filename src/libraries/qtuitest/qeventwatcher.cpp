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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qeventwatcher_p.h"

#include <QSet>
#include <QPointer>
#include <QCoreApplication>

struct QEventWatcherPrivate
{
    QSet<int>      types;
    QSet<QObject*> objects;
    int            count;
};

QEventWatcher::QEventWatcher(QObject* parent)
    : QObject(parent),
      d(new QEventWatcherPrivate)
{
    d->count = 0;
}

QEventWatcher::~QEventWatcher()
{ delete d; d = 0; }

void QEventWatcher::addType(QEvent::Type type)
{ d->types << int(type); }

void QEventWatcher::addObject(QObject* obj)
{
    d->objects << obj;
    qApp->installEventFilter(this);
}

int QEventWatcher::count() const
{ return d->count; }

bool QEventWatcher::eventFilter(QObject* obj, QEvent* e)
{
    if (!d->objects.contains(obj))         return false;
    if (!d->types.contains(e->type())) return false;

    ++d->count;
    emit event(obj, e->type());
    return false;
}

