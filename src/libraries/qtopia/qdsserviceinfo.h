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

#ifndef QDS_SERVICE_INFO_H
#define QDS_SERVICE_INFO_H

// Qtopia includes
#include <Qtopia>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <QMimeType>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSServiceInfoPrivate;
class QDSServices;
class QString;

// ============================================================================
//
//  QDSServiceInfo
//
// ============================================================================

class QTOPIA_EXPORT QDSServiceInfo
{
    friend class QDSServices;

public:
    QDSServiceInfo();
    QDSServiceInfo( const QDSServiceInfo& other );
    QDSServiceInfo( const QString& name,
                    const QString& service );

    ~QDSServiceInfo();

    // Operators
    const QDSServiceInfo& operator=( const QDSServiceInfo& other );
    bool operator==( const QDSServiceInfo& other ) const;
    bool operator!=( const QDSServiceInfo& other ) const;

    // Access
    bool isValid() const;
    bool isAvailable() const;
    QString serviceId() const;
    QString serviceName() const;
    QString name() const;
    QStringList requestDataTypes() const;
    bool supportsRequestDataType(
        const QMimeType& type = QMimeType( QString() ) ) const;
    QStringList responseDataTypes() const;
    bool supportsResponseDataType(
        const QMimeType& type = QMimeType( QString() ) ) const;
    QStringList attributes() const;
    QString description() const;
    QString icon() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
private:

    bool supportsRequestDataTypeOrWild( const QString& type = QString() ) const;
    bool supportsResponseDataTypeOrWild( const QString& type = QString() ) const;

    QDSServiceInfoPrivate* d;
};

Q_DECLARE_USER_METATYPE( QDSServiceInfo );

#endif //QDS_SERVICE_INFO_H
