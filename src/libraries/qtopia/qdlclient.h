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

#ifndef QDL_CLIENT_H
#define QDL_CLIENT_H

// Local includes
#include "qdllink.h"

// Qtopia includes
#include <Qtopia>

// Qt includes
#include <QObject>
#include <QMap>
#include <QByteArray>

// Forward class declarations
class QDLClientPrivate;
class QDSData;
class QDSServiceInfo;

// ============================================================================
//
// QDLClient
//
// ============================================================================

class QTOPIA_EXPORT QDLClient : public QObject
{
    Q_OBJECT

public:
    QDLClient( QObject *parent, const QString& name );
    virtual ~QDLClient();

    // Access
    void saveLinks( QDataStream& stream ) const;
    QDLLink link( const int linkId ) const;
    QList<int> linkIds() const;
    QString hint() const;
    QString linkAnchorText( const int linkId, const bool noIcon = false ) const;
    bool validLinkId( const int linkId ) const;

    // Modification
    void setHint( const QString& hint );
    virtual void loadLinks( QDataStream& stream );
    virtual int addLink( QDSData& link );
    virtual void setLink( const int linkId, const QDLLink& link );
    virtual void removeLink( const int linkId );
    void breakLink( const int linkId, const bool broken = true );

public slots:
    void clear();
    void requestLinks( QWidget* parent );
    void requestLinks( const QDSServiceInfo& qdlService );
    void activateLink( const int linkId );
    virtual void verifyLinks();

private:
    QDLClientPrivate* d;
};

#endif //QDL_CLIENT_H
