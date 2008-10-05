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

#ifndef QEVENTWATCHER_H
#define QEVENTWATCHER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QEvent>

class QEventWatcherPrivate;

class QEventWatcher : public QObject
{
    Q_OBJECT

public:
    explicit QEventWatcher(QObject* parent =0);
    virtual ~QEventWatcher();

    void addType(QEvent::Type);
    void addObject(QObject*);

    int count() const;

signals:
    void event(QObject*,int);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QEventWatcherPrivate* d;
};

#endif

