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

#include "qmediasessionbuilder.h"


/*!
    \class QMediaSessionBuilder
    \preliminary
    \brief The QMediaSessionBuilder class is a base class to be used by Media Engines
            which need to create sessions within the Media Server.

    A Media Engine should make available at least one QMediaSessionBuilder to be used
    in the construction of media sessions.

    QMediaSessionBuilder is not instantiated directly, it should be inherited
    and its methods overridden to provide the required functionality.

    \code
    Example
    {
        class UrlSessionBuilder : public QMediaSessionBuilder
        {
        public:
            UriSessionBuilder()
            {
                // create m_attributes, perhaps from args
            }

            ~UriSessionBuilder() {}

            QString type() { return "com.trolltech.qtopia.uri"; }
            Attributes const& attributes() { return m_attributes; }

            QMediaServerSession* createSession(QMediaSessionRequest request)
            {
                QUrl    url;
                request >> url;
                // do something
                return session;
            }

            void destroySession(QMediaServerSession* session)
            {
                delete session;
            }

        private:
            Attributes      m_attributes;
        };
    }
    \endcode

    \sa QMediaEngine
    \sa QMediaSessionRequest
    \sa QMediaServerSession

    \ingroup multimedia
*/

/*!
    \typedef QMediaSessionBuilder::Attributes

    A QMap of QString to QVariant. Attributes are generic name value pairs for
    use in communicating static information about the builder.
*/

/*!
    Destruct a QMediaSessionBuilder.
*/

QMediaSessionBuilder::~QMediaSessionBuilder()
{
}

/*!
    \fn QString QMediaSessionBuilder::type() const;

    Returns a string used as an identifier for the type of builder. The string
    should be of the form reverse-dns.local-type. Example:
    com.trolltech.qtopia.uri.
*/

/*!
    \fn Attributes const& QMediaSessionBuilder::attributes() const;

    Returns a QMap of Attribute Name and Value pairs. This may be used for
    communication of some builder type information to a builder type manager,
    or for custom builder types may just be information that is useful to be
    presented into the Qtopia Value-space.

    \sa QValueSpaceItem
*/

/*!
    \fn QMediaServerSession* QMediaSessionBuilder::createSession(QMediaSessionRequest sessionRequest);

    This function should create a QMediaServerSession, if able, based upon the
    information in the session request. This function will be called if the
    type of the QMediaSessionRequest \a sessionRequest matches the type of the
    builder. Additionally, if the builder is one of the common builder types it
    will only be called if it matches extra criteria, as defined by the builder
    type, for example, the mime type of the content.

    \sa QMediaSessionRequest
    \sa QMediaServerSession
*/

/*!
    \fn void QMediaSessionBuilder::destroySession(QMediaServerSession* serverSession);

    This function is called when the session \a serverSession is no longer
    necessary. The builder will only be asked to destroy sessions that it
    created. The builder must destroy the session at this time.
*/

/*!
    \typedef QMediaSessionBuilderList

    A QList of QMediaSessionBuilder. A convenience for dealing with standard
    collections of builders.
*/

