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
#ifndef BSCIDRM_H
#define BSCIDRM_H

#include <stdlib.h>
#include <stdio.h>
#include "bsci.h"
#include "bsciMMI.h"
#include "bsciLibMgmt.h"
#include "bsciLicMgmt.h"
#include "bsciContentAccess.h"
#include "bsciContentMgmt.h"
#include <qdrmrights.h>


class BSciDrm
{
public:
    static void initialiseAgent( const QString &id, SBSciCallbacks *callbacks );

    static void releaseAgent();

    static EPermission transformPermission( QDrmRights::Permission permissionType );

    static QString formatInterval( const SBSciDuration &duration );

    static QDrmRights constraints( QDrmRights::Permission permission, ERightsStatus status, SBSciConstraints *constraints );

    static bool isStateful( const SBSciConstraints &constraints );

    static const char *getError( int error );

    static void printError( int error, const QString &method, const QString &filePath );

    static void printError( int error, const QString &method );

    static bool hasRights( const QString &dcfFilePath, QDrmRights::Permission permission );

    static QString getMetaData( const QString &filePath, enum EMetaData item );

    static qint64 getContentSize( const QString &dcfFilePath );

    static QString getPreviewUri( const QString &dcfFilePath );

    static QStringList convertTextArray( const SBSciTextArray &text );

    static const QByteArray formatPath( const QString &filePath );

    static void *context;
};

#endif
