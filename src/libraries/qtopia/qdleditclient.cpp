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

// Local includes
#include "qdl_p.h"
#include "qdleditclient.h"
#include "qdllink.h"
#include "qdlwidgetclient_p.h"

// Qtopia includes
#ifdef QTOPIA_PHONE
#include <QSoftMenuBar>
#endif //QTOPIA_PHONE

// Qt includes
#include <QTextEdit>
#include <qtopialog.h>

// System includes
#include <sys/types.h>
#include <unistd.h>

// ============================================================================
//
// QDLEditClient
//
// ============================================================================

/*!
    \class QDLEditClient
    \brief The QDLEditClient class is used to add QDLLinks to rich-text
           documents

    QDLEditClient is a subclass of QDLClient which manages the QDLLinks in
    collaboration with the rich-text in QTextEdit. This allows the anchor text
    for QDLLinks to updated as links are added, set, or removed from the
    client object.

    \sa QDLClient, QDLBrowserClient

    \ingroup qtopiaemb
*/

/*!
    Constructs a QDLEditClient with \a edit as the parent widget. The QDLClient
    is identified by \a name, which should be unique within a group of
    QDLClients. \a name should only contain alpha-numeric characters,
    underscores and spaces.
*/
QDLEditClient::QDLEditClient( QTextEdit *edit, const QString& name )
:   QDLClient( edit, name ),
    d( 0 )
{
    d = new QDLWidgetClientPrivate( edit );
}

/*!
    \internal
*/
QDLEditClient::~QDLEditClient()
{
    delete d;
}

#ifdef QTOPIA_PHONE
/*!
    Adds an "Insert Link" action item to \a context which connects to
    QDLEditClient::requestLinks().
*/
QMenu* QDLEditClient::setupStandardContextMenu( QMenu *context )
{
    // Are we a valid widget?
    if( !isValid() ) {
        qLog(DataLinking) << "BUG : trying to setup a context "
                          << "menu on a null widget client";
        return 0;
    }

    QWidget *w = d->widget();
    if ( !context )
        context = QSoftMenuBar::menuFor( w, QSoftMenuBar::AnyFocus );

    QAction *insertLinkAction = new QAction( QIcon( ":icon/qdllink" ),
                                             tr( "Insert Link" ),
                                             w );

    QObject::connect( insertLinkAction,
                      SIGNAL( triggered( bool ) ),
                      this,
                      SLOT( requestLinks() ) );

    context->addAction( insertLinkAction );

    QSoftMenuBar::addMenuTo( w, context, QSoftMenuBar::AnyFocus );
    return context;
}
#endif

/*!
    Determines if the client object is valid. Calls to methods on an invalid
    client will fail.
*/
bool QDLEditClient::isValid() const
{
    return d->isValid();
}

/*!
    Returns the hint used when requesting links.

    \sa QDLClient::setHint()
*/
QString QDLEditClient::hint() const
{
    // Are we a valid widget?
    if( !isValid() ) {
        return QString();
    }

    // If the user has manually set a hint use that
    // otherwise determine a hint(selection)
    QString h = QDLClient::hint();
    if( !h.isEmpty() )
        return h;

    return d->determineHint();
}

/*!
    Adds the link stored in \a link to the client object and inserts the anchor
    text for the link to the parent widget's text. The link Id is returned if
    the link is added correctly, otherwise 0 is returned.

    QDL sources create \a link, which is returned during
    QDLEditClient::requestLinks(). Therefore in normal usage this method isn't
    called directly by a client.
*/
int QDLEditClient::addLink( QDSData& link )
{
    // Are we a valid widget?
    if( !isValid() )
        return 0;

    // Use the QDLClient to add the link
    int linkId = QDLClient::addLink( link );

    // Now add the link text to the widget
    d->insertText( QDLClient::linkAnchorText( linkId ) );

    // Return the link ID
    return linkId;
}

/*!
    Updates the link stored identified by \a linkId with \a link, and updates
    the anchor text for the link in the parent widget's text.
*/
void QDLEditClient::setLink( const int linkId, const QDLLink& link )
{
    // Are we a valid widget?
    if( !isValid() )
        return;

    // Check that the link ID is valid
    if ( !QDLClient::validLinkId( linkId ) )
        return;

    QDLClient::setLink( linkId, link );
    QString t = d->text();

    QString anchor;
    int pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            0,
                                            anchor );
    int prevStopPos = -1;
    while ( pos != -1 ) {
        // Remove the old link
        t.remove( pos, anchor.length() );

        // Insert the new link, but only if there is no link directly before
        // this one. This handles when QTextEdit splits a link with an image
        QString anchorText = QDLClient::linkAnchorText( linkId );
        if ( pos != prevStopPos ) {
            t.insert( pos, anchorText );
            prevStopPos = pos + anchorText.length();
        }
        else {
            prevStopPos = -1;
        }

        // find the next anchor
        pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            pos + anchorText.length(),
                                            anchor );
    }

    // Set the new widget text
    d->setText( t );
}

/*!
    Removes the link stored identified by \a linkId, and updates the
    anchor text for the link in the parent widget's text.
*/
void QDLEditClient::removeLink( const int linkId )
{
    // Are we a valid widget?
    if( !isValid() )
        return;

    // Check that the link ID is valid
    if ( !QDLClient::validLinkId( linkId ) )
        return;

    QString t = d->text();

    QString anchor;
    int pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            0,
                                            anchor );
    while ( pos != -1 ) {
        // Remove the old link
        t.remove( pos, anchor.length() );

        // find the next anchor
        pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            pos,
                                            anchor );
    }

    // Set the new widget text and remove the old link from the QDLClient
    d->setText( t );
    QDLClient::removeLink( linkId );
}

/*!
    Requests QDLLinks from a selected QDL source. The user selects the
    source from a list of available QDL sources. Returned links
    from the source are automatically added to the client using
    QDLEditClient::addLink().
*/
void QDLEditClient::requestLinks()
{
    // Are we a valid widget?
    if( !isValid() )
        return;

    QDLClient::requestLinks( d->widget() );
}

/*!
    Verifies the correctness of the links stored by the client object. This
    method determines if QDLLinks are broken, ensures all stored links have
    anchor text in the parent widget's text, and anchor text for unstored
    links is removed from the parent widget's text.
*/
void QDLEditClient::verifyLinks()
{
    d->verifyLinks( this );
}

