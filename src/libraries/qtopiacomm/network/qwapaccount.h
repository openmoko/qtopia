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

#ifndef QWAP_ACCOUNT
#define QWAP_ACCOUNT

#include <qtopiaglobal.h>

#include <QUrl>
#include <QString>

class QWapAccountPrivate;

class QTOPIACOMM_EXPORT QWapAccount
{
public:
    enum MMSVisibility {
        Default,
        SenderHidden,
        SenderVisible
    };
    
    QWapAccount();
    QWapAccount( const QString& wapConfig );
    QWapAccount( const QWapAccount& copy );
    
    virtual ~QWapAccount();

    QWapAccount& operator=(const QWapAccount& copy);
    bool operator==(const QWapAccount& other );

    QString configuration() const;

    QString name() const;
    void setName( const QString& name );

    QString dataInterface() const; 
    void setDataInterface( const QString& ifaceHandle );

    QUrl gateway() const;
    void setGateway( const QUrl& );

    QUrl mmsServer() const;
    void setMmsServer( const QUrl& url );    

    int mmsExpiry() const;
    void setMmsExpiry( int mmsExpiry );

    QWapAccount::MMSVisibility mmsSenderVisibility() const;
    void setMmsSenderVisibility( QWapAccount::MMSVisibility vis );
    
    bool mmsDeliveryReport() const;
    void setMmsDeliveryReport( bool allowReport );

private:
    QWapAccountPrivate* d;
    
};

#endif //QWAP_ACCOUNT
