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
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QFrame>
#include <QMap>

class QFrame;
class QTimer;
class QHBoxLayout;
class QLabel;
class QString;

// QStatusBar sucks!
class StatusBar : public QFrame
{
    Q_OBJECT
public:
    StatusBar( QWidget *parent = 0 );
    virtual ~StatusBar();

    void addWidget( QWidget *widget, int stretch = 0, bool permanent = false );
    void removeWidget( QWidget *widget );

public slots:
    void showMessage( const QString &message, int timeout = 0 );

signals:
    void clicked();

private:
    QMap<QWidget*,QFrame*> widgets;
    QLabel *statusLabel;
    QTimer *statusClearer;
    QHBoxLayout *layout;
};

#endif
