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



#ifndef QTMailWindow_H
#define QTMailWindow_H

#include <qevent.h>
#include "emailclient.h"
#include "readmail.h"

class MailListView;
class WriteMail;
class StatusDisplay;

class QStackedWidget;

class QTMailWindow: public QMainWindow
{
    Q_OBJECT
public:
    QTMailWindow(QWidget *parent = 0, Qt::WFlags fl = 0);
    ~QTMailWindow();
    static QTMailWindow *singleton();
    void forceHidden(bool hidden);
    void setVisible(bool visible);
    QWidget* currentWidget() const;

public slots:
    void raiseWidget(QWidget *, const QString &);

    void closeEvent(QCloseEvent *e);
    void setDocument(const QString &);

protected:
    void init();

    EmailClient *emailClient;
    QStackedWidget *views;
    StatusDisplay *status;
    bool noShow;

    static QTMailWindow *self; //singleton
};

#endif
