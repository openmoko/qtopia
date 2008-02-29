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

#ifndef FAXPRINTERPLUGIN_H
#define FAXPRINTERPLUGIN_H

#include <qprinterinterface.h>

class BluetoothPrinterPluginPrivate;

class QTOPIA_PLUGIN_EXPORT BluetoothPrinterPlugin : public QtopiaPrinterPlugin
{
    Q_OBJECT
public:
    BluetoothPrinterPlugin( QObject *parent = 0 );
    ~BluetoothPrinterPlugin();

    void print( QMap<QString, QVariant> options );
    void printFile( const QString &fileName, const QString &mimeType = QString() );
    void printHtml( const QString &html );
    bool abort();
    bool isAvailable();
    QString name();

private:
    BluetoothPrinterPluginPrivate *d;
};

#endif
