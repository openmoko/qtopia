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
#ifndef QTOPIADOCUMENTSERVER_H
#define QTOPIADOCUMENTSERVER_H
#include <QObject>
#include <qtopiaglobal.h>

class QtopiaDocumentServerPrivate;

class QTOPIA_EXPORT QtopiaDocumentServer : public QObject
{
    Q_OBJECT
public:
    QtopiaDocumentServer( QObject *parent = 0 );

    virtual ~QtopiaDocumentServer();

    void shutdown();

signals:
    void shutdownComplete();

private slots:
    void threadFinished();

private:
    QtopiaDocumentServerPrivate *d;
};

#endif
