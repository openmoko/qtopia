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

#ifndef QDL_H
#define QDL_H

// Qtopia includes
#include <Qtopia>

// Qt includes
#include <QList>

// Forward class declarations
class QDataStream;
class QDLClient;
class QObject;

// ============================================================================
//
// functions
//
// ============================================================================

QTOPIA_EXPORT QDataStream& operator>>( QDataStream& stream,
                                       QList<QDLClient *>& clientList );

QTOPIA_EXPORT QDataStream& operator<<( QDataStream& stream,
                                       const QList<QDLClient *>& clientList );

// ============================================================================
//
// QDL
//
// ============================================================================

class QTOPIA_EXPORT QDL
{
public:
    static QList<QDLClient *> clients( QObject *parent );

    static void saveLinks( QString &str, QList<QDLClient *> clientList );
    static void loadLinks( const QString &str, QList<QDLClient *> clientList );
    static void releaseLinks( const QString& str );

    static void activateLink( const QString &href,
                              const QList<QDLClient *> &clientList );

    static const QString ANCHOR_HREF_PREFIX;

    static const QString CLIENT_DATA_KEY;
    static const QString SOURCE_DATA_KEY;
};

#endif // QDL_H
