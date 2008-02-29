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

#ifndef QDS_SERVICE_INFO_P_H
#define QDS_SERVICE_INFO_P_H

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

// Qt includes
#include <QString>
#include <QStringList>

// ============================================================================
//
//  QDSServiceInfoPrivate
//
// ============================================================================

class QDSServiceInfoPrivate
{
public:
    QDSServiceInfoPrivate();
    QDSServiceInfoPrivate( const QDSServiceInfoPrivate& other );
    QDSServiceInfoPrivate( const QString& name,
                           const QString& service );

    void processSettings();
    bool supportsDataType( const QStringList& supported, const QString& type );
    bool correctQtopiaServiceDescription();

    // Data members
    QString             mService;
    QString             mName;
    QString             mUiName;
    QStringList         mRequestDataTypes;
    QStringList         mResponseDataTypes;
    QStringList         mAttributes;
    QStringList         mDepends;
    QString             mDescription;
    QString             mIcon;
    bool                mProcessed;
    bool                mValid;
};

#endif //QDS_SERVICE_INFO_P_H
