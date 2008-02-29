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

#ifndef __QOBEXCOMMON_P_H__
#define __QOBEXCOMMON_P_H__

#include <QString>
#include <qglobal.h>
#include <QMap>
#include <QVariant>

#include <openobex/obex.h>

class QObexHeader
{
public:
    enum HeaderType { MIMETYPE, NAME, LENGTH, DESCRIPTION };
    QObexHeader();

    inline QString mimeType() const
    {
        return m_map.value(QObexHeader::MIMETYPE).value<QString>();
    }

    inline void setMimeType(const QString &mimetype)
    {
        m_map.insert(QObexHeader::MIMETYPE, QVariant::fromValue(mimetype));
    }

    inline QString name() const
    {
        return m_map.value(QObexHeader::NAME).value<QString>();
    }

    inline void setName(const QString &name)
    {
        m_map.insert(QObexHeader::NAME, QVariant::fromValue(name));
    }

    inline qint64 length() const
    {
        return m_map.value(QObexHeader::LENGTH).value<qint64>();
    }

    inline void setLength(qint64 len)
    {
        m_map.insert(QObexHeader::LENGTH, QVariant::fromValue(len));
    }

    inline QString description() const
    {
        return m_map.value(QObexHeader::DESCRIPTION).value<QString>();
    }

    inline void setDescription(const QString &description)
    {
        m_map.insert(QObexHeader::DESCRIPTION, QVariant::fromValue(description));
    }

private:
    QMap<HeaderType, QVariant> m_map;
};

void getHeaders( obex_t *self, obex_object_t *object, QObexHeader &header );
void setHeaders( obex_t *self, obex_object_t *object, const QObexHeader &header );

#define OBEX_STREAM_BUF_SIZE 4096

#endif
