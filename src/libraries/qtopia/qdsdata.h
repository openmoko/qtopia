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

#ifndef QDS_DATA_H
#define QDS_DATA_H

// Qtopia includes
#include <Qtopia>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSDataPrivate;
class QUniqueId;
class QByteArray;
class QMimeType;
class QString;
class QFile;
class QDataStream;

// ============================================================================
//
//  QDSData
//
// ============================================================================

class QTOPIA_EXPORT QDSData
{
public:
    QDSData();
    QDSData( const QDSData& other );
    explicit QDSData( const QUniqueId& key );
    QDSData( const QByteArray& data, const QMimeType& type );
    QDSData( const QString& data, const QMimeType& type );
    QDSData( QFile& data, const QMimeType& type );

    ~QDSData();

    // Operators
    const QDSData& operator=( const QDSData& other );
    bool operator==( const QDSData& other ) const;
    bool operator!=( const QDSData& other ) const;

    // Access
    bool isValid() const;
    QMimeType type() const;
    QByteArray data() const;
    QString toString() const;
    QIODevice* toIODevice() const;

    // Modification
    QUniqueId store();
    bool remove();
    bool modify( const QByteArray& data );
    bool modify( const QString& data );
    bool modify( QFile& data );
    bool modify( const QByteArray& data, const QMimeType& type );
    bool modify( const QString& data, const QMimeType& type );
    bool modify( QFile& data, const QMimeType& type );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QDSDataPrivate* d;
};

Q_DECLARE_USER_METATYPE( QDSData );


#endif //QDS_DATA_H
