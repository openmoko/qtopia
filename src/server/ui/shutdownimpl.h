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
#ifndef SHUTDOWNIMPL_H
#define SHUTDOWNIMPL_H

#include "ui_shutdown.h"
#include <qdialog.h>

#include "qtopiaserverapplication.h"

class QTimer;
class QAbstractButton;

class ShutdownImpl : public QDialog, Ui::ShutdownDialog
{
    Q_OBJECT
public:
    ShutdownImpl( QWidget* parent = 0, Qt::WFlags fl = 0 );

signals:
    void shutdown( QtopiaServerApplication::ShutdownType );

private slots:
    void cancelClicked();
    void timeout();
public slots:
    void rebootClicked();
    void quitClicked();
    void restartClicked();
    void shutdownClicked();

private:
    void initiateAction();

    QTimer *timer;
    int progress;
    QtopiaServerApplication::ShutdownType operation;
};

#endif

