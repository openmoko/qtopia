/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include <qcontent.h>

class FileTransferListModel;
class QModelIndex;
class FileTransfer;
class QListView;
class QCloseEvent;
class TaskManagerEntry;;
class QAction;
class QTabWidget;

struct VObjectTransfer
{
    int id;
    QString fileName;
    QString mimeType;
};


class ReceiveWindow : public QMainWindow
{
    Q_OBJECT

public:
    ReceiveWindow(QWidget *parent = 0);

public slots:
    void receiveInitiated(int id, const QString &filename,
            const QString &mime, const QString &description);
    void sendInitiated(int id, const QString &filename, const QString &mime,
            const QString &description);
    void progress(int id, qint64 bytes, qint64 total);
    void completed(int id, bool error);

signals:
    void abortTransfer(int id);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void activated(const QModelIndex &index);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void showWindow();
    void stopCurrentTransfer();

private:
    void setUpView(QListView *view);
    QContentId saveFile(const FileTransfer &file);
    void handleIncomingVObject(int id, const QString &fileName,
            const QString &mime, const QString &description);
    bool handleVObjectReceived(int id, bool error);

    FileTransferListModel *m_model;
    QList<VObjectTransfer> m_vObjects;
    TaskManagerEntry *m_taskManagerEntry;
    QAction *m_cancelAction;
    QTabWidget *m_tabs;
    QListView *m_incomingView;
    QListView *m_outgoingView;
};

#endif
