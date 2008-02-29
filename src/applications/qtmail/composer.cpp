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

#include "emailcomposer.h"
#include "genericcomposer.h"
#ifndef QTOPIA_NO_MMS
#include "mmscomposer.h"
#endif
#include "emscomposer.h"
#include "composer.h"

#include <QApplication>

#include <qcontent.h>

//#define ENABLEEMSCOMPOSER

TextComposerInterface::TextComposerInterface( QObject *parent, const char *name )
    : ComposerInterface( parent, name )
{
}

TextComposerInterface::~TextComposerInterface()
{
}

ComposerInterface::ComposerInterface( QObject *parent, const char *name )
    : QObject( parent )
{
    setObjectName( name );
}

ComposerInterface::~ComposerInterface()
{
}

QString ComposerInterface::id() const
{
    return ComposerFactory::id( this );
}

int ComposerInterface::type() const
{
    return ComposerFactory::type( this );
}

QString ComposerInterface::fullName() const
{
    return ComposerFactory::fullName( this );
}

QString ComposerInterface::nickName() const
{
    return ComposerFactory::nickName( this );
}

QIcon ComposerInterface::displayIcon() const
{
    return ComposerFactory::displayIcon( this );
}

void ComposerInterface::attach( const QContent & )
{
    // default implementation does nothing
}

void ComposerInterface::attach( const QString & )
{
    // default implementation does nothing
}

QString ComposerFactory::id( const ComposerInterface *iface )
{
    QString val;
    if( iface )
        val = iface->metaObject()->className();
    return val;
}

QString ComposerFactory::fullName( const ComposerInterface *iface )
{
    return ComposerFactory::fullName( ComposerFactory::id( iface ) );
}

QString ComposerFactory::nickName( const ComposerInterface *iface )
{
    return ComposerFactory::nickName( ComposerFactory::id( iface ) );
}

QIcon ComposerFactory::displayIcon( const ComposerInterface *iface )
{
    return ComposerFactory::displayIcon( ComposerFactory::id( iface ) );
}


QString ComposerFactory::fullName( const QString &cid )
{
    if( cid == "EmailComposerInterface" )
        return qApp->translate("ComposerFactory","Email");
#ifndef QTOPIA_NO_SMS
    else if( cid == "GenericComposerInterface" )
        return qApp->translate("ComposerFactory","Text Message");
#endif
#ifndef QTOPIA_NO_MMS
    else if( cid == "MMSComposerInterface" )
        return qApp->translate("ComposerFactory","Multimedia Message");
#endif
#ifdef ENABLEEMSCOMPOSER
    else if( cid == "EMSComposerInterface" )
        return qApp->translate("ComposerFactory","EMS");
#endif
    return QString();
}

QString ComposerFactory::nickName( const QString &cid )
{
    if( cid == "EmailComposerInterface" )
        return qApp->translate("ComposerFactory","Email");
#ifndef QTOPIA_NO_SMS
    else if( cid == "GenericComposerInterface" )
        return qApp->translate("ComposerFactory","Message");
#endif
#ifndef QTOPIA_NO_MMS
    else if( cid == "MMSComposerInterface" )
        return qApp->translate("ComposerFactory","MMS");
#endif
#ifdef ENABLEEMSCOMPOSER
    else if( cid == "EMSComposerInterface" )
        return qApp->translate("ComposerFactory","EMS");
#endif
    return QString();
}

QIcon ComposerFactory::displayIcon( const QString &cid )
{
    if( cid == "EmailComposerInterface" )
        return QIcon(":icon/email");
#ifndef QTOPIA_NO_SMS
    else if( cid == "GenericComposerInterface" )
        return QIcon(":icon/txt");
#endif
#ifndef QTOPIA_NO_MMS
    else if( cid == "MMSComposerInterface" )
        return QIcon(":icon/multimedia");
#endif
#ifdef ENABLEEMSCOMPOSER
    else if( cid == "EMSComposerInterface" )
        return QIcon(":icon/multimedia");
#endif
    return QIcon();
}

int ComposerFactory::type( const ComposerInterface *iface )
{
    int t = -1;
    if( iface )
    {
        QString id = ComposerFactory::id( iface );
        if( id == "EmailComposerInterface" )
            t = MailMessage::Email;
#ifndef QTOPIA_NO_SMS
        else if( id == "GenericComposerInterface" )
            t = MailMessage::SMS;
#endif
#ifndef QTOPIA_NO_MMS
        else if( id == "MMSComposerInterface" )
            t = MailMessage::MMS;
#endif
#ifdef ENABLEEMSCOMPOSER
        else if( id == "EMSComposerInterface" )
            t = MailMessage::EMS;
#endif
    }
    return t;
}

QStringList ComposerFactory::interfaces( int t )
{
    QStringList in;

    // the order here is the order they appear in the UI
#ifndef QTOPIA_NO_SMS
    if( t < 0 || (t == MailMessage::SMS) || (t == (MailMessage::SMS | MailMessage::Email)) )
        // composer can handle only SMS or both SMS and Email
        in.append( "GenericComposerInterface" );
#endif
#ifndef QTOPIA_NO_MMS
    if( t < 0 || t == MailMessage::MMS )
        in.append( "MMSComposerInterface" );
#endif
#ifdef ENABLEEMSCOMPOSER
    if( t < 0 || t == MailMessage::EMS )
        in.append( "EMSComposerInterface" );
#endif
    if( t < 0 || t == MailMessage::Email )
        in.append( "EmailComposerInterface" );
    return in;
}

QStringList ComposerFactory::interfaces( const MailMessage &mail )
{
    return ComposerFactory::interfaces( mail.type() );
}

QString ComposerFactory::defaultInterface( int t )
{
    QString ifaceID;
    QList<QString> ifaceIDs = ComposerFactory::interfaces( t );
    if( ifaceIDs.count() )
        ifaceID = ifaceIDs.first();
    return ifaceID;
}

QString ComposerFactory::defaultInterface( const MailMessage &mail )
{
    return ComposerFactory::defaultInterface( mail.type() );
}

ComposerInterface *ComposerFactory::create( const QString &id, QWidget *parent, const char *name )
{
#ifndef QTOPIA_NO_SMS
    if( id == "GenericComposerInterface" )
        return new GenericComposerInterface( parent, name );
    // your composer interface here
    else
#endif
    if( id == "EmailComposerInterface" )
        return new EmailComposerInterface( parent, name );
#ifndef QTOPIA_NO_MMS
    else if( id == "MMSComposerInterface" )
        return new MMSComposerInterface( parent, name );
#endif
#ifdef ENABLEEMSCOMPOSER
    else if( id == "EMSComposerInterface" )
        return new EMSComposerInterface( parent, name );
#endif
    else
    {
        qWarning(QString( "ComposerFactory::create - can't create unknown composer interface " + id).toAscii() );
        return 0;
    }
}
