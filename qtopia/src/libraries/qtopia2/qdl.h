/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef HAVE_QDL_H
#define HAVE_QDL_H

#include <qlist.h>
#include <qstring.h>
#include <qcstring.h>
#include <qmap.h>
#include <qobject.h>

#include <qtopia/qpeglobal.h>

class PimRecord;
class QWidget;
#ifdef QTOPIA_PHONE
class ContextMenu;
#endif

class QTOPIA_EXPORT QDLLink : public QObject
{
    Q_OBJECT
public:
    QDLLink();
    QDLLink( const QCString &appRef, const QByteArray &dataRef, const QString &desc, const QCString &icon );
    QDLLink( const QDLLink &other );

    bool isNull() const;

    void setAppRef( const QCString &ref );
    QCString appRef() const;
    void setDataRef( const QByteArray &ref );
    QByteArray dataRef() const;
    void setDescription( const QString &desc );
    QString description() const;
    void setIcon( const QCString &icon );
    QCString icon() const;

    QDLLink &operator=( const QDLLink &other );
private:
    QCString mAppRef, mIcon;
    QByteArray mDataRef;
    QString mDescription;
};
QDataStream &operator>>( QDataStream &stream, QDLLink &link );
QDataStream &operator<<( QDataStream &stream, const QDLLink &link );

class QDLHeartBeatPrivate;
class QTOPIA_EXPORT QDLHeartBeat : public QObject
{
    Q_OBJECT
public:
    QDLHeartBeat( const QString &clientID );
    ~QDLHeartBeat();

private slots:
    void beat();

private:
    QDLHeartBeatPrivate *d;
};

class QDLClientPrivate;
class QTOPIA_EXPORT QDLClient : public QObject
{
    Q_OBJECT
public:
    QDLClient( QObject *parent, const char *name );

    bool isRequestActive() const;

    QDLLink link( uint lid ) const;
    QMap<uint, QDLLink> links() const;

    uint count() const;

    virtual void addLink( const QDLLink &newLink );
    virtual void setLink( uint lid, const QDLLink &newLink );
    virtual void removeLink( uint lid );

    QString hint() const;
    void setHint( const QString &hint );
public slots:
    virtual void clear();
    virtual void verifyLinks();

    void requestLink( QWidget *parent );
    void requestLink( const QString &channel );
    
protected slots:

    void receiveLinks( const QCString &sig, const QByteArray &data );

    void requestTimeout();

protected:
    uint nextLID();

private:
    QDLClientPrivate *d;
};

class QDLWidgetClientPrivate;
class QTOPIA_EXPORT QDLWidgetClient : public QDLClient
{
    Q_OBJECT
public:
    QDLWidgetClient( QWidget *w, const char *name );

#ifdef QTOPIA_PHONE
    ContextMenu *setupStandardContextMenu( ContextMenu *context = 0 );
#endif

    virtual void addLink( const QDLLink &newLink );
    virtual void setLink( uint lid, const QDLLink &newLink );
    virtual void removeLink( uint lid );

    QString hint() const;
public slots:

    void requestLink();

    virtual void verifyLinks();

protected slots:
    virtual void setText( const QString &txt );

protected:
    void setWidget( QWidget *w );
    QWidget *widget() const;

    void setWidgetType( const QString &t );
    QString widgetType() const;

    virtual QString text() const;

    virtual QString determineHint() const;

    virtual int textPos() const;
private:
    QDLWidgetClientPrivate *d;
};

// General public QDL usage class.
QDataStream &operator>>( QDataStream &stream, QList<QDLClient> &clientList );
QDataStream &operator<<( QDataStream &stream, const QList<QDLClient> &clientList );
class QTOPIA_EXPORT QDL : public Qt
{
public:
    static QList<QDLClient>  clients( QObject *parent );

    static void sendHeartBeat( const QString &clientID );

    static void loadLinks( const QString &str, QDLClient *client );
    static void loadLinks( const QString &str, QList<QDLClient> clientList );

    static void saveLinks( QString &str, QDLClient *client );
    static void saveLinks( QString &str, QList<QDLClient> clientList );

    static void activateLink( const QString &ahref, const QList<QDLClient> &clientList );

    static QString encodeAhref( const QString &ahref );
    static QString decodeAhref( const QString &ahref );

    static QString lidsToAnchors( const QString &lidText, const QDLClient *client, bool withIcons = TRUE );

    static QString removeLids( const QString &lidText );

    static uint lidToUInt( const QString &lid );
    static QString lidFromUInt( uint lid );

    static const QString DATA_KEY;

    static const QCString CLIENT_CHANNEL;

    static const ushort 
    u1, u2, u3, u4, u5,
    u6, u7, u8, u9, u0 
    ;
};


#endif // HAVE_QDL_H
