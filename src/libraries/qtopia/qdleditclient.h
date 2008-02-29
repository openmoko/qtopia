/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef QDL_EDIT_CLIENT_H
#define QDL_EDIT_CLIENT_H

// Local includes
#include "qdlclient.h"

// Qt includes
#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <QMenu>
#endif //QTOPIA_KEYPAD_NAVIGATION

// Forward class declarations
class QDLLink;
class QDLWidgetClientPrivate;
class QTextEdit;

// ============================================================================
//
// QDLEditClient
//
// ============================================================================

class QTOPIA_EXPORT QDLEditClient : public QDLClient
{
    Q_OBJECT

public:
    QDLEditClient( QTextEdit *edit, const QString& name );
    virtual ~QDLEditClient();

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *setupStandardContextMenu( QMenu *context = 0 );
#endif

    // Access
    bool isValid() const;
    QString hint() const;

    // Modification
    virtual int addLink( QDSData& link );
    virtual void setLink( const int linkId, const QDLLink &link );
    virtual void removeLink( const int linkId );

public slots:
    void requestLinks();
    void verifyLinks();

private:

    QDLWidgetClientPrivate *d;
};

#endif //QDL_EDIT_CLIENT_H
