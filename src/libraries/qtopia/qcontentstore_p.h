/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QCONTENTSTORE_H
#define QCONTENTSTORE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QContentFilter>
#include <QThread>
#include <QCache>
#include <QReadWriteLock>

#include <qtopia/private/qcontentengine_p.h>

class QContentSetEngine;
class QContentFilterSetEngine;
class QMimeTypeData;

class QContentStore : public QObject
{
    Q_OBJECT
public:

    enum LookupFlag
    {
        Lookup    = 0x01,
        Commit    = 0x02,
        Construct = 0x04
    };

    typedef QFlags< LookupFlag > LookupFlags;

    QContentStore( QObject *parent = 0 );

    virtual ~QContentStore();

    virtual QContent contentFromId( QContentId contentId ) = 0;
    virtual QContent contentFromFileName( const QString &fileName, LookupFlags lookup ) = 0;
    virtual QContent contentFromEngineType( const QString &engineType ) = 0;

    virtual bool commitContent( QContent *content ) = 0;
    virtual bool removeContent( QContent *content ) = 0;
    virtual bool uninstallContent( QContentId contentId ) = 0;

    virtual void batchCommitContent( const QContentList &content ) = 0;
    virtual void batchUninstallContent( const QContentIdList &content ) = 0;

    virtual bool moveContentTo( QContent *content, const QString &newFileName ) = 0;
    virtual bool copyContentTo( QContent *content, const QString &newFileName ) = 0;

    virtual QIODevice *openContent( QContent *content, QIODevice::OpenMode mode ) = 0;

    virtual QStringList contentCategories( QContentId contentId ) = 0;

    virtual QContentEnginePropertyCache contentProperties( QContentId id ) = 0;

    virtual QStringList contentMimeTypes( QContentId contentId ) = 0;

    virtual QMimeTypeData mimeTypeFromId( const QString &mimeId ) = 0;

    virtual QContentSetEngine *contentSet( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode ) = 0;

    virtual QContentFilterSetEngine *contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType ) = 0;

    virtual QStringList filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType ) = 0;

    virtual int contentCount( const QContentFilter &filter ) = 0;

    virtual void addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission) = 0;
    virtual void removeAssociation(const QString& mimeType, const QString& application) = 0;
    virtual void setDefaultApplicationFor(const QString& mimeType, const QString& application) = 0;
    virtual QList<QMimeEngineData> associationsForApplication(const QString& application ) = 0;
    virtual QList<QMimeEngineData> associationsForMimeType(const QString& mimeType ) = 0;

    static QContentStore *instance();

    static bool initialized();

    QString errorString() const;

protected:
    void setErrorString( const QString &error );

    QContentEngine::Attributes dirtyAttributes( const QContentEngine &engine ) const;

    void setId( QContentId id, QContentEngine *engine ) const;

    void setDrmState( QContent::DrmState state, QContentEngine *engine ) const;

    void setLastUpdated( const QDateTime &date, QContentEngine *engine ) const;

    QContentEngine *contentEngine( QContent *content ) const;

private:
    QString m_errorString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QContentStore::LookupFlags);
Q_DECLARE_USER_METATYPE_ENUM(QContentStore::LookupFlags);

class QContentCache
{
public:
    QContentCache();

    QContent lookup( QContentId contentId );
    void cache( const QContent &content );
    void remove( QContentId contentId );

    static QContentCache *instance();

private:
    QCache< QContentId, QContent > m_cache;
    QReadWriteLock m_lock;
};

#endif
