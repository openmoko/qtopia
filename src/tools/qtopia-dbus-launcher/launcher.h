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

#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#include <QObject>
#include <QString>

class QDBusMessage;

class Launcher : public QObject
{
    Q_OBJECT

public:
    Launcher(QObject *parent = 0);
    ~Launcher();

    void startup(const QString &app);

private slots:
    void handleSignalReceived(const QString &, const QDBusMessage &);

private:
    QString m_app;
};

#endif
