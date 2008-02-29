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

#ifndef QSERIALIODEVICE_P_H
#define QSERIALIODEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <qobject.h>
#include <qprocess.h>


class QSerialIODevice;

class QPseudoTtyProcess : public QProcess
{
    Q_OBJECT
public:
    QPseudoTtyProcess( QSerialIODevice *device, int masterFd,
                       int slaveFd, bool isPPP );
    ~QPseudoTtyProcess();

    void clearDevice() { device = 0; }
    void deviceReadyRead();

protected:
    void setupChildProcess();

private slots:
    void masterReadyRead();
    void childStateChanged( QProcess::ProcessState state );
    void deviceReady();

private:
    QSerialIODevice *device;
    int masterFd;
    int slaveFd;
    bool isPPP;
    bool readySeen;
    char buffer[1024];
};

#endif // QSERIALIODEVICE_P_H
