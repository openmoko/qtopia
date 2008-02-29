/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef SCRIPT_THREAD
#define SCRIPT_THREAD

#include <qtopiaglobal.h>
#include <QList>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QWaitCondition>

class QTOPIACOMM_EXPORT ScriptThread : public QThread {
    Q_OBJECT
public:
    explicit ScriptThread( QObject * parent = 0 );
    ~ScriptThread();

    void addScriptToRun( const QString& scriptPath, const QStringList& parameter );
    int remainingTasks() const;
    void run();
signals:
    void scriptDone();

private:
    QList<QString> scripts;
    QList<QStringList> params;
    bool quit;
    QWaitCondition waitCond;
    mutable QMutex mutex;
};

#endif //SCRIPT_THREAD
