/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA All rights reserved.
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

#include "utils.h"

#include <QFileSystem>
#include <Qtopia>
#include <QRegExp>
#include <math.h>

bool SizeUtils::isSufficientSpace(qlonglong size, QString &neededSpace)
{
    QFileSystem packagesFs = QFileSystem::fromFileName( Qtopia::packagePath() );
    qlonglong avail = (qlonglong)packagesFs.blockSize() * packagesFs.availBlocks();
    
    if ( avail < size )
    {
        neededSpace = getSizeString(size - avail);
        return false;
    }
    return true;
}

QString SizeUtils::getSizeString( qlonglong size )
{
    double s = size;
    double scaledSize = s;
    QString suffix;
    bool skip = false;
    if ( s < 0 ) s = 0;
    if ( s < 1024 ) {
        suffix = QObject::tr("B","bytes");
        scaledSize = s;
        skip = true;
    }
    s /= 1024;
    if ( skip == false && s < 1024 ) {
        suffix = QObject::tr("KB","kilobytes");
        scaledSize = s;
        skip = true;
    }
    s /= 1024;
    if ( skip == false && s < 1024 ) {
        suffix = QObject::tr("MB","megabytes");
        scaledSize = s;
        skip = true;
    }
    s /= 1024;
    if ( skip == false && s < 1024 ) {
        suffix = QObject::tr("GB","gigabytes");
        scaledSize = s;
        skip = true;
    }
    return QString().sprintf("%0.2f",scaledSize ) + suffix;

}

qlonglong SizeUtils::parseInstalledSize( QString installedSize )
{
    QRegExp rx("^(\\d+\\.?\\d+)([km]?)$");
    installedSize = installedSize.trimmed().toLower();
    long multiplier;
    if ( rx.indexIn(installedSize) !=0 )
        return -1;

    QStringList captures = rx.capturedTexts();
    if ( !captures.count() == 3 )
        return -1;

    if ( captures[2].isEmpty()  )
        multiplier = 1;
    else if ( captures[2] == "k" )
        multiplier = 1024;
    else if ( captures[2] == "m" )
        multiplier = 1024 * 1024;
    else
        return -1;

    bool ok = false;
    double d = captures[1].toDouble( &ok );
    if ( !ok )
        return -1;
    double rounded = round( d );
    if ( floor(d) == rounded )
        rounded = rounded + 0.5;
    return (qlonglong)(rounded * multiplier) ;
}


bool LidsUtils::isLidsEnabled()
{
    return QFile::exists("/proc/sys/lids/locks");
}
