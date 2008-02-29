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

#ifndef CONTENTLNK_P_H
#define CONTENTLNK_P_H

#include <qcontent.h>
#include <qcontentset.h>
#include <QMultiHash>
#include <QMap>
#include <QPair>
#include <QFlags>
#include <QThreadStorage>

class QFileInfo;

/*!
  \class ContentLinkPrivate
  \internal
  A normal document object managed by the QContent system (not
  an application, and not DRM controlled).
*/
class ContentLinkPrivate : public QSharedData
{
public:
    ContentLinkPrivate();
    virtual ~ContentLinkPrivate();
    ContentLinkPrivate( const ContentLinkPrivate &copy );
    virtual bool isValid(bool force=false) const;
    virtual const QString &name() const;
    virtual const QIcon &icon() const;
    virtual const QIcon &icon( QDrmRights::Permission permission ) const;
    virtual const QString &type() const;
    virtual const QString &comment() const;
    virtual const QString &file() const;
    virtual qint64 size() const;
    virtual const QString &rotation() const { return cRotation; }
    virtual QDrmRights::Permissions permissions( bool = false ) const{ return QDrmRights::Unrestricted; }
    virtual QDrmRights rights( QDrmRights::Permission permission ) const { return QDrmRights( permission, QDrmRights::Valid ); }
    virtual const QStringList &mimeTypes() const { return cMimeTypes; }
    virtual const QStringList &mimeTypeIcons() const;
    virtual const QList< QDrmRights::Permission > &mimeTypePermissions() const;
    virtual QDateTime createdDate() const;
    virtual QDateTime modifiedDate() const;
    virtual const QDateTime &lastUpdated() const { return cLastUpdated; }

    virtual const QString &media() const;
    virtual bool setMedia( const QString &media );

    virtual ContentLinkPrivate &operator=( const ContentLinkPrivate &rhs );
    virtual void loadIcon( bool force = false );
    bool fileKnown() const { return !cPath.isEmpty(); }
    virtual const QString &linkFile() const {return cLinkFile;}
    virtual void execute() const;
    virtual void execute(const QStringList& args) const;
    virtual const QString &iconName() const;
    virtual void setProperty(const QString& key, const QString& value, const QString& group=QString());
    virtual QString property(const QString& key, const QString& group=QString()) const;

    virtual void removeFiles();

    void syncFileName();

    virtual ContentLinkPrivate *createCopy() const{ return new ContentLinkPrivate( *this ); }

    virtual QIODevice *open(QIODevice::OpenMode) const;

    enum ContentState
    {
        New,
        Edited,
        Committed
    };

    virtual ContentState contentState() const;

    static bool prepareDirectories(const QString& lf);

    static QList< QDrmRights::Permission > extractPermissions( const QString &permissionList );
    static QString packPermissions( const QList< QDrmRights::Permission > &permissions );

protected:


    static QIcon unknownDocIcon();
    static QPixmap unknownDocPixmap( int size );

    virtual void load( const QHash<QString, QVariant> & );

    QString cName;      // user visible name
    QString cIconPath;      // path/reference to icon
    QString cType;  // mime type string, or qtopia type
    QString cComment;   // exif or other metadata, drm info
    QString cPath;      // path/reference to content
    enum Validity { UnknownValidity, Valid, Invalid };
    Validity valid;         // current validity
    QString cRotation;  // rotation of app at exec time
    QStringList cMimeTypes;
    QStringList cMimeTypeIcons;
    QList< QDrmRights::Permission > cMimeTypePermissions;
    QIcon cIcon;
    QString cLinkFile;
    QMap<QPair<QString, QString>, QString> cPropertyList;
    QStringList cCategories;
    qint64 cSize;
    QDateTime cLastUpdated;

    QString cMedia;
    QDrmRights::Permissions cPermissions;
    ContentState cContentState;

    QContentId cId;   // record number in backing store
    QContent::Role um;
    QContent::DrmState drm;

    bool cNameChanged;

    static QHash<QContentId, QString> errors;
    static QThreadStorage<ContentLinkSql *> dbs;

    enum ExtraData { Categories=0x01, Properties=0x02, NameTranslated=0x04, MimePermissions=0x08, MimeIcons=0x10 };
    int cExtraLoaded;

    bool batchLoading;

private:

    static ContentLinkSql *persistenceEngine();

    friend class QContent;
    friend class ContentLinkSql;
    friend QDebug &operator<<(QDebug &debug, const ContentLinkPrivate *contentLink);
};

/*!
  \class ContentLinkPrivateDRM
  \internal
  A DRM controlled content object managed by the QContent system
  (possibly an application, eg Java midlet).

  The content if backed by a file will be stored in an encrypted
  .dcf format in a directory & registered with the DRM agent.  A DRM
  rights object is required to access the file.  If the content arrived
  as a DRM message (either Forward Lock, or Combined Delivery) then the
  DRM agent will have created the rights object and it will be registered
  but if the rights object has not arrived yet (Separate Delivery or
  Superdistribution) a rights object might not be available.

  The real metadata such as size, mimetype and name are stored inside
  the DCF object and can only be accessed via the DRM agent.  The content-
  lnk object keeps a cache of these values.  The size of the encrypted
  object will be different from the real size.

  The decrypted content may be available as a stream.  It is not permitted
  to store the stream to a file.  The stream should be passed directly
  to the rendering program by the launcher.  The size attribute may be
  used to display a progress bar.

  The metadata also includes a URI to an icon.  If the URI is not to a
  local file, then either the remote file must be fetched, or a place-
  holder icon used.  In the case of an application the correct icon as
  specified by the URI should be used if possible.

  The comment method returns information about the current status of the
  rights object to allow display in a tool-tip format.
*/
class ContentLinkPrivateDRM : public ContentLinkPrivate
{
public:
    ContentLinkPrivateDRM();
    ContentLinkPrivateDRM( const ContentLinkPrivate &copy );
    ContentLinkPrivateDRM( const ContentLinkPrivateDRM &copy );
    virtual ~ContentLinkPrivateDRM();

    ContentLinkPrivateDRM &operator =( const ContentLinkPrivateDRM &rhs );

    virtual const QIcon &icon() const;
    virtual const QIcon &icon( QDrmRights::Permission permission ) const;
    virtual const QString &comment() const;
    virtual QDrmRights rights( QDrmRights::Permission permission ) const;
    virtual QDrmRights::Permissions permissions( bool force = false ) const;
    virtual qint64 size() const;

    virtual void removeFiles();

    virtual ContentLinkPrivate *createCopy() const{ return new ContentLinkPrivateDRM( *this ); }

protected:
    const QIcon &invalidIcon() const;

    virtual void loadIcon( bool force = false );
    void loadInvalidIcon( bool force = false );
    QPixmap pixmap( int size, bool valid ) const;

    static QIcon unknownDocIcon( bool valid );
    static QPixmap unknownDocPixmap( int size, bool valid );

#ifdef Q_WS_QWS
    virtual void execute(const QStringList& args) const;
#endif

private:
    QIcon cInvalidIcon;
};

#endif
