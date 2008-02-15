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

#ifndef ATBLUETOOTHCOMMANDS_H
#define ATBLUETOOTHCOMMANDS_H

#include <QObject>

class AtCommands;
class AtInterface_HandsfreeIpcAdaptor;

class AtBluetoothCommands : public QObject
{
    Q_OBJECT

public:
    AtBluetoothCommands( AtCommands * parent );
    ~AtBluetoothCommands();

    void setSpeakerVolume(int);
    void setMicrophoneVolume(int);

public slots:
    void atbldn();
    void atbrsf( const QString& params );
    void atvgm(const QString &params);
    void atvgs(const QString &params);
    void atnrec(const QString &params);
    void atbvra(const QString &params);
    void atbinp(const QString &params);
    void atbtrh(const QString &params);

private:
    AtCommands *atc;
    AtInterface_HandsfreeIpcAdaptor *m_adaptor;

};

#endif // ATBLUETOOTHCOMMANDS_H

