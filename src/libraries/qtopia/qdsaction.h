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

#ifndef QDS_ACTION_H
#define QDS_ACTION_H

// Local includes
#include "qdsdata.h"

// Qt includes
#include <QObject>
#include <QString>
#include <QStringList>
#include <QIcon>

// Qtopia includes
#include <Qtopia>
#include <QMimeType>
#include <QLocalUniqueId>
#include <qtopiaglobal.h>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSActionPrivate;
class QDSServiceInfo;
class QDataStream;

// ============================================================================
//
//  QDSAction
//
// ============================================================================

class QTOPIA_EXPORT QDSAction : public QObject
{
    Q_OBJECT

public:
    explicit QDSAction( QObject* parent = 0 );
    QDSAction( const QDSAction& other );
    QDSAction( const QString& name,
               const QString& service,
               QObject* parent = 0 );
    explicit QDSAction( const QDSServiceInfo& serviceInfo,
                        QObject* parent = 0 );

    ~QDSAction();

    const QDSAction& operator=( const QDSAction& other );

    // Enumerations
    enum ResponseCode { Invalid, Complete, CompleteData, Error };

    // Access methods
    bool isValid() const;
    bool isAvailable() const;
    bool isActive() const;

    QLocalUniqueId id() const;
    const QDSServiceInfo& serviceInfo() const;
    QDSData responseData() const;
    QString errorMessage() const;

    // Action invocation
    bool invoke();
    bool invoke( const QDSData &requestData,
                 const QByteArray& auxiliary = QByteArray() );
    int exec();
    int exec( const QDSData &requestData,
              const QByteArray& auxiliary = QByteArray() );

signals:
    void response( const QLocalUniqueId& actionId );
    void response( const QLocalUniqueId& actionId, const QDSData& responseData );
    void error( const QLocalUniqueId& actionId, const QString& message );

private:
    QDSActionPrivate* d;
};

#endif //QDS_ACTION_H
