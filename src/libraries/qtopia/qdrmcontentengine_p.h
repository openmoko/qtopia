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
#ifndef QDRMCONTENTENGINE_P_H
#define QDRMCONTENTENGINE_P_H

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

#include <qtopia/private/qfscontentengine_p.h>

class QDrmContentEngine : public QFSContentEngine
{
public:
    QDrmContentEngine();
    virtual ~QDrmContentEngine();

    virtual QIODevice *open( QIODevice::OpenMode mode );

    virtual bool execute( const QStringList &arguments ) const;

    virtual QDrmRights::Permissions queryPermissions();

    virtual QDrmRights rights( QDrmRights::Permission ) const;

    virtual bool canActivate() const;

    virtual bool activate( QDrmRights::Permission permission, QWidget *parent );

    virtual bool reactivate( QDrmRights::Permission permission, QWidget *parent );

    virtual QDrmContentLicense *requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options );

    virtual QContentEngine *createCopy() const;

protected:
    QDrmContentEngine( const QString &engineType );
};

#endif
