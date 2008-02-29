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

#ifndef __RECEIVEWINDOW_H__
#define __RECEIVEWINDOW_H__

#include <QMainWindow>
#include <QList>

class ReceivedFilesModel;
class QModelIndex;
class ReceivedFile;
class QTableView;
class QCloseEvent;
class RunningApplicationsViewItem;

class ReceiveWindow : public QMainWindow
{
    Q_OBJECT

public:
    ReceiveWindow(QWidget *parent = 0);
    ~ReceiveWindow();

public slots:
    void receiveInitiated(int id, const QString &filename, const QString &mime, const QString &description);
    void sendInitiated(int id, const QString &filename, const QString &mime);
    void progress(int id, qint64 bytes, qint64 total);
    void completed(int id, bool error);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void showWindow();
    void saveFile(int index);

private:
    void handleSupportedFormatRecv(int id, const QString &filename, const QString &mime, const QString &description);
    bool handleSupportedFormatComplete(int id, bool error);

    QTableView *m_files;
    ReceivedFilesModel *m_model;
    QList<ReceivedFile> m_list;
    RunningApplicationsViewItem *m_runningAppsItem;
};

#endif
