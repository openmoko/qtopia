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

#ifndef QDS_ACTION_REQUEST_H
#define QDS_ACTION_REQUEST_H

// Qt includes
#include <QObject>

// Qtopia includes
#include <Qtopia>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSActionRequestPrivate;
class QDSServiceInfo;
class QDSData;

// ============================================================================
//
//  QDSActionRequest
//
// ============================================================================

class QTOPIA_EXPORT QDSActionRequest : public QObject
{
    Q_OBJECT

public:

    explicit QDSActionRequest( QObject* parent = 0 );
    QDSActionRequest( const QDSActionRequest& other );

    QDSActionRequest( const QDSServiceInfo& serviceInfo,
                      const QString& channel,
                      QObject* parent = 0 );

    QDSActionRequest( const QDSServiceInfo& serviceInfo,
                      const QDSData& requestData,
                      const QString& channel,
                      const QByteArray& auxiliary = QByteArray(),
                      QObject* parent = 0 );

    ~QDSActionRequest();

    const QDSActionRequest& operator=( const QDSActionRequest& other );

    const QDSServiceInfo& serviceInfo() const;
    bool isValid() const;
    bool isComplete() const;

    const QDSData& requestData() const;
    const QDSData& responseData() const;
    const QByteArray& auxiliaryData() const;
    QString errorMessage() const;

    bool respond();
    bool respond( const QDSData& responseData );
    bool respond( const QString& error );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QDSActionRequestPrivate* d;
};

Q_DECLARE_USER_METATYPE( QDSActionRequest );

#endif //QDS_ACTION_REQUEST_H
