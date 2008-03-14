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
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{


    QFile maxBrightness;
    if (QFileInfo("/sys/devices/platform/omapfb/panel/backlight_max").exists() ) {
        maxBrightness.setFileName("/sys/devices/platform/omapfb/panel/backlight_max");
    }

    QString strvalue;
    int value;
    if( !maxBrightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"max brightness File not opened";
    } else {
        QTextStream in(&maxBrightness);
        in >> strvalue;
        maxBrightness.close();
    }

    value = strvalue.toInt();

    return value;
}


QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    char cmd[80];
    int steps =  qpe_sysBrightnessSteps();
    qWarning() <<"setBrightness"<<b << steps;

    if(b == 1) {
        // dim
        b = steps - (steps/4);
    }
    else if (b == -1) {
        //bright
        b = steps;
    }

    else if(b == 0) {

    } else if(b == steps) {
    }
    else {

    }

    QFile brightness;
    if (QFileInfo("/sys/devices/platform/omapfb/panel/backlight_level").exists() ) {
        brightness.setFileName("/sys/devices/platform/omapfb/panel/backlight_level");
    }

    if( !brightness.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"brightness File not opened";
    } else {
        QTextStream out(&brightness);
        out << QString::number(b);
        brightness.close();
    }
}
