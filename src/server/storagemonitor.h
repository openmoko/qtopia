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

#ifndef _STORAGEMONITOR_H_
#define _STORAGEMONITOR_H_

#include <QObject>
class QFileSystem;
class QStorageMetaInfo;
class QMessageBox;

class StorageMonitor : public QObject
{
    Q_OBJECT
public:
    StorageMonitor(QObject *o = 0);
    ~StorageMonitor();

public slots:
    void checkAvailStorage();
    void systemMsg(const QString &msg, const QByteArray &data);

private slots:
    void showNotification();
    void showCleanupWizard();

private:
    void outOfSpace(QString& text);
    long fileSystemMetrics(const QFileSystem* fs);

    QStorageMetaInfo *sinfo;
    QMessageBox *box;
    bool suppressNotification;

};

#endif // _STORAGEMONITOR_H_
