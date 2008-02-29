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

// Local includes
#include "qds_p.h"
#include "qdsservices.h"
#include "qdsserviceinfo.h"

// Qtopia includes
#include <QMimeType>

// QT includes
#include <QSettings>

// ============================================================================
//
//  QDSServices
//
// ============================================================================

/*!
    \class QDSServices
    \brief The QDSServices class finds available QDS services.

    QDSServices can used to search for available QDS services. The search can
    be filtered on a combination of request data type, response data type or
    Qtopia service name.

    For example if you wanted to find a QDS service which could convert a jpeg
    image into a bitmap image you would do the following:

    \code
    QDSServices service( "image/jpeg", "image/bmp" )

    if ( service.count() == 0 )
        qWarning() << "No jpeg to bitmap image conversion service available";
    \endcode

    The data types can include a wildcard to match across multiple types, i.e.
    for a request data type filter of \c "text*" the search would find services
    which had a request data type of \c "text/plain", \c "text/html", and so on.
    A null data type is used to identify no request or response data.

    Qtopia service names may also use wildcards, e.g. \c "MyApp*", but attributes
    do not support wildcards.

    \sa QDSServiceInfo

    \ingroup ipc
*/

/*!
    Finds all QDS services available on the device which have a request data type
    \a requestDataType, a response data type of \a responseDataType (both case
    insensitive), contains all the attributes listed in \a attributes (case insensitive),
    and uses Qtopia services which match \a service (case sensitive).
*/
QDSServices::QDSServices( const QString& requestDataType,
                          const QString& responseDataType,
                          const QStringList& attributes,
                          const QString& service )
{
    QDir files( Qtopia::qtopiaDir()+QLatin1String("etc/qds"), service, QDir::Unsorted, QDir::Files );
    QStringList filesList = files.entryList();
    if ( filesList.count() == 0 )
        return;


    // Find QDS services in each
    foreach ( QString qdsService, filesList ) {
        processQdsServiceFile( qdsService,
                               requestDataType,
                               responseDataType,
                               attributes );
    }
}

/*!
    Finds and returns the first QDS service with name \a name (case sensitive) in
    the list. If no such service exists an invalid QDSServiceInfo is returned.
*/
QDSServiceInfo QDSServices::findFirst( const QString& name )
{
    foreach (QDSServiceInfo serviceInfo, *this) {
        if ( serviceInfo.name() == name )
            return serviceInfo;
    }

    return QDSServiceInfo();
}


/*!
    \internal
*/
void QDSServices::processQdsServiceFile( const QString service,
                                         const QString& requestDataTypeFilter,
                                         const QString& responseDataTypeFilter,
                                         const QStringList& attributesFilter )
{
    QSettings serviceFile( Qtopia::qtopiaDir() + QLatin1String("etc/qds/") + service, QSettings::IniFormat );
    QStringList qdsServices = serviceFile.childGroups();
    foreach ( QString name, qdsServices ) {
        if ( name == QLatin1String("QDSInformation") ) //the group QDSInformation is reserved
            continue;
        if ( name == QLatin1String("Translation") ) //skip translations
            continue;
        QDSServiceInfo serviceInfo( name, service );
        if ( serviceInfo.isValid() &&
             passTypeFilter( serviceInfo, requestDataTypeFilter, Request ) &&
             passTypeFilter( serviceInfo, responseDataTypeFilter, Response ) &&
             passAttributesFilter( serviceInfo, attributesFilter ) )
        {
            append( serviceInfo );
        }
    }
}

bool QDSServices::passTypeFilter( const QDSServiceInfo& serviceInfo,
                                  const QString& typeFilter,
                                  const Mode mode )
{
    if ( typeFilter == "*" )
        return true;

    if ( mode == Request )
        return serviceInfo.supportsRequestDataTypeOrWild( typeFilter );
    else if ( mode == Response )
        return serviceInfo.supportsResponseDataTypeOrWild( typeFilter );

    return false;
}

bool QDSServices::passAttributesFilter( const QDSServiceInfo& serviceInfo,
                                        const QStringList& attributesFilter )
{
    const QStringList& attributes = serviceInfo.attributes();

    foreach( QString attribute, attributesFilter ) {
        if ( !attributes.contains( attribute, Qt::CaseInsensitive ) ) {
            return false;
        }
    }

    return true;
}
