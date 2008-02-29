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
#ifndef CONNECTIONSTATUSWIDGET_H
#define CONNECTIONSTATUSWIDGET_H

#include <QWidget>

class QPushButton;

class ConnectionStatusWidget : public QWidget
{
    Q_OBJECT
public:
    ConnectionStatusWidget( QWidget *parent = 0 );
    ~ConnectionStatusWidget();

private slots:
    void setState( int state );
    void connectionMessage( const QString &message, const QByteArray &data );
    void statusClicked();
    void hintClicked();

private:
    void showEvent( QShowEvent *e );

    QString section;
    QPushButton *statusBtn;
    QPushButton *hintBtn;
    int mState;
};

#endif
