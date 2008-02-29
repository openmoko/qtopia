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

#ifndef QDL_BROWSER_CLIENT_H
#define QDL_BROWSER_CLIENT_H

// Local includes
#include "qdlclient.h"

// Qt includes
#include <QTextBrowser>

// Forward class declarations
class QDLBrowserClientPrivate;

// ============================================================================
//
// QDLBrowserClient
//
// ============================================================================

class QTOPIA_EXPORT QDLBrowserClient : public QTextBrowser
{
    Q_OBJECT

public:
    QDLBrowserClient( QWidget* parent, const QString& name );
    virtual ~QDLBrowserClient();

    // QTextBrowser overloads
    virtual void setSource( const QUrl & name );

    // Modification
    void loadLinks( const QString &str );

public slots:
    void activateLink( const QUrl& link );
    void verifyLinks();

private slots:
    void browserTextChanged();

private:
    QDLBrowserClientPrivate *d;
};

#endif //QDL_BROWSER_CLIENT_H
