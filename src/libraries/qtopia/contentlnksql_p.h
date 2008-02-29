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

#ifndef CONTENTLNKSQL_P_H
#define CONTENTLNKSQL_P_H

#include <qcontent.h>
#include <qcontentset.h>
#include <qcontentplugin.h>
#include <qcontentfilter.h>
#include "contentlnk_p.h"

#include <QSqlQuery>
#include <QHash>
#include <QMutex>

/*!
  \internal
  When finding the size of the result set some databases can quickly return
  a size without iterating over the query.  Instantiating the appropriate
  functor at database initialization elides the need for a conditional
  every time the size is checked.
*/
class ResultSizeCounter
{
public:
    ResultSizeCounter() {};
    virtual ~ResultSizeCounter() {};
    virtual int operator() ( QSqlQuery& ) = 0;
    virtual bool supportsNumRowsAffected() const = 0;
};

class NativeSizeCounter : public ResultSizeCounter
{
public:
    NativeSizeCounter() : ResultSizeCounter() {}
    virtual ~NativeSizeCounter() {};
    int operator() ( QSqlQuery &qry )
    {
        return qry.size();
    }
    virtual bool supportsNumRowsAffected() const { return true; }
};

class IteratingSizeCounter : public ResultSizeCounter
{
public:
    IteratingSizeCounter() : ResultSizeCounter() {}
    virtual ~IteratingSizeCounter() {};
    int operator() ( QSqlQuery &qry )
    {
        int pos=qry.at();
        if ( ! qry.isActive() ) return -1;
        int cnt = 0;
        qry.seek(-1);
        while ( qry.next() ) cnt++;
        qry.seek(pos);
        return cnt;
    }
    virtual bool supportsNumRowsAffected() const { return false; }
};

/*!
  \brief
  Concrete Content link persistence by SQL database, tuned for SQLite
*/
class ContentLinkSql : public QSharedData
{
public:
    ContentLinkSql();
    ~ContentLinkSql();
    QContentId postLink( ContentLinkPrivate *, QContent::ChangeType& );
    bool removeLink( QContentId );
    QStringList categoriesById( QContentId );
    int recordCount( const QContentFilter &filter );
    bool removeCategoryMap(QContentId cid);
    bool appendNewCategoryMap( const QString &, const QString &, const QContentId, const QString &trans = QString::null, bool useTransaction=true );
    QContentIdList matches( const QContentIdList &, const QContentFilter &, const QStringList & );
    bool writeProperty(QContentId cId, const QString& key, const QString& value, const QString& group=QString() );
    QMap<QPair<QString,QString>,QString> readProperties( const QContentId cId );
    QContentId updateLink( ContentLinkPrivate *cl );
    QContentId updateLinkUnchecked(ContentLinkPrivate *cl, const QContentIdList &cidList);

    static QString getCodeFromDocState( ContentLinkPrivate *cl );

    bool error() const { return isError; }
    QString errorText() const { return errText; } //for debug purposes only
    void setLinkError( QContentId id, const QString &e ) { ContentLinkPrivate::errors[ id ] = e; }

    //for debug purposes only
    QString linkError( QContentId id ) const { return ContentLinkPrivate::errors[ id ]; }

    int countRecords(QSqlQuery &qry) { if (sizeCounter) return (*sizeCounter)( qry ); else return -1; }
    bool hasRecords(QSqlQuery &qry);

    static const QHash<QString,QString> &getLocNames();


    QHash<QString, QVariant> linkByPath( const QString &path, bool isLink = false );
    QHash<QString, QVariant> linkById( QContentId cId );
    QContentId idByExecutable( const QString &name );
    QList< QHash<QString, QVariant> > linksById( const QContentIdList &cIds );

    QStringList mimeFilterMatches( const QContentFilter &filter, const QString &subType = QString() );
    QStringList syntheticFilterGroups( const QContentFilter &filter );
    QStringList syntheticFilterKeys( const QContentFilter &filter, const QString &group );
    QStringList syntheticFilterMatches( const QContentFilter &filter, const QString &group, const QString &key );
    QStringList categoryFilterMatches( const QContentFilter &filter, const QString &key );

    struct MimeData
    {
        QContentId cid;
        QString mimeTypes;
        QString icons;
        QString permissions;
    };

    QList< MimeData > getMimeData() ;

protected:
    bool isError;
    QString errText;
    static QHash<QString,QString> locNames;
private:

    typedef QPair< QString, QVariant > Parameter;

    void checkFeatures();
    bool addDRMFlagsColumn( QSqlQuery &qry );

    QHash<QString, QVariant> linkFromRecord( const QSqlRecord &record, QtopiaDatabaseId databaseId ) const;

    QString buildFrom( const QContentFilter &, const QStringList &joins );
    QString buildWhereClause( const QContentFilter &filter, QList< Parameter > *parameters, int *insertAt, QStringList *joins );
    QString buildQtopiaTypes( const QStringList &types, const QString &conjunct, QList< Parameter > *parameters );
    QString buildMimeTypes( const QStringList &mimes, const QString &conjunct, QList< Parameter > *parameters );
    QString buildPaths( const QStringList &locations, const QStringList &directories, const QString &conjunct, QList< Parameter > *parameters );
    QString buildCategories( const QStringList &categories, const QString &conjunct, QList< Parameter > *parameters, QStringList *joins );
    QString buildDrm( const QStringList &drm, const QString &conjunct, QList< Parameter > *parameters );
    QString buildSynthetic( const QStringList &synthetic, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins );
    QString buildOrderBy( const QStringList &, QList< Parameter > *, int *, QStringList * );
    int mimeCode(const QtopiaDatabaseId &dbid, const QString &mimeType);
    int drmCode( QContent::DrmState state );
    int locationCode( const QString &location );
    int queryLocationCode( const QString &location );
    bool queryFailed(const QSqlQuery &);
    QString buildQuery( const QString &queryTemplate, const QContentFilter &filter, QList< Parameter > *parameters );
    QString buildQuery( const QString &queryTemplate, const QContentFilter &filter, const QStringList &sortOrder, QList< Parameter > *parameters );
    void bindParameters( QSqlQuery *query, const QList< Parameter > &parameters );
    QString addParameter( const QVariant &parameter, QList< Parameter > *parameters );
    QString addParameter( const QVariant &parameter, QList< Parameter > *parameters, int *insertAt );

    QSet< QContentFilter::FilterType > getAllFilterTypes( const QContentFilter &filter );
    QSet< QString > getAllSyntheticKeys( const QContentFilter &filter );

    ResultSizeCounter *sizeCounter;
    static QMutex guardMutex;

    QContentIdList getContentIds( const QString &path, bool isLink );
};

#endif
