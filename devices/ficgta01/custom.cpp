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
#include <QFileInfo>

#include <QTextStream>
#include <QDebug>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    QFile maxBrightness;
    QString strvalue;
    if (QFileInfo("/sys/class/backlight/gta01-bl/max_brightness").exists() ) {
        //ficgta01
        maxBrightness.setFileName("/sys/class/backlight/gta01-bl/max_brightness");
    } else {
        //ficgta02
        maxBrightness.setFileName("/sys/class/backlight/pcf50633-bl/max_brightness");
    }
    if(!maxBrightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream in(&maxBrightness);
        in >> strvalue;
        maxBrightness.close();
    }
     return  strvalue.toInt();
}


QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    char cmd[80];

    int brightessSteps = qpe_sysBrightnessSteps();
    if(b > brightessSteps)
        b = brightessSteps;

    if(b == 1) {
        // dim
        b = brightessSteps / 4;
    } else if (b == -1) {
        //bright
        b = brightessSteps;
    }

    QFile brightness;
    if (QFileInfo("/sys/class/backlight/gta01-bl/brightness").exists() ) {
        brightness.setFileName("/sys/class/backlight/gta01-bl/brightness");
        //ficgta01
    } else {
        brightness.setFileName("/sys/class/backlight/pcf50633-bl/brightness");
        //ficgta02
    }

    if(!brightness.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&brightness);
        out << QString::number(b);
        brightness.close();
    }
}

