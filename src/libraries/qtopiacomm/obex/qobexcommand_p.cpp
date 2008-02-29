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
#include <private/qobexcommand_p.h>
#include <qobexheader.h>

#include <QIODevice>

QObexCommand::QObexCommand(QObex::Request req, const QObexHeader &header,
        const QByteArray &data)
    : m_req(req),
      m_header(header),
      m_setPathFlags(0)
{
    m_isba = true;
    m_data.data = new QByteArray(data);

    m_id = nextId();
}

QObexCommand::QObexCommand(QObex::Request req, const QObexHeader &header,
                           QIODevice *device)
    : m_req(req),
      m_header(header),
      m_setPathFlags(0)
{
    m_isba = false;
    m_data.device = device;

    m_id = nextId();
}

QObexCommand::~QObexCommand()
{
    if (m_isba)
        delete m_data.data;
}

#if QT_VERSION < 0x040400
QBasicAtomic QObexCommand::idCounter = Q_ATOMIC_INIT(1);
#else
QAtomicInt QObexCommand::idCounter(1);
#endif

int QObexCommand::nextId()
{
#if QT_VERSION < 0x040400
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
#else
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSetOrdered(id, id + 1))
            break;
    }
    return id;
#endif
}
