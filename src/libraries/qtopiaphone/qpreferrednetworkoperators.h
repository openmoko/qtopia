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

#ifndef QPREFERREDNETWORKOPERATORS_H
#define QPREFERREDNETWORKOPERATORS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QPreferredNetworkOperators : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(List)
public:
    explicit QPreferredNetworkOperators( const QString& service = QString::null,
                                         QObject *parent = 0,
                                         QCommInterface::Mode mode = Client );
    ~QPreferredNetworkOperators();

    enum List
    {
        Current,
        UserControlled,
        OperatorControlled,
        HPLMN
    };

    struct Info
    {
        uint index;
        uint format;
        uint id;
        QString name;
        QStringList technologies;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

    struct NameInfo
    {
        QString name;
        uint id;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

    static QList<QPreferredNetworkOperators::Info> resolveNames
            ( const QList<QPreferredNetworkOperators::Info>& opers,
              const QList<QPreferredNetworkOperators::NameInfo>& names );

public slots:
    virtual void requestOperatorNames();
    virtual void requestPreferredOperators
        ( QPreferredNetworkOperators::List list );
    virtual void writePreferredOperator
        ( QPreferredNetworkOperators::List list,
          const QPreferredNetworkOperators::Info & oper );

signals:
    void operatorNames
        ( const QList<QPreferredNetworkOperators::NameInfo>& names );
    void preferredOperators
        ( QPreferredNetworkOperators::List list,
          const QList<QPreferredNetworkOperators::Info>& opers );
    void writePreferredOperatorResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QPreferredNetworkOperators::List)
Q_DECLARE_USER_METATYPE(QPreferredNetworkOperators::Info)
Q_DECLARE_USER_METATYPE(QPreferredNetworkOperators::NameInfo)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QPreferredNetworkOperators::NameInfo>)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QPreferredNetworkOperators::Info>)

#endif /* QPREFERREDNETWORKOPERATORS_H */
