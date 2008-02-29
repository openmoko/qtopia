/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __RECEIVEWINDOW_H__
#define __RECEIVEWINDOW_H__

#include <QMainWindow>
#include <QList>

class ReceivedFilesModel;
class QModelIndex;
class ReceivedFile;
class QTableView;

class ReceiveWindow : public QMainWindow
{
    Q_OBJECT

public:
    ReceiveWindow(QWidget *parent = 0);
    ~ReceiveWindow();

public slots:
    void receiveInitiated(int id, const QString &filename, const QString &mime);
    void sendInitiated(int id, const QString &filename, const QString &mime);
    void progress(int id, qint64 bytes, qint64 total);
    void completed(int id, bool error);

private slots:
    void fileSelected(const QModelIndex &index);

private:
    void handleSupportedFormatRecv(int id, const QString &filename, const QString &mime);
    bool handleSupportedFormatComplete(int id, bool error);

    QTableView *m_files;
    ReceivedFilesModel *m_model;
    QList<ReceivedFile> m_list;
};

#endif
