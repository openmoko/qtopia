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

#ifndef SECURITYINFO_H
#define SECURITYINFO_H

#include <QWidget>
class QTextBrowser;
class SecurityInfo : public QWidget
{
    Q_OBJECT
public:
    SecurityInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~SecurityInfo();

    bool eventFilter( QObject* watched, QEvent *event );
private:
    QTextBrowser* infoDisplay;
private slots:
    void init();
};

#endif
