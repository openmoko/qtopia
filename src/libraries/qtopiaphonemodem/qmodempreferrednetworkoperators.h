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

#ifndef QMODEMPREFERREDNETWORKOPERATORS_H
#define QMODEMPREFERREDNETWORKOPERATORS_H

#include <qpreferrednetworkoperators.h>

class QModemService;
class QAtResult;
class QModemPreferredNetworkOperatorsPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemPreferredNetworkOperators
        : public QPreferredNetworkOperators
{
    Q_OBJECT
public:
    explicit QModemPreferredNetworkOperators( QModemService *service );
    ~QModemPreferredNetworkOperators();

public slots:
    void requestOperatorNames();
    void requestPreferredOperators
        ( QPreferredNetworkOperators::List list );
    void writePreferredOperator
        ( QPreferredNetworkOperators::List list,
          const QPreferredNetworkOperators::Info & oper );

private slots:
    void copn( bool ok, const QAtResult& result );
    void cplsQuery( bool ok, const QAtResult& result );
    void cpolQuery( bool ok, const QAtResult& result );
    void cpolSet( bool ok, const QAtResult& result );
    void cpolSet2( bool ok, const QAtResult& result );

protected:
    bool deleteBeforeUpdate() const;
    void setDeleteBeforeUpdate( bool value );
    bool quoteOperatorNumber() const;
    void setQuoteOperatorNumber( bool value );

private:
    QModemPreferredNetworkOperatorsPrivate *d;

    uint listNumber( QPreferredNetworkOperators::List list );
    void writeNextPreferredOperator();
};

#endif /* QMODEMPREFERREDNETWORKOPERATORS_H */
