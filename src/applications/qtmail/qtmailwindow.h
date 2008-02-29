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



#ifndef QTMailWindow_H
#define QTMailWindow_H

#ifdef QTOPIA_DESKTOP
#include "qtmailgui_qd.h"
#else
#include "qtmailgui.h"
#endif
#include <qstackedwidget.h>
#include <qevent.h>
#include "emailclient.h"
#include "readmail.h"

class MailListView;
class WriteMail;

class QTMailWindow: public QTMailGui
{
    Q_OBJECT
public:
#ifdef QTOPIA_DESKTOP
    QTMailWindow();
#else
    QTMailWindow(QWidget *parent = 0, Qt::WFlags fl = 0);
#endif
    ~QTMailWindow();
    static QTMailWindow *singleton();
    void forceHidden(bool hidden);
    void setVisible(bool visible);

public slots:
    void raiseWidget(QWidget *, const QString &);

    void closeEvent(QCloseEvent *e);
    void setDocument(const QString &);

protected:
    void init();

    EmailClient *emailClient;
    QStackedWidget *views;
    bool mOpenedByUser;
    static QTMailWindow *self; //singleton
    QWidget *parentWidget;
    bool noShow;
};

#endif
