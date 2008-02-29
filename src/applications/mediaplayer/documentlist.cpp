/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/storage.h>
#include <qpainter.h>
#include <qimage.h>
#include <qcopchannel_qws.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlistview.h>
#include <qlist.h>
#include <qtimer.h>
#include <qpixmap.h>
#include "playlistselection.h"
#include "documentlist.h"


static const int MAX_SEARCH_DEPTH = 10;


class DocumentListPrivate {
public:
    DocumentListPrivate();
    ~DocumentListPrivate();

    void initialize( const QString &mimefilter );
    QString mimeFilter;

    QFileInfo *nextFile();
    const DocLnk *iterate();
    bool store( DocLnk* dl );

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


DocumentListPrivate::DocumentListPrivate() 
{
}


void DocumentListPrivate::initialize( const QString &mimefilter ) 
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
    docPathsSearched = 0;
    searchDepth = -1;
    state = Find;
    dit = 0;
}


DocumentListPrivate::~DocumentListPrivate()
{
    delete dit;
}


QFileInfo *DocumentListPrivate::nextFile( )
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
			    if ( searchDepth < MAX_SEARCH_DEPTH ) {
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


bool DocumentListPrivate::store( DocLnk* dl )
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


const DocLnk *DocumentListPrivate::iterate()
{
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
		    reference.insert( fi->filePath(), (void*)2 );
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
	}
	dit = new QDictIterator<void>(reference);
	state = MakeUnknownFiles;
    }

    if ( state == MakeUnknownFiles ) {
	//qDebug("state MakeUnknownFiles");
	for (void* c; (c=dit->current()); ++(*dit) ) {
	    if ( c == (void*)2 ) {
		DocLnk* dl = new DocLnk;
		QFileInfo fi( dit->currentKey() );
		dl->setFile( fi.filePath() );
		dl->setName( fi.baseName() );
		if ( store(dl) ) {
		    ++*dit;
		    return dl;
		}
	    }
	}

	delete dit;
	dit = 0;
	state = Done;
    }

    //qDebug("state Done");
    return NULL;
}



DocumentList::DocumentList( const QString &mimefilter, QObject *parent, const char *name )
 : QObject( parent, name )
{
    d = new DocumentListPrivate();

    d->storage = new StorageInfo( this );

    d->initialize( mimefilter );

    d->tid = 0;

    connect( d->storage, SIGNAL( disksChanged() ), SLOT( storageChanged() ) );

    d->systemChannel = new QCopChannel( "QPE/System", this );
    connect( d->systemChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	 this, SLOT(systemMessage( const QCString &, const QByteArray &)) );
}


void DocumentList::add( const DocLnk& doc ) 
{
    if ( QFile::exists( doc.file() ) )
	emit added( doc );
}


void DocumentList::start() 
{
    resume();
}


void DocumentList::pause() 
{
    killTimer( d->tid );
    d->tid = 0;
}


void DocumentList::resume() 
{
    if ( d->tid == 0 )
	d->tid = startTimer( 1 );
}


void DocumentList::resend()
{
    // Re-emits all the added items to the list (firstly letting everyone know to
    // clear what they have as it is being sent again)
    pause();
    emit allRemoved();
    QTimer::singleShot( 5, this, SLOT( resendWorker() ) );
}


void DocumentList::resendWorker()
{
    const QList<DocLnk> &list = d->dls.children();
    for ( QListIterator<DocLnk> it( list ); it.current(); ++it ) 
	add( *(*it) );
    resume();
}


void DocumentList::rescan()
{
    pause();
    emit allRemoved();
    d->initialize( d->mimeFilter );
    resume();
}


DocumentList::~DocumentList( )
{
    delete d->systemChannel;
    delete d;
}


void DocumentList::systemMessage( const QCString &msg, const QByteArray &data )
{
    if ( msg == "linkChanged(QString)" ) {
	QDataStream stream( data, IO_ReadOnly );
        QString arg;
        stream >> arg;
	qDebug( "linkchanged( %s )", arg.latin1() );
	pause();
	const QList<DocLnk> &list = d->dls.children();
	for ( QListIterator<DocLnk> it( list ); it.current(); ++it ) {
	    if ( (*it)->linkFile() == arg ) {
		qDebug( "found old link" );
		DocLnk* dl = new DocLnk( arg );
		// add new one if it exists and matches the mimetype
		if ( d->store( dl ) ) {
		    // Existing link has been changed, send old link ref and a ref
		    // to the new link
		    qDebug( "change case" );
		    emit changed( *(*it), *dl );
		} else {
		    // Link has been removed or doesn't match the mimetypes any more
		    // so we aren't interested in it, so take it away from the list
		    qDebug( "removal case" );
		    emit removed( *(*it) );
		}
		d->dls.remove( (*it) ); // remove old link from docLnkSet
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


void DocumentList::storageChanged()
{
    // ### Optimization opportunity
    // Could be a bit more intelligent and somehow work out which
    // mtab entry has changed and then only scan that and add and remove
    // links appropriately.
    rescan();
}


void DocumentList::timerEvent( QTimerEvent *te )
{
    if ( te->timerId() == d->tid ) {
	// Do 10 at a time
	for (int i = 0; i < 10; i++ ) {
	    const DocLnk *lnk = d->iterate();
	    if ( lnk ) {
		add( *lnk );
	    } else {
		// stop when done
		pause();
		emit doneForNow();
		break;
	    }
	}
    }
}


