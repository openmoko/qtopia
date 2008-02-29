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

#include "qcontent.h"
#include "contentlnksql_p.h"
#include "contentlnk_p.h"
#include "drmcontent_p.h"

#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif
#include <qtopialog.h>
#include <qtopiasql.h>
#include <qmimetype.h>
#include <qstorage.h>
#include <qtopiaipcenvelope.h>

#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTextStream>
#include <QSqlError>
#include <QHash>
#include <QSettings>
#include <QtDebug>
#include <QSet>

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

QHash<QString,QString> ContentLinkSql::locNames;
QMutex ContentLinkSql::guardMutex(QMutex::Recursive);

/*!
  \internal
  Obtain a (cached) list of mapping paths to human-readable location name
  strings, which are used in filter expressions such as "loc/CF Card"
  This code is derived from similar code in QStorageMetaInfo::update()

  TODO - examine and verify assumptions about the behaviour of groups
  in QSettings.
*/
const QHash<QString,QString> &ContentLinkSql::getLocNames()
{
    if ( locNames.count() == 0 )
    {
        QSettings cfg("Trolltech", "Storage");
        if ( cfg.status() != QSettings::NoError )
            qWarning( "No valid Storage.conf, using paths instead!" );
#ifdef Q_OS_LINUX
        struct mntent *me;
        FILE *mntfp = setmntent( "/etc/fstab", "r" );
        if ( mntfp )
        {
            while ( (me = getmntent( mntfp )) != 0 )
            {
                QString disk = me->mnt_fsname;
                cfg.beginGroup(disk);
                if ( cfg.status() != QSettings::NoError
                        && !cfg.contains( "Name" ))
                {
                    cfg.endGroup();
                    continue;
                }
                QString path = me->mnt_dir;
                QString name = path;  // default
                name = cfg.value( "Name", name ).toString();
                locNames[ name ] = path;
                cfg.endGroup();
            }
        }
#endif
        if ( locNames.count() == 0 && cfg.status() != QSettings::NoError )
        {
            QStringList allMPs = cfg.childGroups();
            foreach( QString grp, allMPs )
                locNames[ grp ] = grp;
        }
    }
    return locNames;
}

/*
   \internal
   Construct a new SQL object, storing a value object of the database to
   run the SQL against.  The database must support transactions as these
   are used to maintain database integrity for foreign keys.  Also the
   last insert id feature is required.
*/
ContentLinkSql::ContentLinkSql()
    : isError( false )
    , sizeCounter( 0 )
{
    if ( QtopiaSql::systemDatabase().driver()->hasFeature( QSqlDriver::QuerySize ))
        sizeCounter = new NativeSizeCounter();
    else
        sizeCounter = new IteratingSizeCounter();
    checkFeatures();
}

/*
   \internal
   Destruct the object - nothing to do here.
*/
ContentLinkSql::~ContentLinkSql()
{
    delete sizeCounter;
}

QString ContentLinkSql::getCodeFromDocState( ContentLinkPrivate *cl )
{
    switch ( cl->um )
    {
        case QContent::Document :
            return QLatin1String("d");
        case QContent::Application :
            return QLatin1String("a");
        case QContent::Data :
            return QLatin1String("b");
        default:   // unknown
            return QString::null;  // NULL
    };
}

/*
   \internal
   ensure the database used has the features required
*/
void ContentLinkSql::checkFeatures()
{
    if ( !QtopiaSql::systemDatabase().driver()->hasFeature( QSqlDriver::LastInsertId ))
    {
        isError = true;
        errText += QtopiaSql::systemDatabase().driverName() + QLatin1String(" database reports no last insert id support! ");
        qLog(DocAPI) << errText;
    }
}

/*!
  \internal
  Query for a link with the path value equal to \a cl->path() and if found
  update that record with the values from cl

  return true if an update was done
  */
QContentId ContentLinkSql::updateLink( ContentLinkPrivate *cl )
{
    errText = QString::null;
    isError = false;

    if(!cl->linkFile().isEmpty() && !cl->fileKnown())
        return QContent::InvalidId;

    bool isLink = !(cl->linkFile().isEmpty() && cl->fileKnown());

    // QTOPIA_DOCAPI_TODO, foreach in the list, pull out the old entry from the database, compare and modify.
    QContentIdList cidList = getContentIds( isLink ? cl->linkFile() : cl->file(), isLink );

    if( cidList.isEmpty() )
        return QContent::InvalidId;

    return updateLinkUnchecked(cl, cidList);
}

// Do the actual update here for each id passed.
QContentId ContentLinkSql::updateLinkUnchecked(ContentLinkPrivate *cl, const QContentIdList &cidList)
{
    QString path = cl->linkFile().isEmpty() ? cl->file() : cl->linkFile();

    QList< Parameter > parameters;

    errText = QString::null;
    isError = false;
    qLog(DocAPI) << "Updating link:" << cl->name() << cl->type() << cl->cName;
    QString doc = getCodeFromDocState( cl );
    QString qry;
    if ( !cl->cName.isEmpty() )
        qry += QString( QLatin1String("uiName = %1") )
                .arg( addParameter( cl->cName, &parameters ) );

    if ( !cl->type().isEmpty() )
        qry += QString( QLatin1String(", mType = %1") )
                .arg( addParameter( mimeCode(QtopiaSql::databaseIdForPath( path ), cl->type()), &parameters ) );

    if(cl->fileKnown())
    {
        qry += QString( ", path = %1" )
                .arg( addParameter( cl->file().section( QLatin1Char('/'), -1 ), &parameters ) );
        qry += QString( ", location = %2" )
                .arg( addParameter( locationCode( cl->file().section( QLatin1Char('/'), 0, -2 ) ), &parameters ) );
    }

    if ( !cl->iconName().isEmpty() )
        qry += QString( QLatin1String(", icon = %1") )
                .arg( addParameter(cl->iconName(), &parameters) );

    qry += QString( QLatin1String(", drmFlags = %1") )
            .arg( addParameter( drmCode( cl->drm ), &parameters ) );

    if ( !doc.isEmpty() )
        qry += QString( QLatin1String(", docStatus = %1") )
                .arg( addParameter( doc, &parameters ) );

    if ( !cl->mimeTypes().isEmpty() && !cl->mimeTypes().first().isEmpty() )
        qry += QString( ", mimeTypes = %1" )
                .arg( addParameter( cl->mimeTypes().join( QLatin1String(";") ), &parameters ) );

    QFileInfo fi( path );

    qry += QString( ", lastUpdated = %1" )
            .arg( addParameter( fi.lastModified().toTime_t(), &parameters ) );

    if ( qry[0] == QLatin1Char(',') )
        qry = qry.mid( 2 );
    qry += QLatin1String(" WHERE ");

    QString conjunct;
    QContentId cid = QContent::InvalidId;
    foreach(cid, cidList) {
        qry += conjunct + QString( "cid = %1" )
                .arg( addParameter( cid.second, &parameters ) );
        conjunct = QLatin1String(" OR ");
    }

    qry = QLatin1String("UPDATE content SET ") + qry;

    QSqlQuery query( QtopiaSql::database(cidList.first().first) );
    query.prepare( qry );

    bindParameters( &query, parameters );

    QtopiaSql::logQuery( query );

    if ( !query.exec() )
    {
        qLog(DocAPI) << "ContentLinkSql::updateLinkUnchecked: Query failed!! qry =" << qry << endl
                << "error =" << query.lastError();
        isError = true;
        errText += query.lastError().text();
        ContentLinkPrivate::errors[QContent::InvalidId] = errText;
        return QContent::InvalidId;
    }
    return cid;  // should prob check num records affected here
}


QContentIdList ContentLinkSql::getContentIds( const QString &path, bool isLink )
{
    static const QString realQuery = QLatin1String(
            "SELECT cid "
            "FROM content left join locationLookup on content.location = locationLookup.pKey "
            "WHERE path = :path and locationLookup.location = :location" );

    static const QString linkQuery = QLatin1String(
            "SELECT cid "
            "FROM content left join locationLookup on content.linkLocation = locationLookup.pKey "
            "WHERE linkFile = :path and locationLookup.location = :location" );

    QtopiaDatabaseId dbId = QtopiaSql::databaseIdForPath( path );
    QSqlDatabase db = QtopiaSql::database( dbId );

    qLog(DocAPI) << "Checking if file exists in the content database:" << path;

    QSqlQuery qUL_qry( db );
    qUL_qry.prepare( isLink ? linkQuery : realQuery );

    qUL_qry.bindValue( QLatin1String("path"), path.section( QLatin1Char('/'), -1 ) );
    qUL_qry.bindValue( QLatin1String("location"), path.section( QLatin1Char('/'), 0, -2 ) );

    QtopiaSql::logQuery( qUL_qry );

    if ( !qUL_qry.exec() )
    {
        qLog(DocAPI) << "ContentLinkSql::getContentId: Query failed!! qry=" << qUL_qry.lastQuery() << endl
                << "error=" << qUL_qry.lastError();
        isError = true;
        errText += qUL_qry.lastError().text();
        ContentLinkPrivate::errors[QContent::InvalidId] = errText;
        return QContentIdList();
    }

    QContentIdList cidList;

    while (qUL_qry.next())
        cidList.append(QContentId(dbId, qUL_qry.value(0).toULongLong()));

    return cidList;
}

/*!
  Post a created QContent into the database.
*/
QContentId ContentLinkSql::postLink( ContentLinkPrivate *cl, QContent::ChangeType &change )
{
    QMutexLocker lock(&guardMutex);

    bool isLink = !(cl->linkFile().isEmpty() && cl->fileKnown());

    QString path = isLink ? cl->linkFile() : cl->file();

    errText = QString::null;
    isError = false;

    if( cl->cName.isEmpty() || (!cl->linkFile().isEmpty() && !cl->fileKnown()) )
    {   // shouldn't be here as it's really not a valid QContent
        return QContent::InvalidId;
    }

    if( cl->cId != QContent::InvalidId )
    {
        QContentId cid = updateLinkUnchecked(cl, QContentIdList() << cl->cId );

        if ( cid != QContent::InvalidId )
        {
            change = QContent::Updated;
            return cid;
        }
    }
    else
    {
        // See if the record exists and update if so.
        QContentIdList cidList = getContentIds( path, isLink );

        if( !cidList.isEmpty() )
        {
            QContentId cid = updateLinkUnchecked(cl, cidList);

            if ( cid != QContent::InvalidId )
            {
                change = QContent::Updated;
                return cid;
            }
        }
    }

    QtopiaDatabaseId dbid=QtopiaSql::databaseIdForPath( path );

    QSqlDatabase db=QtopiaSql::database(dbid);

    qLog(DocAPI) << "Posting: " << cl->cName << cl->type() << cl->drm << getCodeFromDocState( cl ) << (cl->fileKnown() ? cl->file() : QString::null) << cl->iconName() << cl->linkFile() << cl->mimeTypes();

    QString types;
    if (cl->mimeTypes().count()) {
        types = cl->mimeTypes().join( QLatin1String(";") );
    }
    QSqlQuery aPL_qry( db );  // NO TR
    aPL_qry.prepare( QLatin1String("INSERT INTO content ") +
            QLatin1String("(uiName, mType, drmFlags, docStatus, path, location, icon, linkFile, linkLocation, mimeTypes, lastUpdated) "
            "VALUES (:uiName, :mType, :drmFlags, :docStatus, :path, :location, :icon, :linkFile, :linkLocation, :mimeTypes, :lastUpdated)"));
    aPL_qry.bindValue( QLatin1String("uiName"), cl->cName );
    aPL_qry.bindValue( QLatin1String("mType"), mimeCode(QtopiaSql::databaseIdForPath( path ), cl->type()) );
    aPL_qry.bindValue( QLatin1String("drmFlags"), QVariant( drmCode( cl->drm ) ) );
    aPL_qry.bindValue( QLatin1String("docStatus"), getCodeFromDocState( cl ));
    if(cl->fileKnown())
    {
        aPL_qry.bindValue( QLatin1String("path"), cl->file().section( QLatin1Char('/'), -1 ) );
        aPL_qry.bindValue( QLatin1String("location"), locationCode( cl->file().section( QLatin1Char('/'), 0, -2 ) ) );
    }
    else
    {
        aPL_qry.bindValue( QLatin1String("path"), "" );
        aPL_qry.bindValue( QLatin1String("location"), 0 );
    }
    aPL_qry.bindValue( QLatin1String("icon"), cl->iconName() );
    if(!cl->linkFile().isEmpty())
    {
        aPL_qry.bindValue( QLatin1String("linkFile"), cl->linkFile().section( QLatin1Char('/'), -1 ) );
        aPL_qry.bindValue( QLatin1String("linkLocation"), locationCode( cl->linkFile().section( QLatin1Char('/'), 0, -2 ) ) );
    }
    else
    {
        aPL_qry.bindValue( QLatin1String("linkFile"), "" );
        aPL_qry.bindValue( QLatin1String("linkLocation"), 0 );
    }
    aPL_qry.bindValue( QLatin1String("mimeTypes"), types );
    aPL_qry.bindValue( QLatin1String("lastUpdated"), cl->modifiedDate().toTime_t());
    QVariant v;
    if ( QtopiaSql::exec(aPL_qry, db, !cl->batchLoading).type() != QSqlError::NoError )
    {
        qLog(DocAPI) << "Query Failed for" << cl->cName
                << "\nqry =" << aPL_qry.lastQuery()
                << "\nerror =" << aPL_qry.lastError ();
        isError = true;
        errText += aPL_qry.lastError().text();
        ContentLinkPrivate::errors[QContent::InvalidId] = errText;
        return QContent::InvalidId;
    }
    if (aPL_qry.numRowsAffected () == 0)
    {
        qLog(DocAPI) << "Insert Failed for" << cl->cName
                << "\nqry =" << aPL_qry.lastQuery()
                << "\nerror =" << aPL_qry.lastError ();
        isError = true;
        errText += aPL_qry.lastError().text();
        ContentLinkPrivate::errors[QContent::InvalidId] = errText;
        return QContent::InvalidId;
    }

    // at present, no need for a transaction
    // QtopiaSql::database().commit();
    v = aPL_qry.lastInsertId();
    change = QContent::Added;
    return QContentId( dbid, v.toULongLong() );
}

/*!
  Remove a QContent from the backing database, given the
  \a cid record number.  If the operation failed, because for
  example there was a problem with the database, returns false and
  error() returns true.  If the operation failed because there
  was not exactly one matching \a cid return false.

  The database should have a integrity mechanism such that
  any entries in the categorymap table which reference the removed
  \a cid are also removed (otherwise dangling references would
  clutter the table).  In SQLite this is done with triggers, in
  MySQL and others it is done with InnoDB foreign key constraint
  clauses.
*/
bool ContentLinkSql::removeLink( QContentId cid )
{
    errText = QString::null;
    isError = false;
    if (cid==QContent::InvalidId)
        return false;
    QSqlDatabase db=QtopiaSql::database(cid.first);
    QSqlQuery dfContent( db );
    dfContent.prepare( QLatin1String("DELETE FROM content WHERE cid=:cid") );
    QSqlQuery dfCategoryMap( db );
    dfCategoryMap.prepare( QLatin1String("DELETE FROM mapCategoryToContent WHERE cid=:cid") );
    QSqlQuery dfContentProps( db );
    dfContentProps.prepare( QLatin1String("DELETE FROM contentProps WHERE cid=:cid") );
    dfContent.bindValue( QLatin1String("cid"), cid.second );
    dfCategoryMap.bindValue( QLatin1String("cid"), cid.second );
    dfContentProps.bindValue( QLatin1String("cid"), cid.second );
    QtopiaSql::logQuery( dfContent );
    QtopiaSql::logQuery( dfCategoryMap );
    QtopiaSql::logQuery( dfContentProps );
    if ( !dfContent.exec() || !dfCategoryMap.exec() || !dfContentProps.exec() )
    {
        QSqlError error;
        if(dfContent.lastError().isValid())
            error = dfContent.lastError();
        else if(dfCategoryMap.lastError().isValid())
            error = dfCategoryMap.lastError();
        else if(dfContentProps.lastError().isValid())
            error = dfContentProps.lastError();
        qLog(DocAPI) << "ContentLinkSql::removeLink: Query failed!! qry=" << dfContent.lastQuery()
                << "\n|| qry=" << dfCategoryMap.lastQuery()
                << "\n|| qry=" << dfContentProps.lastQuery()
                << "\nerror =" << error;
        isError = true;
        errText += error.text();
        ContentLinkPrivate::errors[cid] = errText;
        return false;
    }

    if ( sizeCounter->supportsNumRowsAffected() )
        if ( dfContent.numRowsAffected() != 1 )
        {
            isError = true;
            errText += QString( "Remove link by Id (%1,%2) affected %3 records!" )
                    .arg( cid.first ).arg( cid.second ).arg( dfContent.numRowsAffected() );
            ContentLinkPrivate::errors[cid] = errText;
            return false;
        }

    return true;
}

/*!
   Return the categories assigned to a particular contentlink, given the
   \a cid ContentLink record number.  If no categories are assigned an
   empty hash is returned.  If there is an error with the query, an empty
   hash is returned and additionally isError() returns true.
*/
 QStringList ContentLinkSql::categoriesById( QContentId cid )
{
    errText = QString::null;
    isError = false;
    QStringList result;
    if(cid == QContent::InvalidId)
        return result;
    QSqlQuery qCBI_qry( QtopiaSql::database(cid.first) );
    qCBI_qry.prepare( QLatin1String("SELECT mapCategoryToContent.categoryid "
                "FROM mapCategoryToContent INNER JOIN categories "
            "ON categories.categoryid=mapCategoryToContent.categoryid WHERE mapCategoryToContent.cid=:cid") );
    qCBI_qry.bindValue( "cid", cid.second );
    QtopiaSql::logQuery( qCBI_qry );
    if ( !qCBI_qry.exec() )
    {
        qLog(DocAPI) << "ContentLinkSql::categoriesById: Query failed!! qry=" << qCBI_qry.lastQuery()
                << "\nerror=" << qCBI_qry.lastError();
        isError = true;
        errText += qCBI_qry.lastError().text();
        return result;
    }
    while ( qCBI_qry.next() )
        result << qCBI_qry.value( 0 ).toString();
    return result;
}

int ContentLinkSql::recordCount( const QContentFilter &filter )
{
    QList< Parameter > parameters;

    QString queryString = QLatin1String(
            "select count(distinct cid) "
            "from %1 "
            "where %2" );

    queryString = buildQuery( queryString, filter, &parameters );

    if( queryString.isEmpty() )
        return 0;

    int count = 0;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
    {
        QSqlQuery query( QtopiaSql::database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::logQuery(query);
        if( query.exec() && query.first() )
            count += qvariant_cast< int >( query.value( 0 ) );
        else
        {
            qLog(DocAPI) << "ContentLinkSql::recordCount() query failed";
            qLog(DocAPI) << queryString;
        }
    }
    return count;
}

bool ContentLinkSql::addDRMFlagsColumn( QSqlQuery &xsql )
{
    if( xsql.exec(QLatin1String("alter table content add drmFlags INTEGER")) &&
        xsql.exec(QLatin1String("update content set drmFlags = 65536 where drmStatus = 'p'") ) &&
        xsql.exec(QLatin1String("update content set drmFlags = 32768 where drmStatus != 'p'") ) )
    {
        return true;
    }
    else
        return false;
}


bool ContentLinkSql::removeCategoryMap(QContentId cid)
{
    errText = QString::null;
    isError = false;
    if(cid == QContent::InvalidId)
        return false;
    QSqlQuery removeExistingQry( QtopiaSql::database(cid.first) ); // NO TR
    removeExistingQry.prepare( QLatin1String("DELETE FROM mapCategoryToContent WHERE cid=:cid") );
    removeExistingQry.bindValue(QLatin1String("cid"), cid.second);
    QtopiaSql::logQuery( removeExistingQry );
    if (!removeExistingQry.exec()) {
        qLog(DocAPI) << "ContentLinkSql::removeCategoryMap: Query failed!! qry=" << removeExistingQry.lastQuery()
                << "\nerror=" << removeExistingQry.lastError()
                << "\nFailed removing categories for" << cid;
        return false;
    }

    return true;
}

/*
   \internal
   Add a new Category entry for a record.
   The record number of the newly created record is returned.
   Integrity constraints are checked, but if a matching \a cat is not found in the
   categories table it is assumed to be a new category and added.  If this occurs
   the \a trans is used as the translation.
   */
bool ContentLinkSql::appendNewCategoryMap( const QString &cat, const QString &scope, const QContentId cid, const QString &trans, bool useTransaction )
{
    errText = QString::null;
    isError = false;

    if(cid == QContent::InvalidId)
        return false;

    QSqlDatabase db=QtopiaSql::database(cid.first);
    if(cat.isEmpty())
    {
        qLog(DocAPI) << "Trying to append empty category to cid(" << cid << "), ignoring";
        return false;
    }

    qLog(DocAPI) << "Adding cat" << cat << "to" << cid;

    //
    // first check that the cid and the category we want to map both still exist
    // there must be exactly one row returned from the foreign key checks

    {
        QSqlQuery aNCM_fkchk_cid_qry( db );  // NO TR
        aNCM_fkchk_cid_qry.prepare( QLatin1String("SELECT * FROM content WHERE cid=:cid") );

        aNCM_fkchk_cid_qry.bindValue( QLatin1String("cid"), cid.second );

        QtopiaSql::logQuery( aNCM_fkchk_cid_qry );
        if ( !aNCM_fkchk_cid_qry.exec() )
        {
            isError = true;
            errText += aNCM_fkchk_cid_qry.lastError().text();
            qLog(DocAPI) << "couldn't update categories for" << cid;
            return false;
        }

        if ( !hasRecords( aNCM_fkchk_cid_qry ) )
        {
            isError = true;
            errText += QString( "content id (%1, %2) didnt exist" ).arg( cid.first ).arg( cid.second );
            errText += aNCM_fkchk_cid_qry.lastError().text();
            qLog(DocAPI) << "couldn't update categories for" << cid;
            return false;
        }
    }

    {
        static const QString selectString = QLatin1String("SELECT * FROM categories WHERE categoryid=:categoryid");
        QSqlQuery aNCM_fkchk_category_qry( db );  // NO TR
        aNCM_fkchk_category_qry.prepare( selectString );

        aNCM_fkchk_category_qry.bindValue( QLatin1String("categoryid"), cat );

        QtopiaSql::logQuery( aNCM_fkchk_category_qry );
        if ( !aNCM_fkchk_category_qry.exec() )
        {
            isError = true;
            errText += aNCM_fkchk_category_qry.lastError().text();
            qLog(DocAPI) << "couldn't update categories for" << cid;
            return false;
        }

        if ( !hasRecords( aNCM_fkchk_category_qry ) )
        {
            bool found = false;
            QString catLabel;
            QString catScope;
            QString catIcon;
            int catFlags = 0;

            foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
            {
                if( dbId == cid.first )
                    continue;

                QSqlQuery query( QtopiaSql::database( dbId ) );
                query.prepare( selectString );

                query.bindValue( QLatin1String("categoryid"), cat );

                QtopiaSql::logQuery( query );

                if( query.exec() && query.first() )
                {
                    QSqlRecord record = query.record();

                    catLabel = record.value( QLatin1String( "categorytext" ) ).toString();
                    catScope = record.value( QLatin1String( "categoryscope" ) ).toString();
                    catIcon = record.value( QLatin1String( "categoryicon" ) ).toString();
                    catFlags = record.value( QLatin1String( "flags" ) ).toInt();

                    found = true;

                    break;
                }
            }

            QSqlQuery aNCM_cat_qry( db );  // NO TR
            aNCM_cat_qry.prepare( QLatin1String("INSERT INTO categories ( categoryid, categorytext, categoryscope, categoryicon, flags ) VALUES ( :categoryid, :categorytext, :categoryscope, :categoryicon, :flags )") );

            if( found )
            {
                aNCM_cat_qry.bindValue( QLatin1String("categoryid"), cat );
                aNCM_cat_qry.bindValue( QLatin1String("categorytext"), catLabel );
                aNCM_cat_qry.bindValue( QLatin1String("categoryscope"), catScope );
                aNCM_cat_qry.bindValue( QLatin1String("categoryicon"), catIcon );
                aNCM_cat_qry.bindValue( QLatin1String("flags"), catFlags );
            }
            else
            {
                aNCM_cat_qry.bindValue( QLatin1String("categoryid"), cat );
                aNCM_cat_qry.bindValue( QLatin1String("categorytext"), trans.isEmpty() ? cat : trans );
                aNCM_cat_qry.bindValue( QLatin1String("categoryscope"), scope );
                aNCM_cat_qry.bindValue( QLatin1String("categoryicon"), QString() );
                aNCM_cat_qry.bindValue( QLatin1String("flags"), 0 );
            }
            if (QtopiaSql::exec(aNCM_cat_qry, db, useTransaction).type() != QSqlError::NoError)
            {
                isError = true;
                errText += aNCM_cat_qry.lastError().text();
                qLog(DocAPI) << "couldn't insert category" << cat << "for" << cid;
                return false;
            } else {
                QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("categoriesChanged()") );
            }
        }
        else
        {
            qLog(DocAPI) << "Category" << cat << "has at least 1 match(es) in the database, not inserting new";
        }
    }


    //
    // now do the actual update
    QSqlQuery aNCM_qry( db );  // NO TR
    aNCM_qry.prepare( QLatin1String("INSERT INTO mapCategoryToContent ( categoryid, cid ) VALUES ( :categoryid, :cid )") );

    aNCM_qry.bindValue( QLatin1String("categoryid"), cat );
    aNCM_qry.bindValue( QLatin1String("cid"), cid.second );
    if ( QtopiaSql::exec(aNCM_qry, db, useTransaction).type() != QSqlError::NoError )
    {
        isError = true;
        errText += aNCM_qry.lastError().text();
        qLog(DocAPI) << "couldn't update categories for" << cid;
        return false;
    }

    return true;
}

QList< ContentLinkSql::MimeData > ContentLinkSql::getMimeData()
{
    QList< MimeData > data;

    QString queryString = QLatin1String(
            "select content.cid, content.mimeTypes, icons.value, perms.value "
            "from content "
                "left join contentProps as icons ON content.cid = icons.cid and icons.grp = :iconGroup and icons.name = :iconKey "
                "left join contentProps as perms ON content.cid = perms.cid and perms.grp = :permGroup and perms.name = :permKey "
            "where not content.mimeTypes is null" );

    QContentIdList ids;

    foreach(QtopiaDatabaseId dbid, QtopiaSql::databaseIds())
    {
        QSqlQuery query( QtopiaSql::database( dbid ) );
        query.prepare( queryString );

        query.bindValue( QLatin1String( "iconGroup" ), QLatin1String( "none"                ) );
        query.bindValue( QLatin1String( "iconKey"   ), QLatin1String( "MimeTypeIcons"       ) );
        query.bindValue( QLatin1String( "permGroup" ), QLatin1String( "DRM"                 ) );
        query.bindValue( QLatin1String( "permKey"   ), QLatin1String( "MimeTypePermissions" ) );

        QtopiaSql::logQuery( query );

        if ( query.exec() )
        {
            while ( query.next() )
            {
                MimeData mime =
                {
                    QContentId( dbid, query.value( 0 ).toULongLong() ),
                    query.value( 1 ).toString(),
                    query.value( 2 ).toString(),
                    query.value( 3 ).toString()
                };

                data.append( mime );

                ids.append( mime.cid );
            }
        }
    }

    QContent::cache( ids );

    return data;
}

/*!
  \internal
  Return a list of all records in the data base which match all of the filter
  expressions in \a filter.

  If \a idList is non-empty, only check those records and return a list of the
  ones which match.  This is acheived by creating a temporary table with those
  record ids and running the select with a join on the temporary table.
 */
QContentIdList ContentLinkSql::matches( const QContentIdList &idList, const QContentFilter &filter, const QStringList &sortOrder )
{
    QContentIdList results;

    if( !filter.isValid() )
        return results;

    int insertAt = 0;
    QList< Parameter > parameters;
    QStringList joins;

    QString where = QLatin1String(" WHERE ") + buildWhereClause( filter, &parameters, &insertAt, &joins );
    QString order = buildOrderBy( sortOrder, &parameters, &insertAt, &joins );
    QString qry = QLatin1String( "SELECT DISTINCT content.cid FROM " ) + buildFrom( filter, joins );

    foreach(QtopiaDatabaseId dbid, QtopiaSql::databaseIds())
    {
        QString cidString;

        if( !idList.isEmpty() )
        {
            foreach(QContentId id, idList)
                if(id.first == dbid)
                    cidString += QString::number(id.second) + ",";

            if(!cidString.isEmpty())
                cidString.chop( 1 );

            cidString = QLatin1String(" and content.cid in (") + cidString + QLatin1String(") ");
        }

        QSqlQuery qGM_qry( QtopiaSql::database(dbid) );
        qGM_qry.prepare( qry + where + cidString + order );

        bindParameters( &qGM_qry, parameters );

        QtopiaSql::logQuery( qGM_qry );
        if ( !qGM_qry.exec())
        {
            qLog(DocAPI) << "ContentLinkSql::matches: Query failed!! qry=" << qGM_qry.lastQuery()
                    << QLatin1String("\nerror=") << qGM_qry.lastError();
            isError = true;
                errText += qGM_qry.lastError().text();

                continue;
        }

        while ( qGM_qry.next() )
                results.append( QContentId( dbid, qGM_qry.value( 0 ).toULongLong()));
    }
    //qLog(DocAPI) << "results =" << results;
    return results;
}

QString ContentLinkSql::buildWhereClause( const QContentFilter &filter, QList< Parameter > *parameters, int *insertAt, QStringList *joins )
{
    // build up the WHERE clause
    QString operandConjunct = filter.operand() == QContentFilter::Or ? QLatin1String( " OR " ) : QLatin1String( " AND " );
    QString conjunct = QLatin1String(" ");
    QString qry;
    QString part;

    part = buildQtopiaTypes( filter.arguments( QContentFilter::Role ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildPaths( filter.arguments( QContentFilter::Location ), filter.arguments( QContentFilter::Directory ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildMimeTypes( filter.arguments( QContentFilter::MimeType ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildCategories( filter.arguments( QContentFilter::Category ), operandConjunct, parameters, insertAt, joins );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildSynthetic( filter.arguments( QContentFilter::Synthetic ), operandConjunct, parameters, insertAt, joins );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildDrm( filter.arguments( QContentFilter::DRM ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;

        conjunct = operandConjunct;
    }

    foreach( QContentFilter subFilter, filter.subFilters() )
    {
        QString subQuery = buildWhereClause( subFilter, parameters, insertAt, joins );

        if( !subQuery.isEmpty() )
        {
            qry += conjunct + QLatin1String( "(" ) + subQuery + QLatin1String( ")" );

            conjunct = operandConjunct;
    }
    }

    if( filter.negated() )
        return QLatin1String( "NOT " ) + qry;
    else
    return qry;
}

QString ContentLinkSql::buildQtopiaTypes( const QStringList &types, const QString &conjunct, QList< Parameter > *parameters )
{
    if (types.isEmpty())
        return QString::null;

    QString expression;
    QString c;

    foreach( QString type, types )
    {
        QString value;

        if ( type == QLatin1String("Document") )
            value = QLatin1String( "d" );
        else if ( type == QLatin1String("Data") )
            value = QLatin1String( "b" );
        else if( type == QLatin1String( "Folder" ) )
            value = QLatin1String( "f" );
        else if( type == QLatin1String( "Archive" ) )
            value = QLatin1String( "x" );
        else
            value = QLatin1String( "a" );

        expression += c + QString( "docStatus = %1" ).arg( addParameter( value, parameters ) );

        c = conjunct;
    }

    if( types.size() == 1 )
        return expression;
    else
        return QString( "(%1)" ).arg( expression );
}

QString ContentLinkSql::buildPaths( const QStringList &locations, const QStringList &directories, const QString &conjunct, QList< Parameter > *parameters )
{
    if (locations.isEmpty() && directories.isEmpty())
        return QString();

    QString expression;
    QString bracketedConjunct = QString( ")%1(" ).arg( conjunct );
    QString c;

    foreach( QString location, locations )
    {
        location = QDir::cleanPath( location );

        if( location.contains( QLatin1Char('*') ) )
        {
            location.replace( QLatin1Char('*'), QLatin1Char('%') );

            expression += c + QString( "real.location LIKE %1 OR link.location LIKE %2" )
                    .arg( addParameter( location, parameters ) )
                    .arg( addParameter( location, parameters ) );

            c = bracketedConjunct;
        }
        else
        {
            if( location.endsWith( QLatin1Char('/') ) )
                location.chop( 1 );

            expression += c + QString(
                    "real.location LIKE %1 OR link.location LIKE %2 OR "
                    "real.location = %3 OR link.location = %4" )
                    .arg( addParameter( location + QLatin1String( "/%" ), parameters ) )
                    .arg( addParameter( location + QLatin1String( "/%" ), parameters )  )
                    .arg( addParameter( location, parameters ) )
                    .arg( addParameter( location, parameters ) );

            c = bracketedConjunct;
        }
    }

    foreach( QString directory, directories )
    {
        directory = QDir::cleanPath( directory );

        if( directory.contains( QLatin1Char('*') ) )
        {
            directory.replace( QLatin1Char('*'), QLatin1Char('%') );

            expression += c + QString( "real.location LIKE %1 OR link.location LIKE %2" )
                    .arg( addParameter( directory, parameters ) )
                    .arg( addParameter( directory, parameters ) );

            c = bracketedConjunct;
        }
        else
        {
            if( directory.endsWith( QLatin1Char('/') ) )
                directory.chop( 1 );

            expression += c + QString( "real.location = %1 OR link.location = %2" )
                    .arg( addParameter( directory, parameters ) )
                    .arg( addParameter( directory, parameters ) );

            c = bracketedConjunct;
    }
    }

    return !expression.isEmpty() ? QString( "(%1)" ).arg( expression ) : QString();
}

QString ContentLinkSql::buildMimeTypes( const QStringList &mimes, const QString &conjunct, QList< Parameter > *parameters )
{
    if( mimes.isEmpty() )
        return QString();

    QString expression;
    QString c;

    foreach( QString mime, mimes )
    {
        if( mime.contains( QLatin1Char('*') ) )
        {
            mime.replace( QLatin1Char('*'), QLatin1Char('%') );

            expression += c + QString( "mimeTypeLookup.mimeType LIKE %1" )
                    .arg( addParameter( mime, parameters ) );
        }
        else
        {
            expression += c + QString( "mimeTypeLookup.mimeType = %1" )
                    .arg( addParameter( mime, parameters ) );
        }

        c = conjunct;
    }

    return !expression.isEmpty() ? QString( "(%1)" ).arg( expression ) : QString();
}

QString ContentLinkSql::buildCategories( const QStringList &categories, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins )
{
    QString expression;
    QString c;

    foreach( QString category, categories )
    {
        QString table = QString( "cat%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

        if( category == QLatin1String( "Unfiled" ) )
        {
            joins->append( QString( " left join mapCategoryToContent as %1 on %1.cid = content.cid" ).arg( table ) );

            expression += c + QString( "%1.categoryid is NULL" ).arg( table );
        }
        else if( category.contains( "*" ) )
        {
            category.replace(QLatin1Char('*'), QLatin1Char('%'));

            joins->append( QString( " left join mapCategoryToContent as %1 on %1.cid = content.cid and %1.categoryid like %2" )
                    .arg( table )
                    .arg( addParameter( category, parameters, insertAt ) ) );

            expression += c + QString( "%1.categoryid is not NULL" ).arg( table );
        }
        else
        {
            joins->append( QString( " left join mapCategoryToContent as %1 on %1.cid = content.cid and %1.categoryid = %2" )
                    .arg( table )
                    .arg( addParameter( category, parameters, insertAt ) ) );

            expression += c + QString( "%1.categoryid is not NULL" ).arg( table );
        }

        c = conjunct;
    }

    return expression;
}

QString ContentLinkSql::buildDrm( const QStringList &drm, const QString &conjunct, QList< Parameter > *parameters )
{
    QString expression;
    QString c;

    foreach( QString filter, drm )
    {
        if( filter == QLatin1String( "Protected" ) )
        {
            expression += c + QString( "drmFlags != %1" )
                    .arg( addParameter( QVariant( 65536 ), parameters ) );

            c = conjunct;
        }
        else if( filter == QLatin1String( "Unprotected" ) )
        {
            expression += c + QString( "drmFlags == :param%1" )
                    .arg( addParameter( QVariant( 65536 ), parameters ) );

            c = conjunct;
        }
    }

    return expression;
}

bool ContentLinkSql::writeProperty(QContentId cId, const QString& key, const QString& value, const QString& group )
{
    QString grp=group.trimmed();
    if( grp.isEmpty() )
        grp = QLatin1String("none");
    if ( cId == QContent::InvalidId )
        return false;

    bool result = false;

    if( value.isEmpty() )
    {
        QSqlQuery removeQuery( QtopiaSql::database( cId.first ) );
        removeQuery.prepare( QLatin1String( "delete from contentProps where cid=:cid and name=:name and grp=:grp" ) );

        removeQuery.bindValue( QLatin1String( "cid" ), cId.second );
        removeQuery.bindValue( QLatin1String( "name" ), key );
        removeQuery.bindValue( QLatin1String( "grp" ), grp );

        QtopiaSql::logQuery( removeQuery );

        if( removeQuery.exec() )
        {
            result = true;
        }
        else
        {
            qLog(DocAPI) << "Failed to delete property" << group << key << "from content" << cId;
        }
    }
    else
    {
        QSqlQuery qProperties_qry( QtopiaSql::database(cId.first) );  // this cant be prepared
        qProperties_qry.prepare( QLatin1String("select count(*) from contentProps where cid=:cid and name=:name and grp=:grp") );  // NO TR
        qProperties_qry.bindValue( QLatin1String("cid"), cId.second );
        qProperties_qry.bindValue( QLatin1String("name"), key );
        qProperties_qry.bindValue( QLatin1String("grp"), grp );

        QtopiaSql::logQuery( qProperties_qry );
        if ( qProperties_qry.exec() )
        {
            bool update = qProperties_qry.first() && qProperties_qry.value(0).toInt() > 0;
            qProperties_qry.clear();
            if (update) {
                QSqlQuery qPropertiesUpdate_qry( QtopiaSql::database(cId.first) );  // no tr
                qPropertiesUpdate_qry.prepare( QLatin1String("update contentProps set value = :value where cid = :cid and grp = :grp and name = :name") );
                qPropertiesUpdate_qry.bindValue( QLatin1String("value"), value );
                qPropertiesUpdate_qry.bindValue( QLatin1String("cid"), cId.second );
                qPropertiesUpdate_qry.bindValue( QLatin1String("grp"), grp );
                qPropertiesUpdate_qry.bindValue( QLatin1String("name"), key );
                QtopiaSql::logQuery( qPropertiesUpdate_qry );
                result = qPropertiesUpdate_qry.exec();
            } else {
                QSqlQuery qPropertiesInsert_qry( QtopiaSql::database(cId.first) );  // no tr
                qPropertiesInsert_qry.prepare( QLatin1String("insert into contentProps(cid,name,value,grp) values (:cid, :name, :value, :grp)") );
                qPropertiesInsert_qry.bindValue( QLatin1String("cid"), cId.second );
                qPropertiesInsert_qry.bindValue( QLatin1String("name"), key );
                qPropertiesInsert_qry.bindValue( QLatin1String("value"), value );
                qPropertiesInsert_qry.bindValue( QLatin1String("grp"), grp );
                QtopiaSql::logQuery( qPropertiesInsert_qry );
                result = qPropertiesInsert_qry.exec();
            }
        }
        else
        {
            qLog(DocAPI) << "ContentLinkSql::writeProperty: Query failed!! qry=" << qProperties_qry.lastQuery()
                    << "\nerror=" << qProperties_qry.lastError();
        }
    }
    return result;
}

QMap<QPair<QString,QString>,QString> ContentLinkSql::readProperties( const QContentId cId )
{
    QMap<QPair<QString,QString>,QString> result;
    if ( cId == QContent::InvalidId )
        return result;
    errText = QString::null;
    isError = false;

    QSqlQuery qRP_qry( QtopiaSql::database(cId.first) );
    qRP_qry.prepare( QLatin1String("SELECT grp, name, value  FROM contentProps WHERE cid=:cid") );
    qRP_qry.bindValue( QLatin1String("cid"), cId.second );

    QtopiaSql::logQuery( qRP_qry );
    if ( !qRP_qry.exec() ) {
        qLog(DocAPI) << "ContentLinkSql::readProperties: Query failed!! qry=" << qRP_qry.lastQuery()
                << "\nerror=" << qRP_qry.lastError();
        isError = true;
        errText += qRP_qry.lastError().text();
        return result;
    }
    if ( !hasRecords(qRP_qry) )
        return result;

    static const QString None(QLatin1String("none"));
    qRP_qry.seek(-1, false);
    while (qRP_qry.next()) {
        QString grp = qRP_qry.value(0).toString();
        if (grp == None)
            grp = QString();
        QPair<QString,QString> key(qRP_qry.value(1).toString(),grp);
        result.insert(key, qRP_qry.value(2).toString());
    }

    return result;
}

uint qHash(QPair<QtopiaDatabaseId, QString> in)
{
    return qHash(QString::number(in.first)+in.second);
}

int ContentLinkSql::mimeCode(const QtopiaDatabaseId &dbid, const QString &mimeType)
{
    typedef QPair<QtopiaDatabaseId, QString> cacheKeyType;
    static QCache<cacheKeyType, int> mimeCodeCache;
    cacheKeyType cacheKey(dbid, mimeType);
    if(mimeCodeCache.contains(cacheKey))
        return *mimeCodeCache[cacheKey];
    QSqlQuery location(QtopiaSql::database(dbid));
    location.prepare(QLatin1String("select pKey from mimeTypeLookup where mimeType = :mimeType"));
    location.bindValue(QLatin1String("mimeType"), mimeType);
    QtopiaSql::logQuery( location );
    if (location.exec())
    {
        if (location.first())
        {
            mimeCodeCache.insert(cacheKey, new int(location.value(0).toInt()));
            return location.value(0).toInt();
        }
        else
        {
            location.prepare(QLatin1String("insert into  mimeTypeLookup (mimeType) values (:mimeType)"));
            location.bindValue(QLatin1String("mimeType"), mimeType);
            QtopiaSql::logQuery( location );
            if (location.exec())
            {
                location.clear();
                int result=mimeCode(dbid, mimeType);
                mimeCodeCache.insert(cacheKey, new int(result));
                return result;
            }
        }
    }
    return 0;
}

int ContentLinkSql::drmCode( QContent::DrmState state )
{
    if( state == QContent::Unprotected )
        return 65536;
    else if( state == QContent::Protected )
        return 32768;
    else
        return 0;
}

struct CachedLocationId
{
    int locationId;
    QtopiaDatabaseId databaseId;
};

int ContentLinkSql::locationCode( const QString &location )
{
    static QCache< QString, CachedLocationId > locationCache;

    QtopiaDatabaseId dbId = QtopiaSql::databaseIdForPath( location );

    CachedLocationId *cached = locationCache.contains(location) ? locationCache[ location ] : NULL;

    if( cached && cached->databaseId == dbId )
        return cached->locationId;

    int locationId = queryLocationCode( location );

    cached = new CachedLocationId;

    cached->databaseId = dbId;
    cached->locationId = locationId;

    locationCache.insert( location, cached );

    return locationId;
}

int ContentLinkSql::queryLocationCode( const QString &location )
{
    QSqlDatabase db = QtopiaSql::database( QtopiaSql::databaseIdForPath( location ) );

    QSqlQuery lookupQuery( db );
    lookupQuery.prepare( QLatin1String( "select pKey from locationLookup where location = :location" ) );

    lookupQuery.bindValue( QLatin1String( "location" ), location );

    QtopiaSql::logQuery( lookupQuery );

    if( lookupQuery.exec() )
    {
        if( lookupQuery.first() )
            return qvariant_cast< int >( lookupQuery.value( 0 ) );
        else
    {
            QSqlQuery insertQuery( QLatin1String( "insert into locationLookup(location) values(:location)" ), db );

            insertQuery.bindValue( QLatin1String( "location" ), location );

            QtopiaSql::logQuery( insertQuery );

            if( insertQuery.exec() )
            {
                if(insertQuery.driver()->hasFeature(QSqlDriver::LastInsertId))
                    return insertQuery.lastInsertId ().toInt();
                else
                    return queryLocationCode( location );
            }
        }
    }
    return 0;
}

bool ContentLinkSql::queryFailed(const QSqlQuery &qry)
{
    if(qry.lastError().isValid())
    {
        qLog(DocAPI) << "Query failed, error:" << qry.lastError()
                << "Query was:" << qry.lastQuery();
        return false;
    }
    else
        return true;
}

bool ContentLinkSql::hasRecords(QSqlQuery &qry)
{
    qry.seek(-1);
    if ( qry.next() )
        return true;
    else
        return false;
}

QString ContentLinkSql::buildOrderBy( const QStringList &sortList, QList< Parameter > *parameters, int *insertAt, QStringList *joins  )
{
    QString result;

    if(!sortList.isEmpty())
    {
        foreach( QString filter, sortList )
        {
            QString field=filter;
            if(field.endsWith(QLatin1String(" desc")))
                field.chop(5);

            if(!result.isEmpty())
                result+=QLatin1String(", ");
            if(field == QLatin1String("name"))
                filter.replace(QLatin1String("name"), QLatin1String("uiName COLLATE localeAwareCompare"));
            else if(field == QLatin1String("time"))
                filter.replace(QLatin1String("time"), QLatin1String("lastUpdated"));
            else if(field == QLatin1String("type"))
            {
                QString table = QString( "sort%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

                joins->append( QString( " left join mimeTypeLookup as %1" ).arg( table ) );

                filter.replace(QLatin1String( "type" ), QString( "%1.mimeType COLLATE localeAwareCompare" ).arg( table ) );
            }
            else if( field.startsWith( "synthetic/" ) )
            {
                QString table = QString( "sort%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

                QString groupParam = addParameter( filter.section( QLatin1Char('/'), 1, 1 ), parameters, insertAt );
                QString keyParam   = addParameter( filter.section( QLatin1Char('/'), 2, 2 ), parameters, insertAt );

                QString join = QString(
                        " left join contentProps as %1 on content.cid = %1.cid"
                        " and %1.grp = %2 and %1.name = %3" )
                        .arg( table )
                        .arg( groupParam )
                        .arg( keyParam );

                joins->append( join );

                filter.replace( field, QString( "%1.value COLLATE localeAwareCompare" ).arg( table ) );
            }
            result+=filter;
        }
        result = QLatin1String(" ORDER BY ") + result;
    }
    else
        result = QLatin1String(" ORDER BY content.uiName COLLATE localeAwareCompare");
    return result;
}

QString ContentLinkSql::buildFrom( const QContentFilter &filter, const QStringList &joins )
{
    QString from = QLatin1String( "content" );

    QSet< QContentFilter::FilterType > types = getAllFilterTypes( filter );

    if( types.contains( QContentFilter::Directory ) || types.contains( QContentFilter::Location ) )
        from += QLatin1String(
                " left join locationLookup as real on content.location = real.pKey"
                " left join locationLookup as link on content.linkLocation = link.pKey" );

    if( types.contains( QContentFilter::MimeType ) )
        from += QLatin1String( " left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey" );

    foreach( QString join, joins )
        from += join;

    return from;
}

QSet< QContentFilter::FilterType > ContentLinkSql::getAllFilterTypes( const QContentFilter &filter )
{
    QSet< QContentFilter::FilterType > types = filter.types().toSet();

    foreach( QContentFilter f, filter.subFilters() )
        types.unite( getAllFilterTypes( f ) );

    return types;
}

QSet< QString > ContentLinkSql::getAllSyntheticKeys( const QContentFilter &filter )
{
    QSet< QString > keys;

    QStringList syntheticFilters = filter.arguments( QContentFilter::Synthetic );

    foreach( QString syntheticFilter, syntheticFilters )
        keys.insert( syntheticFilter.section( QLatin1Char('/'), 0, 1 ) );

    foreach( QContentFilter f, filter.subFilters() )
        keys.unite( getAllSyntheticKeys( f ) );

    return keys;
}

QString ContentLinkSql::buildSynthetic( const QStringList &synthetic, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins )
{
    QString expression;
    QString c;

    foreach( QString filter, synthetic )
    {
        QString group = filter.section( QLatin1Char('/'), 0, 0 );
        QString key   = filter.section( QLatin1Char('/'), 1, 1 );
        QString value = filter.section( QLatin1Char('/'), 2 );

        QString table = QString( "prop%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

        QString groupParam = addParameter( group, parameters, insertAt );
        QString keyParam   = addParameter( key  , parameters, insertAt );

        QString join = QString( " left join contentProps as %1 on content.cid = %1.cid and %1.grp = %2 and %1.name = %3" )
                .arg( table  )
                .arg( groupParam )
                .arg( keyParam );

        joins->append( join );

        if( value.isEmpty() )
        {
            expression += c + QString( "(%1.value is null)" )
                    .arg( table );
        }
        else if( value.contains( QLatin1Char('*') ) )
        {
            value.replace( QLatin1Char('*'), QLatin1Char('%') );

            expression += c + QString( "(%1.value like %2 and %1.value not null)" )
                    .arg( table )
                    .arg( addParameter( value, parameters ) );
        }
        else
        {
            expression += c + QString( "(%1.value = %2 and %1.value not null)" )
                    .arg( table )
                    .arg( addParameter( value, parameters ) );
        }

        c = conjunct;
    }

    return expression;
}

QStringList ContentLinkSql::mimeFilterMatches( const QContentFilter &filter, const QString &subType )
{
    QContentFilter subFilter = filter;

    if( !subType.isEmpty() )
        subFilter &= QContentFilter( QContentFilter::MimeType, subType + QLatin1String( "/*" ) );

    QString queryString;
    QList< Parameter > parameters;

    if( subFilter.isValid() )
    {
        QString mimeQuery;

        if( getAllFilterTypes( subFilter ).contains( QContentFilter::MimeType ) )
        {
            mimeQuery = QLatin1String(
                    "select distinct mimeTypeLookup.mimeType "
                    "from %1 "
                    "where %2 "
                    "order by mimeTypeLookup.mimeType COLLATE localeAwareCompare" );
        }
        else
        {
            mimeQuery = QLatin1String(
                    "select distinct mimeTypeLookup.mimeType "
                    "from %1 left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
                    "where %2 "
                    "order by mimeTypeLookup.mimeType COLLATE localeAwareCompare" );
        }

        queryString = buildQuery( mimeQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct mimeType "
                "from mimeTypeLookup "
                "order by mimeType" );
    }

    QMap< QString, QString > filters;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
    {
        QSqlQuery query( QtopiaSql::database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
    {
                QString f = qvariant_cast< QString >( query.value(0) );

                if( !f.isEmpty() )
                    filters.insert( f, f );
        }
    }
    else
    {
            qLog(DocAPI) << "mimeFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( filters.values() );
}

QStringList ContentLinkSql::syntheticFilterGroups( const QContentFilter &filter )
{
    QString queryString;
    QList< Parameter > parameters;

    if( filter.isValid() )
    {
        static const QString groupQuery = QLatin1String(
                "select distinct contentProps.grp "
                "from %1 left join contentProps on content.cid = contentProps.cid "
                "where %2 "
                "order by contentProps.grp" );

        queryString = buildQuery( groupQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct grp "
                "from contentProps "
                "order by grp" );
    }

    QMap< QString, QString > groups;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
    {
        QSqlQuery query( QtopiaSql::database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
    {
                QString group = qvariant_cast< QString >( query.value(0) );

                if( !group.isEmpty() )
                    groups.insert( group, group );
            }
        }
        else
        {
            qLog(DocAPI) << "syntheticFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( groups.values() );
}

QStringList ContentLinkSql::syntheticFilterKeys( const QContentFilter &filter, const QString &group )
{
    QString queryString;
    QList< Parameter > parameters;

    if( filter.isValid() )
    {
        QString keyQuery = QLatin1String(
                "select distinct contentProps.name "
                "from %2 left join contentProps on content.cid = contentProps.cid and contentProps.grp = :group "
                "where %3 "
                "order by contentProps.name COLLATE localeAwareCompare" );

        parameters.append( Parameter( QLatin1String( "group" ), group ) );

        queryString = buildQuery( keyQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct name "
                "from contentProps "
                "where grp = :group" );
    }

    QMap< QString, QString > keys;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
    {
        QSqlQuery query( QtopiaSql::database( dbId ) );
        query.prepare( queryString );

        query.bindValue( QLatin1String( "group" ), group );

        bindParameters( &query, parameters );

        QtopiaSql::logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
    {
                QString key = qvariant_cast< QString >( query.value(0) );

                if( !key.isEmpty() )
                    keys.insert( key, key );
            }
    }
    else
        {
            qLog(DocAPI) << "syntheticFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( keys.values() );
}

QStringList ContentLinkSql::syntheticFilterMatches( const QContentFilter &filter, const QString &group, const QString &key )
{
    QString queryString;
    QList< Parameter > parameters;

    if( filter.isValid() )
    {
        static const QString valueQuery = QLatin1String(
                "select distinct contentProps.value "
                "from %1 left join contentProps on content.cid = contentProps.cid and contentProps.grp = :group and contentProps.name = :key "
                "where %2 "
                "order by contentProps.value COLLATE localeAwareCompare");

        parameters.append( Parameter( QLatin1String( "group" ), group ) );
        parameters.append( Parameter( QLatin1String( "key" ), key ) );

        queryString = buildQuery( valueQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct value "
                "from contentProps "
                "where grp = :group and name = :key" );
    }

    QMap< QString, QString > filters;

    QString filterBase = group + QLatin1Char('/') + key + ('/');

    foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
    {
        QSqlQuery query( QtopiaSql::database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
    {
                QString f = qvariant_cast< QString >( query.value(0) );

                filters.insert( f, filterBase + f );
            }
        }
        else
        {
            qLog(DocAPI) << "syntheticFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( filters.values() );
}

QStringList ContentLinkSql::categoryFilterMatches( const QContentFilter &filter, const QString &scope )
{
    QString queryString;
    QList< Parameter > parameters;
    QString scopeClause;

    if( !scope.isEmpty() )
    {
        scopeClause = QLatin1String( "categories.categoryscope = :scope" );

        parameters.append( Parameter( QLatin1String( "scope" ), scope ) );
    }
    else
        scopeClause = QLatin1String( "categories.categoryscope is null" );

    if( filter.isValid() )
    {
        queryString = QString(
            "select distinct category.categoryid "
            "from %2 inner join mapCategoryToContent as category on content.cid = category.cid "
                    "inner join categories on category.categoryid = categories.categoryid and %1 "
             "where %3 "
            "order by categories.categorytext COLLATE localeAwareCompare")
            .arg( scopeClause );

        queryString = buildQuery( queryString, filter, &parameters );
    }
    else
    {
        queryString = QString(
                "select distinct categoryid "
                "from categories "
                "where %1" )
                .arg( scopeClause );
    }

    QMap< QString, QString > filters;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::databaseIds() )
    {
        QSqlQuery query( QtopiaSql::database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
            {
                QString f = qvariant_cast< QString >( query.value(0) );

                filters.insert( f, f );
            }
        }
    else
        {
            qLog(DocAPI) << "categoryFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( filters.values() );
}

QString ContentLinkSql::buildQuery( const QString &queryTemplate, const QContentFilter &filter, QList< Parameter > *parameters )
{
    if( !filter.isValid() )
        return QString();

    int insertAt = 0;
    QStringList joins;

    QString where = buildWhereClause( filter, parameters, &insertAt, &joins );
    QString from = buildFrom( filter, joins );

    return queryTemplate.arg( from ).arg( where );
}

QString ContentLinkSql::buildQuery( const QString &queryTemplate, const QContentFilter &filter, const QStringList &sortOrder,  QList< Parameter > *parameters )
{
    if( !filter.isValid() )
        return QString();

    int insertAt = 0;
    QStringList joins;

    QString where = buildWhereClause( filter, parameters, &insertAt, &joins );
    QString order = buildOrderBy( sortOrder, parameters, &insertAt, &joins );
    QString from = buildFrom( filter, joins );

    return queryTemplate.arg( from ).arg( where );
}

void ContentLinkSql::bindParameters( QSqlQuery *query, const QList< Parameter > &parameters )
{
    foreach( const Parameter parameter, parameters )
        query->bindValue( parameter.first, parameter.second );
}

QString ContentLinkSql::addParameter( const QVariant &parameter, QList< Parameter > *parameters )
{
    QString id = QString( ":param%1" ).arg( parameters->count(), 3, 10, QLatin1Char( '0' ) );

    parameters->append( Parameter( id, parameter ) );

    return id;
}

QString ContentLinkSql::addParameter( const QVariant &parameter, QList< Parameter > *parameters, int *insertAt )
{
    QString id = QString( ":param%1" ).arg( parameters->count(), 3, 10, QLatin1Char( '0' ) );

    parameters->insert( (*insertAt)++, Parameter( id, parameter ) );

    return id;
}

QHash<QString, QVariant> ContentLinkSql::linkByPath( const QString &path, bool isLink )
{
    static const QString commonString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
                   "real.location real_location, icon, linkFile, link.location link_location, mimeTypes, lastUpdated "
            "FROM content left join locationLookup real on content.location = real.pKey "
                         "left join locationLookup link on content.linkLocation = link.pKey "
                         "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey " );

    static const QString realString = QLatin1String( "WHERE path = :path and real.location = :location" );
    static const QString linkString = QLatin1String( "WHERE linkFile = :path and link.location = :location" );

    errText = QString::null;
    isError = false;

    QtopiaDatabaseId dbId = QtopiaSql::databaseIdForPath( path );

    QSqlQuery qLBI_qry( QtopiaSql::database( dbId ) );
    qLBI_qry.prepare( commonString + (isLink ? linkString : realString) );

    // at present, no need for a transaction
    qLBI_qry.bindValue( QLatin1String( "path"     ), path.section( QLatin1Char('/'), -1     ) );
    qLBI_qry.bindValue( QLatin1String( "location" ), path.section( QLatin1Char('/'),  0, -2 ) );

    QtopiaSql::logQuery( qLBI_qry );

    if( ! qLBI_qry.exec() )
    {
        qLog(DocAPI) << "ContentLinkSql::retrieveById: Query failed!! qry=" << qLBI_qry.lastQuery()
                << "\nerror=" << qLBI_qry.lastError();
        isError = true;
        errText += qLBI_qry.lastError().text();
        return QHash<QString, QVariant>();
        }
    if ( !qLBI_qry.first() )
    {
        isError = true;
        errText += QString( "Get link by path %1 returned %3 records!" )
                .arg( path ).arg(countRecords( qLBI_qry ));
        return QHash<QString, QVariant>();
    }

    return linkFromRecord( qLBI_qry.record(), dbId );
}

/*!
  Given an application \a name return the QContentId for the content item representing
  that application.

  If there is no matching binary, return QContent::Invalid
*/
QContentId ContentLinkSql::idByExecutable( const QString &name )
{
    QContentSet set(QContentFilter::Role, QString("%1").arg(QContent::Application));
    QContent c = set.findExecutable(name);
    QContentId id = c.id();
    return id;
}

QHash<QString, QVariant> ContentLinkSql::linkById( QContentId cId )
{
    static const QString queryString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
                   "real.location real_location, icon, linkFile, link.location link_location, mimeTypes, lastUpdated "
            "FROM content left join locationLookup real on content.location = real.pKey "
                         "left join locationLookup link on content.linkLocation = link.pKey "
                         "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
            "WHERE cid=:cid" );

    errText = QString::null;
    isError = false;

    if( cId == QContent::InvalidId )
        return QHash<QString, QVariant>();

    QSqlQuery qLBI_qry( QtopiaSql::database( cId.first ) );
    qLBI_qry.prepare(queryString);

    // at present, no need for a transaction
    qLBI_qry.bindValue( "cid", cId.second );

    QtopiaSql::logQuery( qLBI_qry );

    if ( ! qLBI_qry.exec() )
    {
        qLog(DocAPI) << "ContentLinkSql::retrieveById: Query failed!! qry=" << qLBI_qry.lastQuery()
                << "\nerror=" << qLBI_qry.lastError();
        isError = true;
        errText += qLBI_qry.lastError().text();
        return QHash<QString, QVariant>();
    }
    if ( !qLBI_qry.first() )
    {
        isError = true;
        errText += QString( "Get link by Id (%1,%2) returned %3 records!" )
                .arg( cId.first ).arg( cId.second ).arg(countRecords( qLBI_qry ));
        return QHash<QString, QVariant>();
    }

    return linkFromRecord( qLBI_qry.record(), cId.first );
}

QList< QHash<QString, QVariant> > ContentLinkSql::linksById( const QContentIdList &cIds )
{
    static const QString queryString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
                   "real.location real_location, icon, linkFile, link.location link_location, mimeTypes, lastUpdated "
            "FROM content left join locationLookup real on content.location = real.pKey "
                         "left join locationLookup link on content.linkLocation = link.pKey "
                         "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
            "WHERE cid IN (%1)" );

    QMultiHash< QtopiaDatabaseId, QString > idLists;
    QList< QHash<QString, QVariant> > resultList;

    errText = QString::null;
    isError = false;

    foreach( QContentId cid, cIds )
        idLists.insert( cid.first, QString::number( cid.second ) );

    foreach(QtopiaDatabaseId dbid, idLists.keys().toSet())
    {
        QStringList cIds = idLists.values( dbid );

        QSqlQuery qLBI_qry( QtopiaSql::database(dbid) );
        qLBI_qry.prepare(queryString.arg( cIds.join( QLatin1String( "," ) ) ));

        QtopiaSql::logQuery( qLBI_qry );

        if ( !qLBI_qry.exec() )
        {
            qLog(DocAPI) << "ContentLinkSql::retrieveByIds: Query failed!! qry=" << qLBI_qry.lastQuery()
                    << "\nerror=" << qLBI_qry.lastError();
            isError = true;
            errText += qLBI_qry.lastError().text();
            continue;
        }

        while( qLBI_qry.next() )
            resultList.append( linkFromRecord( qLBI_qry.record(), dbid ) );
    }
    return resultList;
}

QHash<QString, QVariant> ContentLinkSql::linkFromRecord( const QSqlRecord &record, QtopiaDatabaseId databaseId ) const
{
    QHash<QString, QVariant> link;

    link[ QLatin1String("database") ] = databaseId;

    for( int f = 0; f < record.count(); f++ )
    {
        QString field = record.fieldName(f);
        int dot = field.lastIndexOf('.');
        if (dot > 0)
            field = field.mid(dot+1);

        link[ field ] = record.value(f);
    }
    if(link.contains(QLatin1String("path")))
    {
        static QString real_location("real_location");
        if(link.contains(real_location) && !link[real_location].toString().isEmpty())
        {
            link[QLatin1String("path")] = link[real_location].toString() + QLatin1Char('/') + link[QLatin1String("path")].toString();
            link.remove(real_location);
        }
        }
    if(link.contains(QLatin1String("linkFile")))
    {
        static QString link_location("link_location");
        if(link.contains(link_location) && !link[link_location].toString().isEmpty())
        {
            link[QLatin1String("linkFile")] = link[link_location].toString() + QLatin1Char('/') + link[QLatin1String("linkFile")].toString();
            link.remove(link_location);
        }
    }

    return link;
}
