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

#include "qmailviewerplugin.h"

#include <QApplication>
#include <qtopialog.h>

/*!
    \class QMailViewerPluginInterface
    \mainclass
    \brief The QMailViewerPluginInterface class defines the interface to plug-ins that provide mail message viewers.
    \ingroup messaginglibrary

    The QMailViewerPluginInterface class defines the interface to mail message viewer plug-ins.  Plug-ins will 
    typically inherit from QMailViewerPlugin rather than this class.

    \sa QMailViewerPlugin, QMailViewerInterface, QMailViewerFactory
*/

/*!
    \fn QString QMailViewerPluginInterface::key() const

    Returns a string identifying this plug-in.
*/

/*!
    \fn bool QMailViewerPluginInterface::isSupported( QMailViewerFactory::ContentType type ) const

    Returns true if the viewer provided by this plug-in can display a mail message with \a type content; otherwise returns false.
*/

/*!
    \fn QMailViewerInterface* QMailViewerPluginInterface::create( QWidget* parent )

    Creates an instance of the message viewer class provided by the plug-in, setting the returned object to 
    have the parent widget \a parent.
*/

/*!
    \class QMailViewerPlugin
    \mainclass
    \brief The QMailViewerPlugin class defines a base class for implementing mail message viewer plug-ins.
    \ingroup messaginglibrary

    The QMailViewerPlugin class provides a base class for plug-in classes that provide mail message viewing
    functionality.  Classes that inherit QMailViewerPlugin need to provide overrides of the
    \l {QMailViewerPlugin::key()}{key()}, \l {QMailViewerPlugin::isSupported()}{isSupported()} and 
    \l {QMailViewerPlugin::create()}{create()} member functions.

    \sa QMailViewerPluginInterface, QMailViewerInterface
*/

/*!
    Create a mail message viewer plug-in instance.
*/
QMailViewerPlugin::QMailViewerPlugin()
{
}

/*!
    Destructs the QMailViewerPlugin object.
*/
QMailViewerPlugin::~QMailViewerPlugin()
{
}

/*!
    Returns the list of interfaces implemented by this plug-in.
*/
QStringList QMailViewerPlugin::keys() const
{
    QStringList list;
    return list << "QMailViewerPluginInterface";
}

