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

#ifndef ATV250COMMANDS_H
#define ATV250COMMANDS_H

#include <QObject>

class AtCommands;

class AtV250Commands : public QObject
{
    Q_OBJECT

public:
    AtV250Commands( AtCommands * parent );
    ~AtV250Commands();

public slots:
    void ata();
    void ate( const QString& params );
    void atgcap();
    void atgmi( const QString& params );
    void atgmm( const QString& params );
    void atgmr( const QString& params );
    void atgsn( const QString& params );
    void ath( const QString& params );
    void ati( const QString& params );
    void ato();
    void atq( const QString& params );
    void ats3( const QString& params );
    void ats4( const QString& params );
    void ats5( const QString& params );
    void atv( const QString& params );
    void atz();
    void atampf();
    void atampw();

private:
    int soption( const QString& params, int prev, int min, int max );

    AtCommands *atc;

};

#endif // ATV250COMMANDS_H

