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
#ifndef __QOBEXHEADER_P_H__
#define __QOBEXHEADER_P_H__

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

#include <qobexheader.h>

#include <QHash>
#include <QDateTime>

typedef void* obex_t;
typedef void* obex_object_t;

/*!
    \internal
    \class QObexHeaderPrivate
    Private implementation class for QObexHeader.
*/

class QTOPIA_AUTOTEST_EXPORT QObexHeaderPrivate
{
public:
    enum HeaderEncoding {
        HeaderEncodingMask = 0xc0,
        HeaderUnicodeEncoding = 0x00,
        HeaderByteSequenceEncoding = 0x40,
        HeaderByteEncoding = 0x80,
        HeaderIntEncoding = 0xc0
    };

    QObexHeaderPrivate();
    ~QObexHeaderPrivate();

    void setValue(int headerId, const QVariant &data);
    QVariant value(int headerId) const;
    bool remove(int headerId);
    bool contains(int headerId) const;
    void clear();

    QList<int> keys() const;
    int size() const;
    QString toString();

    QList<int> m_ids;
    QHash<int, QVariant> m_hash;
    QByteArray m_challengeNonce;

    inline static QByteArray getChallengeNonce(const QObexHeader &header) { return header.m_data->m_challengeNonce; }

    static void dateTimeFromString(QDateTime &dateTime, const QString &timeString);
    static void stringFromDateTime(QString &timeString, const QDateTime &dateTime);

    static void unicodeBytesFromString(QByteArray &bytes, const QString &s);
    static void stringFromUnicodeBytes(QString &string, const uchar *data, uint size);

    static bool readOpenObexHeaders(QObexHeader &header, obex_t* handle, obex_object_t *obj);
    static bool writeOpenObexHeaders(obex_t* handle, obex_object_t *obj, bool fitOnePacket, const QObexHeader &header);

    static bool requestShouldFitOnePacket(QObex::Request request);

    static QString headerIdToString(int id);
};

#endif
