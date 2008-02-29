/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QDRMCONTENTPLUGIN_H
#define QDRMCONTENTPLUGIN_H

#include <qdrmcontent.h>
#include <QLocalUniqueId>
#include <QAbstractFileEngineHandler>

class QDrmContentLicensePrivate;

class QTOPIA_EXPORT QDrmContentLicense : public QObject
{
    Q_OBJECT
public:
    QDrmContentLicense();
    virtual ~QDrmContentLicense();

    virtual QContent content() const = 0;
    virtual QDrmRights::Permission permission() const = 0;

    virtual QDrmContent::RenderState renderState() const = 0;

public slots:
    virtual void renderStateChanged( const QContent &content, QDrmContent::RenderState state ) = 0;

signals:
    void rightsExpired( const QContent &content, QDrmRights::Permission permission );
};

class QDrmContentPluginPrivate;

class QTOPIA_EXPORT QDrmContentPlugin : public QObject, public QAbstractFileEngineHandler
{
    Q_OBJECT
public:
    explicit QDrmContentPlugin( QObject *parent );

    virtual ~QDrmContentPlugin();

    virtual QStringList keys() const = 0;

    virtual QStringList types() const = 0;

    virtual QList< QPair< QString, QString > > httpHeaders() const;

    virtual bool isProtected( const QString &fileName ) const = 0;

    virtual QDrmRights::Permissions permissions( const QString &fileName ) = 0;

    virtual QDrmRights getRights( const QString &fileName, QDrmRights::Permission permission ) = 0;

    virtual QDrmContentLicense *requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options );

    virtual QIODevice *createDecoder( const QString &fileName, QDrmRights::Permission permission );

    virtual bool canActivate( const QString &fileName ) = 0;

    virtual bool activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus ) = 0;

    virtual void activate( const QContent &content, QWidget *focus ) = 0;

    virtual void reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus ) = 0;

    virtual bool deleteFile( const QString &fileName ) = 0;

    virtual qint64 unencryptedSize( const QString &fileName ) = 0;

    virtual bool installContent( const QString &fileName, QContent *content ) = 0;

    virtual QPixmap thumbnail( const QString &fileName, const QSize &size, Qt::AspectRatioMode mode = Qt::KeepAspectRatio );

    virtual QAbstractFileEngine *create( const QString &file ) const;

    QDrmContentLicense *license( const QString &fileName ) const;

    static void initialize();

protected slots:
    void registerLicense( QDrmContentLicense *license );

private slots:
    void unregisterLicense( QObject *license );

private:
    QDrmContentPluginPrivate *d;

};

Q_DECLARE_INTERFACE( QDrmContentPlugin, "com.trolltech.Qtopia.QDrmContentPlugin/1.0" );

class QTOPIA_EXPORT QDrmAgentPlugin
{
public:
    virtual ~QDrmAgentPlugin();

    virtual QDrmContentPlugin *createDrmContentPlugin() = 0;
    virtual void createService( QObject *parent ) = 0;
    virtual QList< QWidget * > createManagerWidgets( QWidget *parent = 0 ) = 0;
};

Q_DECLARE_INTERFACE( QDrmAgentPlugin, "com.trolltech.Qtopia.QDrmAgentPlugin/1.0" );

#endif
