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

#ifndef QDS_SERVICES_H
#define QDS_SERVICES_H

// Local includes
#include "qdsserviceinfo.h"

// Qt includes
#include <QList>

// Qtopia includes
#include <Qtopia>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QMimeType;
class QString;

// ============================================================================
//
//  QDSServices
//
// ============================================================================

class QTOPIA_EXPORT QDSServices : public QList<QDSServiceInfo>
{
public:
    explicit QDSServices( const QString& requestDataType = QString( "*" ),
                          const QString& responseDataType = QString( "*" ),
                          const QStringList& attributes = QStringList(),
                          const QString& service = QString( "*" ) );

    QDSServiceInfo findFirst( const QString& name );

private:
    enum Mode { Request, Response };

    void processQdsServiceFile( const QString service,
                                const QString& requestDataTypeFilter,
                                const QString& responseDataTypeFilter,
                                const QStringList& attributesFilter );
    bool passTypeFilter( const QDSServiceInfo& serviceInfo,
                         const QString& typeFilter,
                         const Mode mode );
    bool passAttributesFilter( const QDSServiceInfo& serviceInfo,
                               const QStringList& attributesFilter );

};

#endif //QDS_SERVICES_H
