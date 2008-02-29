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

#ifndef QPRINTINGINTERFACE_H
#define QPRINTINGINTERFACE_H

#include <qtopiaglobal.h>
#include <QMap>
#include <qcontent.h>

class QTOPIAPRINTING_EXPORT QtopiaPrinterInterface
{
public:
    virtual ~QtopiaPrinterInterface();

    virtual void print( QMap<QString, QVariant> options ) = 0;
    virtual void printFile( const QString &fileName, const QString &mimeType = QString() ) = 0;
    virtual void printHtml( const QString &html ) = 0;
    virtual bool abort() = 0;
    virtual QString name() = 0;
    virtual bool isAvailable() = 0;

protected:
    int m_opType;
};

Q_DECLARE_INTERFACE(QtopiaPrinterInterface,
        "com.trolltech.Qtopia.QtopiaPrinterInterface")

class QTOPIAPRINTING_EXPORT QtopiaPrinterPlugin : public QObject, public QtopiaPrinterInterface
{
public:
    explicit QtopiaPrinterPlugin( QObject *parent = 0 );
    virtual ~QtopiaPrinterPlugin();
};

#endif

