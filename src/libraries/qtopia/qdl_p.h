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

#ifndef QDLPRIVATE_H
#define QDLPRIVATE_H

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

// Forward class declarations
class QDLLink;
class QString;

// ============================================================================
//
// QDLPrivate
//
// ============================================================================

class QDLPrivate
{
public:
    static QString linkAnchorText( const QString& clientName,
                                   const int linkId,
                                   const QDLLink& link,
                                   const bool noIcon = false );

    static int indexOfQDLAnchor( const QString& text,
                                 const int startPos,
                                 QString& anchor );

    static int indexOfQDLAnchor( const QString& text,
                                 const QString& clientName,
                                 const int linkId,
                                 const int startPos,
                                 QString& anchor );

    static QString anchorToHref( const QString& anchor );

    static bool decodeHref( const QString& href,
                            QString& clientName,
                            int& linkId );
};

#endif // QDLPRIVATE_H
