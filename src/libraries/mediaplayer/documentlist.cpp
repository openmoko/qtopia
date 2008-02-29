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
#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/storage.h>
#include <qcopchannel_qws.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtimer.h>
#include "documentlist.h"


/*!
  \class AppDocumentListPrivate documentlist.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    It is used for scanning for file interatively.
*/


static const int MAX_SEARCH_DEPTH = 10;


class AppDocumentListPrivate {
public:
    AppDocumentListPrivate();
    ~AppDocumentListPrivate();

    void initialize( const QString &mimefilter );
    QString mimeFilter;

    QFileInfo *nextFile();
    const DocLnk *iterate(bool &finished);
    bool store( DocLnk* dl );
    int estimatedPercentScanned();
    int iterationI;
    int iterationCount;

    DocLnkSet dls;
    QDict<void> reference;
    QDictIterator<void> *dit;
    enum { Find, RemoveKnownFiles, MakeUnknownFiles, Done } state;

    QValueList<QRegExp> mimeFilters;

    QStringList docPaths;
    unsigned int docPathsSearched;

    int searchDepth;
    QDir *listDirs[MAX_SEARCH_DEPTH];
    const QFileInfoList *lists[MAX_SEARCH_DEPTH];
    unsigned int listPositions[MAX_SEARCH_DEPTH];

    QCopChannel *systemChannel;
    StorageInfo *storage;

    int tid;
};


AppDocumentListPrivate::AppDocumentListPrivate() 
{
    for ( int i = 0; i < MAX_SEARCH_DEPTH; i++ ) {
	listDirs[i] = 0;
	lists[i] = 0;
	listPositions[i] = 0;
    }
    tid = 0;
}


void AppDocumentListPrivate::initialize( const QString &mimefilter ) 
{
    mimeFilter = mimefilter;

    // Reset
    dls.clear();
    docPaths.clear();
    reference.clear();
    mimeFilters.clear();

    // Break up filter in to regexps
    QStringList subFilter = QStringList::split(";", mimefilter);
    for( QStringList::Iterator it = subFilter.begin(); it != subFilter.end(); ++ it )
	mimeFilters.append( QRegExp(*it, FALSE, TRUE) );
    QDir docDir( QPEApplication::documentDir() );
    if ( docDir.exists() )
	docPaths += QPEApplication::documentDir();
    int i = 1;
    const QList<FileSystem> &fs = storage->fileSystems();
    QListIterator<FileSystem> it( fs );
    for ( ; it.current(); ++it ) 
	if ( (*it)->isRemovable() ) {
	    docPaths += (*it)->path();
	    i++;
	}

    for ( int i = 0; i < MAX_SEARCH_DEPTH; i++ ) {
        if ( listDirs[i] ) {
            delete listDirs[i];
            listDirs[i] = 0;
        }
        lists[i] = 0;
        listPositions[i] = 0;
    }

    docPathsSearched = 0;
    searchDepth = -1;
    state = Find;
    dit = 0;
}


AppDocumentListPrivate::~AppDocumentListPrivate()
{
    for ( int i = 0; i < MAX_SEARCH_DEPTH; i++ ) 
	if ( listDirs[i] ) 
	    delete listDirs[i];
    delete dit;
}


int AppDocumentListPrivate::estimatedPercentScanned()
{
    if ( state == Done )
        return 100;

    if ( state == MakeUnknownFiles )
        return 75 + ((iterationCount) ? (25*iterationI)/iterationCount : 0);

    double overallProgress = 0.0;
    double levelWeight = 75.0;

    int topCount = docPaths.count();
    if ( topCount > 1 ) {
	levelWeight = levelWeight / topCount;
	overallProgress += (docPathsSearched - 1) * levelWeight;
    }

    for ( int d = 0; d <= searchDepth; d++ ) {
	if ( listDirs[d] ) {
	    int items = lists[d]->count();
	    if ( items > 1 ) {
		levelWeight = levelWeight / items;
		// Take in to account "." and ".."
		overallProgress += (listPositions[d] - 3) * levelWeight;
	    }
	} else {
	    break;
	}
    }

    // qDebug( "overallProgress: %f", overallProgress );
    return (int)overallProgress;
}


QFileInfo *AppDocumentListPrivate::nextFile()
{
    while ( TRUE ) {
	while ( searchDepth < 0 ) {
	    // go to next base path
	    if ( docPathsSearched >= docPaths.count() ) {
		// end of base paths
		return NULL;
	    } else {
		QDir dir( docPaths[docPathsSearched] );
//qDebug("now using base path: %s", docPaths[docPathsSearched].latin1() );
		docPathsSearched++;
		if ( !dir.exists( ".Qtopia-ignore" ) ) {
		    listDirs[0] = new QDir( dir );
		    lists[0] = listDirs[0]->entryInfoList();
		    listPositions[0] = 0;
		    searchDepth = 0;
		}
	    }
	}

	const QFileInfoList *fil = lists[searchDepth];
	QFileInfoList *fl = (QFileInfoList *)fil;
	unsigned int pos = listPositions[searchDepth];

	if ( pos >= fl->count() ) {
	    // go up a depth
	    delete listDirs[searchDepth];
	    listDirs[searchDepth] = 0;
	    lists[searchDepth] = NULL;
	    listPositions[searchDepth] = 0;
	    searchDepth--;
	} else {
	    const QFileInfo *fi = fl->at(pos);
	    listPositions[searchDepth]++;

	    QString bn = fi->fileName();
	    if ( bn[0] != '.' ) {
		if ( fi->isDir()  ) {
		    if ( bn != "CVS" && bn != "Qtopia" && bn != "QtPalmtop" ) { 
			// go down a depth
			QDir dir( fi->filePath() );
//qDebug("now going in to path: %s", bn.latin1() );
			if ( !dir.exists( ".Qtopia-ignore" ) ) {
			    if ( searchDepth < MAX_SEARCH_DEPTH - 1 ) {
				searchDepth++;
				listDirs[searchDepth] = new QDir( dir );
				lists[searchDepth] = listDirs[searchDepth]->entryInfoList();
				listPositions[searchDepth] = 0;
			    }
			}
		    }
		} else {
		    return fl->at(pos);
		}
	    }
	}
    }

    return NULL;
}


bool AppDocumentListPrivate::store( DocLnk* dl )
{
    bool mtch = FALSE;
    if ( mimeFilters.count() == 0 ) {
	mtch = TRUE;
    } else {
	for ( QValueList<QRegExp>::ConstIterator it = mimeFilters.begin();
	    !mtch && it != mimeFilters.end(); ++ it )
	{
	    if ( (*it).match(dl->type()) >= 0 )
		mtch = TRUE;
	}
    }
    if ( mtch ) {
	dls.add( dl ); // store
	return TRUE;
    }

    // don't store - delete
    delete dl;
    return FALSE;
}


#define MAGIC_NUMBER    ((void*)2)

const DocLnk *AppDocumentListPrivate::iterate(bool &finished)
{
    int filesScanned = 0;
    finished = false;
    if ( state == Find ) {
	//qDebug("state Find");
	QFileInfo *fi;
	while ( (fi = nextFile()) ) {
	    if ( fi->extension(FALSE) == "desktop" ) { // No tr
		DocLnk* dl = new DocLnk( fi->filePath() );
		if ( store(dl) )
		    return dl;
	    } else {
		if ( !reference.find( fi->filePath() ) ) {
		    reference.insert( fi->filePath(), MAGIC_NUMBER );
		    filesScanned++;
		    if ( filesScanned > 10 )
			return 0;
		}
	    }
	}
	state = RemoveKnownFiles;
    }

    if ( state == RemoveKnownFiles ) {
	//qDebug("state RemoveKnownFiles");
	const QList<DocLnk> &list = dls.children();
	for ( QListIterator<DocLnk> it( list ); it.current(); ++it ) {
	    reference.remove( (*it)->file() );
	    // ### check if this needs a delete
	}
	dit = new QDictIterator<void>(reference);
	state = MakeUnknownFiles;

        iterationI = 0;
        iterationCount = dit->count();
    }

    if ( state == MakeUnknownFiles ) {
	//qDebug("state MakeUnknownFiles");
	for (void* c; (c=dit->current()); ++(*dit) ) {
	    if ( c == MAGIC_NUMBER ) {
		DocLnk* dl = new DocLnk;
		QFileInfo fi( dit->currentKey() );
		dl->setFile( fi.filePath() );
		QString tmp = fi.fileName();
		int pos = tmp.findRev( '.' );
		dl->setName( (pos == -1) ? tmp : tmp.left( pos ) );
		if ( store(dl) ) {
		    ++*dit;
                    iterationI++;
		    return dl;
		}
	    }
            iterationI++;
	}

	delete dit;
	dit = 0;
	state = Done;
    }

    //qDebug("state Done");
    finished = true;
    return 0;
}



AppDocumentList::AppDocumentList( const QString &mimefilter, QObject *parent, const char *name )
 : QObject( parent, name )
{
    d = new AppDocumentListPrivate();

    d->storage = new StorageInfo( this );

    d->initialize( mimefilter );

    connect( d->storage, SIGNAL( disksChanged() ), SLOT( storageChanged() ) );

    d->systemChannel = new QCopChannel( "QPE/System", this );
    connect( d->systemChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	 this, SLOT(systemMessage(const QCString&,const QByteArray&)) );
}


void AppDocumentList::add( const DocLnk& doc ) 
{
    if ( QFile::exists( doc.file() ) )
	emit added( doc );
}


void AppDocumentList::start() 
{
    emit percentDone(0);
    resume();
}


void AppDocumentList::pause() 
{
    killTimer( d->tid );
    d->tid = 0;
}


void AppDocumentList::resume() 
{
    if ( d->tid == 0 )
	d->tid = startTimer(0);
}


void AppDocumentList::resend()
{
    // Re-emits all the added items to the list (firstly letting everyone know to
    // clear what they have as it is being sent again)
    pause();
    emit allRemoved();
    QTimer::singleShot( 5, this, SLOT( resendWorker() ) );
}


void AppDocumentList::resendWorker()
{
    const QList<DocLnk> &list = d->dls.children();
    for ( QListIterator<DocLnk> it( list ); it.current(); ++it ) 
	add( *(*it) );
    resume();
}


void AppDocumentList::rescan()
{
    pause();
    emit percentDone(0);
    emit allRemoved();
    d->initialize( d->mimeFilter );
    resume();
}


AppDocumentList::~AppDocumentList( )
{
    delete d->systemChannel;
    delete d;
}


void AppDocumentList::systemMessage( const QCString &msg, const QByteArray &data )
{
    if ( msg == "linkChanged(QString)" ) {
	QDataStream stream( data, IO_ReadOnly );
        QString arg;
        stream >> arg;
	DocLnk doclnk( arg );
	qDebug( "linkchanged( %s )", arg.latin1() );
	pause();
	const QList<DocLnk> &list = d->dls.children();
	QListIterator<DocLnk> it( list );
	while ( it.current() ) {
	    DocLnk *doc = it.current();
	    ++it;
	    if ( ( doc->linkFileKnown() && doc->linkFile() == arg )
		|| ( doc->fileKnown() && doc->file() == arg )
		|| ( doc->file() == doclnk.file() ) ) {
		qDebug( "found old link" );
		DocLnk* dl = new DocLnk( arg );
		// add new one if it exists and matches the mimetype
		if ( d->store( dl ) ) {
		    // Existing link has been changed, send old link ref and a ref
		    // to the new link
		    qDebug( "change case" );
		    emit changed( *doc, *dl );
		} else {
		    // Link has been removed or doesn't match the mimetypes any more
		    // so we aren't interested in it, so take it away from the list
		    qDebug( "removal case" );
		    emit removed( *doc );
		}
		d->dls.remove( doc ); // remove old link from docLnkSet
		delete doc;
		resume();
		return;
	    }
	}
	// Didn't find existing link, must be new
	DocLnk* dl = new DocLnk( arg );
	if ( d->store( dl ) ) {
	    // Add if it's a link we are interested in
	    qDebug( "add case" );
	    add( *dl );
	}
	resume();
    }
}


void AppDocumentList::storageChanged()
{
    // ### Optimization opportunity
    // Could be a bit more intelligent and somehow work out which
    // mtab entry has changed and then only scan that and add and remove
    // links appropriately.
    rescan();
}


void AppDocumentList::timerEvent( QTimerEvent *te )
{
    if ( te->timerId() == d->tid ) {
	// Do 10 at a time
	for (int i = 0; i < 3; i++ ) {
	    bool finished;
	    const DocLnk *lnk = d->iterate(finished);
	    if ( lnk ) {
		add( *lnk );
	    } else if ( finished ) {
		// stop when done
		pause();
		emit doneForNow();
		break;
	    }
	}
        emit percentDone(d->estimatedPercentScanned());
    }
}


