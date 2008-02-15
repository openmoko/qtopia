/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef CALLER_H
#define CALLER_H

#include <QMainWindow>
#include <QPhoneCall>
#include <QPhoneCallManager>
#include "ui_caller.h"

class Caller : public QWidget, public Ui::Form
{
    Q_OBJECT
public:
    Caller(QWidget *parent = 0, Qt::WindowFlags flags = 0);

private slots:
    void dial();
    void dialData();
    void accept();
    void stateChanged(const QPhoneCall& call);
    void dataStateChanged(const QPhoneCall& call);
    void newCall(const QPhoneCall& call);
    void dataReady();
    void dataClosed();

private:
// Left-aligned for correct indenting in docs.
QPhoneCallManager *mgr;

    QIODevice *dataDevice;

    void dataWrite(const char *buf, int len);
};

#endif
