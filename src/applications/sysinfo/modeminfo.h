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

#ifndef MODEMINFO_H
#define MODEMINFO_H

#include <QWidget>

class QTextBrowser;
class ModemInfo : public QWidget
{
    Q_OBJECT
public:
    ModemInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ModemInfo();

    bool eventFilter( QObject* watched, QEvent *event );

private slots:
    void configValue( const QString& name, const QString& value );
    void init();

private:
    QTextBrowser* infoDisplay;
    QString manufacturer;
    QString model;
    QString revision;
    QString serial;
    QString extraVersion;

    QString format();
};

#endif
