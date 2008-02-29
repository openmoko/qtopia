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

#ifndef QMAILCOMPOSER_H
#define QMAILCOMPOSER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QIconSet>

#include <qtopiaglobal.h>
#include <qmailmessage.h>

class QContent;
class QMenu;
class QWidget;

class QTOPIAMAIL_EXPORT QMailComposerInterface : public QObject
{
    Q_OBJECT

public:
    QMailComposerInterface( QWidget *parent = 0 );
    virtual ~QMailComposerInterface();

    QString key() const;
    QMailMessage::MessageType messageType() const;

    QString name() const;
    QString displayName() const;
    QIcon displayIcon() const;

    virtual bool isEmpty() const = 0;
    virtual QMailMessage message() const = 0;

    virtual QWidget *widget() const = 0;

    virtual void addActions(QMenu* menu) const;

public slots:
    virtual void setMessage( const QMailMessage& mail ) = 0;
    virtual void clear() = 0;
    virtual void setText( const QString &text, const QString &type );
    virtual void attach( const QContent &lnk, QMailMessage::AttachmentsAction action = QMailMessage::LinkToAttachments );
    virtual void setSignature( const QString &sig );

signals:
    void contentChanged();
    void finished();
};

class QTOPIAMAIL_EXPORT QMailComposerFactory
{
public:
    // Yield the key for each interface supporting the supplied type
    static QStringList keys( QMailMessage::MessageType type = QMailMessage::AnyType );

    // Yield the default key for the supplied type
    static QString defaultKey( QMailMessage::MessageType type = QMailMessage::AnyType );

    // Properties available for each interface
    static QMailMessage::MessageType messageType( const QString& key );
    static QString name( const QString& key );
    static QString displayName( const QString& key );
    static QIcon displayIcon( const QString& key );

    // Use the interface identified by the supplied key to create a composer
    static QMailComposerInterface *create( const QString& key, QWidget *parent = 0 );
};

#endif // QMAILCOMPOSER_H
