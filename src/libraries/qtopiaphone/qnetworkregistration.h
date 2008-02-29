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

#ifndef QNETWORKREGISTRATION_H
#define QNETWORKREGISTRATION_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <QList>

class QDataStream;

class QTOPIAPHONE_EXPORT QNetworkRegistration : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ initialized)
    Q_PROPERTY(QTelephony::RegistrationState registrationState READ registrationState)
    Q_PROPERTY(QString currentOperatorId READ currentOperatorId)
    Q_PROPERTY(QString currentOperatorName READ currentOperatorName)
    Q_PROPERTY(QTelephony::OperatorMode currentOperatorMode READ currentOperatorMode)
    Q_PROPERTY(QString currentOperatorTechnology READ currentOperatorTechnology)
    Q_PROPERTY(int locationAreaCode READ locationAreaCode)
    Q_PROPERTY(int cellId READ cellId)
public:
    explicit QNetworkRegistration( const QString& service = QString(),
                                   QObject *parent = 0,
                                   QCommInterface::Mode mode = Client );
    ~QNetworkRegistration();

    struct AvailableOperator
    {
        QTelephony::OperatorAvailability availability;
        QString name;
        QString shortName;
        QString id;
        QString technology;
        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

    bool initialized() const;
    QTelephony::RegistrationState registrationState() const;
    QString currentOperatorId() const;
    QString currentOperatorName() const;
    QTelephony::OperatorMode currentOperatorMode() const;
    QString currentOperatorTechnology() const;
    int locationAreaCode() const;
    int cellId() const;

public slots:
    virtual void setCurrentOperator
        ( QTelephony::OperatorMode mode,
          const QString& id = QString(),
          const QString& technology = QString() );
    virtual void requestAvailableOperators();

signals:
    void initializedChanged();
    void registrationStateChanged();
    void locationChanged();
    void currentOperatorChanged();
    void setCurrentOperatorResult( QTelephony::Result result );
    void availableOperators
            ( const QList<QNetworkRegistration::AvailableOperator>& opers );
};

class QTOPIAPHONE_EXPORT QNetworkRegistrationServer : public QNetworkRegistration
{
    Q_OBJECT
public:
    explicit QNetworkRegistrationServer( const QString& service, QObject *parent = 0 );
    ~QNetworkRegistrationServer();

protected:
    void updateInitialized( bool value );
    void updateRegistrationState( QTelephony::RegistrationState state );
    void updateRegistrationState
        ( QTelephony::RegistrationState state,
          int locationAreaCode, int cellId );
    void updateCurrentOperator( QTelephony::OperatorMode mode,
                                const QString& id,
                                const QString& name,
                                const QString& technology );
};

Q_DECLARE_USER_METATYPE(QNetworkRegistration::AvailableOperator)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QNetworkRegistration::AvailableOperator>)

#endif /* QNETWORKREGISTRATION_H */
