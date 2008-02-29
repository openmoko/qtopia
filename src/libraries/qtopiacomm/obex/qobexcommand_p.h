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

#ifndef __QOBEXCOMMANDS_P_H__
#define __QOBEXCOMMANDS_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qobexnamespace.h>
#include <qobexheader.h>

#include <QString>
#include <qatomic.h>

class QIODevice;
class QByteArray;

class QObexCommand
{
    public:
        QObexCommand(QObex::Request req, const QObexHeader &header,
                     const QByteArray &data);
        QObexCommand(QObex::Request req,
                     const QObexHeader &header = QObexHeader(),
                     QIODevice *device = 0);
        ~QObexCommand();

        QObex::Request m_req;
        QObexHeader m_header;
        QObex::SetPathFlags m_setPathFlags;
        int m_id;

    // Concept stolen from QFTP
        union {
            QIODevice *device;
            QByteArray *data;
        } m_data;
        bool m_isba;

#if QT_VERSION < 0x040400
        static QBasicAtomic idCounter;
#else
        static QAtomicInt idCounter;
#endif
        static int nextId();

    private:
        Q_DISABLE_COPY(QObexCommand)
};

#endif
