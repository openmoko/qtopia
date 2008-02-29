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

#include <qtopiacomm/private/qobexcommand_p.h>

QObexCommand::QObexCommand(TYPE cmd, const QByteArray &data,
                           const QString &filename, const QString &mimetype)
{
    m_isba = true;
    m_data.data = new QByteArray(data);
    m_filename = filename;
    m_mimetype = mimetype;
    m_cmd = cmd;
    m_id = nextId();
}

QObexCommand::QObexCommand(TYPE cmd, QIODevice *device,
                           const QString &filename, const QString &mimetype)
{
    m_isba = false;
    m_data.device = device;
    m_filename = filename;
    m_mimetype = mimetype;
    m_cmd = cmd;
    m_id = nextId();
}

QObexCommand::~QObexCommand()
{
    if (m_isba)
        delete m_data.data;
}

QBasicAtomic QObexCommand::idCounter = Q_ATOMIC_INIT(1);

int QObexCommand::nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
}
