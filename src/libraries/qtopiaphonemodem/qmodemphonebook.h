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
#ifndef QMODEMPHONEBOOK_H
#define QMODEMPHONEBOOK_H

#include <qphonebook.h>

class QModemService;
class QModemPhoneBookPrivate;
class QModemPhoneBookOperation;
class QModemPhoneBookCache;
class QAtResult;
class QTextCodec;

class QTOPIAPHONEMODEM_EXPORT QModemPhoneBook : public QPhoneBook
{
    Q_OBJECT
public:
    explicit QModemPhoneBook( QModemService *service );
    ~QModemPhoneBook();

    QTextCodec *stringCodec() const;

public slots:
    void getEntries( const QString& store );
    void add( const QPhoneBookEntry& entry, const QString& store, bool flush );
    void remove( uint index, const QString& store, bool flush );
    void update( const QPhoneBookEntry& entry, const QString& store, bool flush );
    void flush( const QString& store );
    void setPassword( const QString& store, const QString& password );
    void clearPassword( const QString& store );
    void requestLimits( const QString& store );
    void requestFixedDialingState();
    void setFixedDialingState( bool enabled, const QString& pin2 );
    void preload( const QString& store );
    void flushCaches();
    void phoneBooksReady();
    void updateCodec( const QString& gsmCharset );

Q_SIGNALS:
    void queryFailed(const QString&);

protected:
    virtual bool hasModemPhoneBookCache() const;
    virtual bool hasEmptyPhoneBookIndex() const;

private:
    void forceStorageUpdate();
    void updateStorageName( const QString& storage, QObject *target=0,
                            const char *slot=0 );
    void sendQuery( QModemPhoneBookCache *cache );
    QModemPhoneBookCache *findCache( const QString& store, bool fast=true,
                                     const QString& initialPassword = QString() );
    void flushOperations( QModemPhoneBookCache *cache );
    virtual void flushAdd( const QPhoneBookEntry& entry, QModemPhoneBookCache *cache );
    void flushRemove( uint index, QModemPhoneBookCache *cache );
    void flushUpdate( const QPhoneBookEntry& entry, QModemPhoneBookCache *cache );
    void removeAllCaches();

private slots:
    void cscsDone( bool ok, const QAtResult& result );
    void readDone( bool ok, const QAtResult& result );
    void readFinished( QModemPhoneBookCache *cache );
    void queryDone( bool ok, const QAtResult& result );
    void slowTimeout();
    void fdQueryDone( bool ok, const QAtResult& result );
    void fdModifyDone( bool ok, const QAtResult& result );
    void selectDone( bool ok, const QAtResult& result );
    void requestCharset();
    void cpbsDone( bool ok, const QAtResult& result );
    void requestStorages();

private:
    QModemPhoneBookPrivate *d;
};

#endif // QMODEMPHONEBOOK_H
