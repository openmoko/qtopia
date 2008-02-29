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

#ifndef COMPOSERINTERFACE_H
#define COMPOSERINTERFACE_H

#include <QObject>
#include <QList>
#include <QString>
#include <QIconSet>

#include "email.h"

class QContent;

class ComposerInterface : public QObject
{
    Q_OBJECT

public:

    ComposerInterface( QObject *parent = 0, const char *name = 0 );
    virtual ~ComposerInterface();

    // type() + some unique string
    QString id() const;
    int type() const;
    QString fullName() const;
    QString nickName() const;
    QIcon displayIcon() const;

    virtual bool hasContent() const = 0;
    virtual void getContent( MailMessage &email ) const = 0;
    virtual void setMailMessage( Email &mail ) = 0;

    virtual QWidget *widget() const = 0;

public slots:
    virtual void clear() = 0;
    virtual void attach( const QContent &lnk );
    virtual void attach( const QString &fileName );

signals:
    void contentChanged();

};

class TextComposerInterface : public ComposerInterface
{
    Q_OBJECT

public:
    TextComposerInterface( QObject *parent = 0, const char *name = 0 );
    virtual ~TextComposerInterface();
    virtual void setText( const QString &txt ) = 0;
};

//not a real factory, but still makes it possible to add composers without
//modifying qtmail
class ComposerFactory
{
    friend class ComposerInterface;
public:
    static QString defaultInterface( const MailMessage &mail );
    static QString defaultInterface( int type = -1 );
    static QStringList interfaces( const MailMessage &mail );
    static QStringList interfaces( int type = -1 );
    static QString fullName( const QString &cid );
    static QString nickName( const QString &cid );
    static QIcon displayIcon( const QString &cid );
    static ComposerInterface *create( const QString &id, QWidget *parent = 0,
                                                        const char *name = 0 );

private:
    static int type( const ComposerInterface *iface );
    static QString id( const ComposerInterface *iface );
    static QString fullName( const ComposerInterface *iface );
    static QString nickName( const ComposerInterface *iface );
    static QIcon displayIcon( const ComposerInterface *iface );
};

#endif
