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

#ifndef __QOBEXCOMMANDS_P_H__
#define __QOBEXCOMMANDS_P_H__

#include <QString>
#include <QAtomic>

class QIODevice;
class QByteArray;

class QObexCommand
{
public:

    enum TYPE {
        CONNECT = 0,
        DISCONNECT,
        PUT,
        GET
    };

    QObexCommand(TYPE cmd, const QByteArray &data,
                 const QString &filename, const QString &mimetype);
    QObexCommand(TYPE cmd, QIODevice *device,
                 const QString &filename, const QString &mimetype);

    ~QObexCommand();

    QString m_mimetype;
    QString m_filename;

    // Concept stolen from QFTP
    union {
        QIODevice *device;
        QByteArray *data;
    } m_data;
    bool m_isba;

    TYPE m_cmd;
    int m_id;

    static QBasicAtomic idCounter;
    static int nextId();
};

#endif
