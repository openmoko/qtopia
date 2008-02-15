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
#ifndef QDSYNC_H
#define QDSYNC_H

#include <QTextBrowser>

class QCopBridge;
class TransferServer;

class QDSync : public QTextBrowser
{
    Q_OBJECT
public:
    QDSync( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~QDSync();

private slots:
    void appMessage( const QString &message, const QByteArray &data );
    void qdMessage( const QString &message, const QByteArray &data );
    void startDaemons();
    void stopDaemons();
    void ethernetGadget();
    void serialGadget();
    void gotConnection();
    void lostConnection();

private:
    void showEvent( QShowEvent *e );
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

    QCopBridge *bridge;
    TransferServer *tserver;
    QAction *serialAction;
    QAction *ethernetAction;
    bool selectDown;
    bool connected;
    bool syncing;

    enum SelectLabelState {
        Blank,
        Sync,
        Cancel,
    } selectLabelState;
};

#endif
