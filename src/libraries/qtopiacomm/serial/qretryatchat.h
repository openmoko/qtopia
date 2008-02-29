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

#ifndef QRETRYATCHAT_H
#define QRETRYATCHAT_H

#include <qatchat.h>

class QRetryAtChatPrivate;

class QTOPIACOMM_EXPORT QRetryAtChat : public QObject
{
    Q_OBJECT
public:
    QRetryAtChat( QAtChat *atchat, const QString& command, int numRetries,
                  int timeout = 1000, bool deleteAfterEmit = true,
                  QObject *parent = 0 );
    ~QRetryAtChat();

signals:
    void done( bool ok, const QAtResult& result );

private slots:
    void doneInternal( bool ok, const QAtResult& result );
    void timeout();

private:
    QRetryAtChatPrivate *d;
};

#endif /* QRETRYATCHAT_H */
