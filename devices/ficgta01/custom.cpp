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

#include <custom.h>
#include <qtopianamespace.h>
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>

#include <qwindowsystem_qws.h>
#include <QValueSpaceItem>
#include <QValueSpaceObject>
#include <stdio.h>
#include <stdlib.h>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>


QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    QFile maxBrightness("/sys/class/backlight/gta01-bl/max_brightness");
    QString strvalue;
    int value;
    if( !maxBrightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream in(&maxBrightness);
        in >> strvalue;
    maxBrightness.close();
    }

//     if(strvalue.isEmpty())
//         value = 3957;
//     else
        value = strvalue.toInt();

    return value;
}


QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    char cmd[80];

    qWarning() <<"setBrightness"<<b <<  qpe_sysBrightnessSteps();

    if(b == 1) {
        // dim
        b = 519;
    }
    else if (b == -1) {
        //bright
        b = qpe_sysBrightnessSteps();
//         QtopiaIpcEnvelope e("QPE/AudioVolumeManager/Ficgta01VolumeService","setAmpMode(bool)");
//         e << true;
//         qWarning()<<"amp mode on";

    }

    else if( b == 0) {
     //blanking screen
//         QtopiaIpcEnvelope e("QPE/AudioVolumeManager/Ficgta01VolumeService","setAmpMode(bool)");
//         e << false;
//       qWarning()<<"amp mode off";

    } else if( b ==  qpe_sysBrightnessSteps() ) {
//         QtopiaIpcEnvelope e("QPE/AudioVolumeManager/Ficgta01VolumeService","setAmpMode(bool)");
//         e << true;
//         qWarning()<<"amp mode on";

    }
    else {
//         QtopiaIpcEnvelope e("QPE/AudioVolumeManager/Ficgta01VolumeService","setAmpMode(bool)");
//         e << true;
//         qWarning()<<"amp mode on";

    }

    QFile brightness("/sys/class/backlight/gta01-bl/brightness");
    if( !brightness.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&brightness);
        out << QString::number(b);
        brightness.close();
    }
}

