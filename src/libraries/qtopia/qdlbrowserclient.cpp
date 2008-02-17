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

// Local includes
#include "qdl.h"
#include "qdlbrowserclient.h"
#include "qdllink.h"
#include "qdlwidgetclient_p.h"

// Qt includes
#include <QTextBrowser>

// ============================================================================
//
// QDLBrowserClientPrivate
//
// ============================================================================

class QDLBrowserClientPrivate
{
public:
    QDLClient*              mQdlClient;
    QDLWidgetClientPrivate* mQdlWidgetClient;
    bool                    mSetText;
    bool                    mLoadLinks;
};

// ============================================================================
//
// QDLBrowserClient
//
// ============================================================================

/*!
    \class QDLBrowserClient
    \mainclass
    \brief The QDLBrowserClient class displays rich-text containing QDLLinks.

    QDLBrowserClient manages and activates QDLLinks in collaboration with
    a rich-text document in QTextBrowser and QDLClient. QDLBrowserClient hooks
    into the QTextBrowser setSource() method so that QDLLinks can be activated
    on QDL sources when a user clicks or selects the anchor in the text.

    \sa QDLClient, QDLEditClient

    \ingroup ipc
*/

/*!
    Constructs a QDLBrowserClient and attaches it to \a parent. The
    QDLClient is identified by \a name, which should be unique within a group of
    QDLClients. \a name should only contain alpha-numeric characters, underscores
    and spaces.
*/
QDLBrowserClient::QDLBrowserClient( QWidget* parent, const QString& name )
:   QTextBrowser( parent ),
    d( 0 )
{
    d = new QDLBrowserClientPrivate();
    d->mQdlClient = new QDLClient( this, name );
    d->mQdlWidgetClient = new QDLWidgetClientPrivate( this );
    d->mSetText = false;
    d->mLoadLinks = false;

    connect( qobject_cast<QTextBrowser*>( this ),
             SIGNAL(textChanged()),
             this,
             SLOT(browserTextChanged()) );

    connect( qobject_cast<QTextBrowser*>( this ),
             SIGNAL(anchorClicked(QUrl)),
             this,
             SLOT(activateLink(QUrl)) );
}

/*!
   Destroys a QDL Browser Client object.
*/
QDLBrowserClient::~QDLBrowserClient()
{
    delete d;
}

/*!
    \reimp
*/
void QDLBrowserClient::setSource( const QUrl & name )
{
    // Filter out QDLLinks that are activated
    if ( !name.toString().startsWith( QDL::ANCHOR_HREF_PREFIX ) )
        QTextBrowser::setSource( name );
}

/*!
    Loads the links in \a str into the client object.
    The \a str is the base64 encoded binary data of the links created by
    QDL::saveLinks().

    \sa verifyLinks()
*/
void QDLBrowserClient::loadLinks( const QString &str )
{
    d->mLoadLinks = true;
    QDL::loadLinks( str, QDL::clients( this ) );
}

/*!
    Activates the QDLLink specified by \a link, which should be in the form
    \c{QDL://<clientName>:<linkId>}.
*/
void QDLBrowserClient::activateLink( const QUrl& link )
{
    if ( link.toString().startsWith( QDL::ANCHOR_HREF_PREFIX ) )
        QDL::activateLink( link.toString(), QDL::clients( this ) );
}

/*!
    Verifies the correctness of the links stored by the client object. This
    method determines if QDLLinks are broken, ensures all stored links have
    anchor text in the parent widget's text, and anchor text for unstored
    links is removed from the parent widget's text.

    This method should only be called after QTextBrowser::setText() and
    loadLinks() have been called.

    \sa loadLinks()
*/
void QDLBrowserClient::verifyLinks()
{
    if ( d->mSetText && d->mLoadLinks )
        d->mQdlWidgetClient->verifyLinks( d->mQdlClient );
}

/*!
    \internal
*/
void QDLBrowserClient::browserTextChanged()
{
    d->mSetText = true;
}
