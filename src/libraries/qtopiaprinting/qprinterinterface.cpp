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

#include <qprinterinterface.h>

/*!
    \class QtopiaPrinterInterface
    \brief The QtopiaPrinterInterface class privides interface to various printers.
    \ingroup multimedia
 */


/*!
    Destructor
 */
QtopiaPrinterInterface::~QtopiaPrinterInterface()
{
}

/*!
    \fn void print( QMap<QString, QVariant> options )

    Sends a print data to a printer. Values in \a options are used to configure the printer.
    Values are consist of a QString form of QPrintEngine::PrintEnginePropertyKey and its value.
*/

/*!
    \fn void printFile( const QString &fileName, const QString &mimeType = QString() )
    Prints \a fileName. \a mimeType is optional.
*/

/*!
    \fn void printHtml( const QString &html )
    Prints \a html.
*/

/*!
    \fn bool abort()
    Attempts to stop the printing. Sends true if printing is successfully interrupted.
*/

/*!
    \fn QString name()
    Returns a name of the printing mechanism.
*/

/*!
    \fn bool isAvailable()
    Returns true if the printing is currently supported.
*/

/*!
    Constructor
 */
QtopiaPrinterPlugin::QtopiaPrinterPlugin( QObject *parent )
    :QObject( parent )
{
}

/*!
    Destructor
 */
QtopiaPrinterPlugin::~QtopiaPrinterPlugin()
{
}

