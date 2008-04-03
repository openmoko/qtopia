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


static QString neo1973BacklightClassFile() {
    QString maxBrightness;

    // If the gta01-bl/max_brightness exists we know it is gta01 otherwise a gta02 is assumed
    if (QFileInfo("/sys/class/backlight/gta01-bl/max_brightness").exists())
        maxBrightness = QLatin1String("/sys/class/backlight/gta01-bl/max_brightness");
    else
        maxBrightness = QLatin1String("/sys/class/backlight/pcf50633-bl/max_brightness");

    return maxBrightness;
}


QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{

    QFile maxBrightness(neo1973BacklightClassFile());

    QString strvalue;
    if(!maxBrightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "File not opened";
    } else {
        QTextStream in(&maxBrightness);
        in >> strvalue;
        maxBrightness.close();
    }

    return strvalue.toInt();
}


QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    qWarning() << "setBrightness" << b <<  qpe_sysBrightnessSteps();

    // dim(1) or bright (-1) or blank (0)? 
    if(b == 1) {
        b = 519;
    } else if (b == -1) {
        b = qpe_sysBrightnessSteps();
    } else if(b == 0) {
    } else if(b == qpe_sysBrightnessSteps()) {
    } else {
    }

    QFile brightness(neo1973BacklightClassFile());

    if(!brightness.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "File not opened";
    } else {
        QTextStream out(&brightness);
        out << QString::number(b);
        brightness.close();
    }
}

