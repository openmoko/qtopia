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
#include "serverinterface.h"
#include <qtopia/mimetype.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/categories.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/storage.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qtimer.h>
#include "documentlist.h"
#include "serverapp.h"


AppLnkSet *DocumentList::appLnkSet = 0;

static const int MAX_SEARCH_DEPTH = 10;


class DocumentListPrivate {
public:
    DocumentListPrivate( ServerInterface *gui );
    ~DocumentListPrivate();

    void initialize();

    const QString nextFile();
    const DocLnk *iterate(bool &finished);
    bool store( DocLnk* dl );
    void estimatedPercentScanned();

    DocLnkSet dls;
    QDict<void> reference;
    QDictIterator<void> *dit;
    enum { Find, RemoveKnownFiles, MakeUnknownFiles, Done } state;

    QStringList docPaths;
    unsigned int docPathsSearched;

    int searchDepth;
    QDir *listDirs[MAX_SEARCH_DEPTH];
    const QFileInfoList *lists[MAX_SEARCH_DEPTH];
    unsigned int listPositions[MAX_SEARCH_DEPTH];

    StorageInfo *storage;

    int tid;

    ServerInterface *serverGui;

    bool needToSendAllDocLinks;
    bool sendAppLnks;
    bool sendDocLnks;
    bool scanDocs;

    QString currentType;
    QStringList loadedAppTypes;
    QStringList curTypes;
    bool hasDocBeenLoaded;
};


void DocumentList::reloadAppLnks()
{
    if ( d->sendAppLnks && d->serverGui ) {
	d->serverGui->applicationScanningProgress( 0 );
	d->serverGui->allApplicationsRemoved();
    }

    delete appLnkSet;
    appLnkSet = new AppLnkSet( MimeType::appsFolderName() );
    d->loadedAppTypes.clear();

    QStringList types;
    if ( d->sendAppLnks && d->serverGui ) {
	static QStringList prevTypeList;
	types = appLnkSet->types();
	d->curTypes = types;
	types.sort();
	for ( QStringList::Iterator ittypes=types.begin(); ittypes!=types.end(); ++ittypes) {
	    if ( !(*ittypes).isEmpty() ) {
		if ( !prevTypeList.contains(*ittypes) ) {
		    QString name = appLnkSet->typeName(*ittypes);
		    QPixmap pm = appLnkSet->typePixmap(*ittypes);
		    QPixmap bgPm = appLnkSet->typeBigPixmap(*ittypes);

		    if (pm.isNull()) {
			QImage img( Resource::loadImage( "UnknownDocument" ) );
			pm = img.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() );
			bgPm = img.smoothScale( AppLnk::bigIconSize(), AppLnk::bigIconSize() );
		    }

		    //qDebug("adding type %s", (*ittypes).latin1());

		    // ### our current launcher expects docs tab to be last
		    d->serverGui->typeAdded( *ittypes, name.isNull() ? (*ittypes) : name, pm, bgPm );
		}
		prevTypeList.remove(*ittypes);
	    }
	}
	for ( QStringList::Iterator ittypes=prevTypeList.begin(); ittypes!=prevTypeList.end(); ++ittypes) {
	    //qDebug("removing type %s", (*ittypes).latin1());
	    if( (*ittypes) == d->currentType )
		d->currentType = QString::null;
	    d->serverGui->typeRemoved(*ittypes);
	}
	prevTypeList = types;
    }

#ifdef QTOPIA_PHONE
    types.clear(); // always clear types, reload only current if set
    if( d->currentType != QString::null )
	types.append( d->currentType );
#endif

    loadAppsForTypes( types );

    if ( d->sendAppLnks && d->serverGui ) 
	d->serverGui->applicationScanningProgress( 100 );
}

bool DocumentList::requirementsSatisfied( AppLnk *l ) const
{
    if ( d->sendAppLnks && d->serverGui ) {
	// Only add applications if all of their requirements are satisifed
	bool reqsSatisfied = TRUE;
	QString req = l->property("Requires");
	if (!req.isEmpty()) {
	    QStringList appReqs = QStringList::split(',', req);
	    QStringList::ConstIterator it;
	    for (it = appReqs.begin(); it != appReqs.end(); ++it) {
		if (!ServerApplication::haveFeature(*it)) {
		    reqsSatisfied = FALSE;
		    break;
		}
	    }
	}
	return reqsSatisfied;
    }
    return FALSE;
}

void DocumentList::loadAppsForTypes( const QStringList &t, bool force )
{
    QStringList types = t;
    QStringList::Iterator it = types.begin();
    while( it != types.end() ) {
	if( d->loadedAppTypes.contains( *it ) && !force ) {
	    it = types.remove( it ); // apps already loaded for this type, remove it
	} else {
	    d->loadedAppTypes.append( *it ); // will be loaded after this function call
	    ++it;
	}
    }
    if( !types.count() )
	return;

    QListIterator<AppLnk> itapp( appLnkSet->children() );
    AppLnk* l;
    while ( (l=itapp.current()) ) {
	++itapp;
	if( types.contains( l->type() ) ) {
	    if( requirementsSatisfied( l ) ) {
		d->serverGui->applicationAdded( l->type(), *l );
	    }
	} /*else {
	    qDebug("not loading applnk of type %s");
	}*/
    }
}

void DocumentList::ensureLoaded( const QString &type )
{
    //qDebug("DocumentList::esnureLoaded( %s )", type.latin1());
    loadAppsForTypes( type, FALSE );
    QStringList curLoadedAppTypes  = d->loadedAppTypes;
    if( type == "Documents" && !d->hasDocBeenLoaded ) {
	for( QStringList::ConstIterator it = d->curTypes.begin() ; it != d->curTypes.end() ; ++it ) {
	    // Load any other type views that have not yet been loaded
	    // The Documents view is about to be reloaded and needs the loaded applications for file associations
	    // Maybe we should just hard-code-reload the applications view for speed, but that seems wrong.
	    // if !curLoadedAppTypes.contains( "Applications" ) loadAppsForTypes( QStringList("Applications") )
	    if( !curLoadedAppTypes.contains( *it ) ) {
		loadAppsForTypes( *it );
		curLoadedAppTypes.append( *it );
	    }
	}
	reloadDocLnks();
    }
}

void DocumentList::reloadDocLnks()
{
    if ( !d->scanDocs )
	return;

    if ( d->sendDocLnks && d->serverGui ) {
	d->serverGui->documentScanningProgress( 0 );
	d->serverGui->allDocumentsRemoved();
    }

    rescan();
}

void DocumentList::linkChanged( QString arg )
{
    //qDebug( "linkchanged( %s )", arg.latin1() );

    if ( arg.isNull() || Global::isAppLnkFileName( arg ) ) {
	reloadAppLnks();
    } else {

	const QList<DocLnk> &list = d->dls.children();
	QListIterator<DocLnk> it( list );
	while ( it.current() ) {
	    DocLnk *doc = it.current();
	    ++it;
	    if ( ( doc->linkFileKnown() && doc->linkFile() == arg ) 
		|| ( doc->fileKnown() && doc->file() == arg ) ) {
		//qDebug( "found old link" );
		DocLnk* dl = new DocLnk( arg );
		// add new one if it exists and matches the mimetype
		if ( d->store( dl ) ) {
		    // Existing link has been changed, send old link ref and a ref
		    // to the new link
		    //qDebug( "change case" );
		    if ( d->serverGui )
			d->serverGui->documentChanged( *doc, *dl );
		    sendDocLnk( doc );

		} else {
		    // Link has been removed or doesn't match the mimetypes any more
		    // so we aren't interested in it, so take it away from the list
		    //qDebug( "removal case" );
		    if ( d->serverGui )
			d->serverGui->documentRemoved( *doc );
		    sendDocLnk( doc );

		}
		d->dls.remove( doc ); // remove old link from docLnkSet
		delete doc;
		return;
	    }
	}
	// Didn't find existing link, must be new
	DocLnk* dl = new DocLnk( arg );
	if ( d->store( dl ) ) {
	    // Add if it's a link we are interested in
	    //qDebug( "add case" );
	    add( *dl );
	    sendDocLnk( dl );
	}

    }
}

void DocumentList::restoreDone()
{
    reloadAppLnks();
    if( d->hasDocBeenLoaded )
	reloadDocLnks();
}

void DocumentList::storageChanged()
{
    // ### can implement better
    reloadAppLnks();
    if( d->hasDocBeenLoaded )
	reloadDocLnks();
    // ### Optimization opportunity
    // Could be a bit more intelligent and somehow work out which
    // mtab entry has changed and then only scan that and add and remove
    // links appropriately.
//    rescan();
}

void DocumentList::sendAllDocLinks()
{
    if ( d->tid != 0 ) {
	// We are in the middle of scanning, set a flag so
	// we do this when we finish our scanning
	d->needToSendAllDocLinks = true;
	return;
    }

    // send over our mime.types files so Qtopia Desktop can map extensions to types properly
    {
        QString files[] = { "/etc/mime.types", QPEApplication::qpeDir()+"etc/mime.types", QString::null };
        QString file;
        for ( int i = 0; (file = files[i]) != QString::null; i++ ) {
            QFile f(file);
            if ( f.open(IO_ReadOnly) ) {
                QByteArray data = f.readAll();
#ifndef QT_NO_COP
                QCopEnvelope e( "QPE/Desktop", "mimeTypes(QByteArray)" );
                e << data;
#endif
            }
        }
    }

    // send over all the icons that are needed for the documents
    QStringList processedIcons;
    AppLnkSet a( MimeType::appsFolderName() );
    for ( QListIterator<AppLnk> it( a.children() ); it.current(); ++it ) {
	const AppLnk &lnk = *it.current();
	QStringList icons = lnk.mimeTypeIcons();
	QStringList types = lnk.mimeTypes();
	QString icon;
	QString type;
	QString exe = lnk.exec();

	// Mime Type icons
	for (QStringList::ConstIterator t=types.begin(),i=icons.begin(); t!=types.end() && i!=icons.end(); ++i,++t) {
	    icon = exe + '/' + *i;
	    type = *t;
	    if ( !processedIcons.contains( icon ) ) {
		processedIcons.append( icon );
		sendIcon( exe, icon, type );
	    }
	}

	// Normal "app" icon
	if ( icon.isNull() ) {
	    icon = lnk.icon();
	    type = *types.begin();
	    // Only send if there is a mime type for this app
	    if ( type != QString::null && !processedIcons.contains( icon ) ) {
		processedIcons.append( icon );
		sendIcon( exe, icon, type );
	    }
	}

    }

    // send all the doc links
    bool sentADocLink = FALSE;
    for ( QListIterator<DocLnk> it( d->dls.children() ); it.current(); ++it ) {
	sentADocLink = TRUE;
	sendDocLnk( it.current() );
    }

    if ( !sentADocLink ) {
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "docLinks(QString)" );
	e << "";
#endif
    }

    d->needToSendAllDocLinks = false;
}

void DocumentList::sendDocLnk( DocLnk *doc )
{
    Categories cats;
    QString contents;
    QFileInfo fi( doc->file() );
    if ( !fi.exists() ) {
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "linkFileRemoved(QString)" );
	e << doc->file();
#endif
	return;
    }

    bool fake = !doc->linkFileKnown();
    if ( !fake ) {
	QFile f( doc->linkFile() );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    ts.setEncoding( QTextStream::UnicodeUTF8 );
	    QString docLnk = ts.read();
	    // Strip out the (stale) LinkFile entry
	    int start = docLnk.find( "\nLinkFile = " ) + 1;
	    if ( start > 0 ) {
		int end = docLnk.find( "\n", start + 1 ) + 1;
		contents += docLnk.left(start);
		contents += docLnk.mid(end);
	    } else {
		contents += docLnk;
	    }
	    contents += "LinkFile = " + doc->linkFile() + "\n";
	    f.close();
	} else
	    fake = TRUE;
    }
    if (fake) {
	contents += "[Desktop Entry]\n"; // No tr
	contents += "Categories = " + // No tr
	    cats.labels("Document View",doc->categories()).join(";") + "\n"; // No tr
	contents += "Name = "+doc->name()+"\n"; // No tr
	contents += "Type = "+doc->type()+"\n"; // No tr
    }
    contents += "File = "+doc->file()+"\n"; // No tr // (resolves path)
    contents += QString("Size = %1\n").arg( fi.size() ); // No tr
#ifndef QT_NO_COP
    QCopEnvelope e( "QPE/Desktop", "docLink(QString)" );
    e << contents;
#endif
}

void DocumentList::sendIcon( QString exe, QString icon, QString type )
{
#ifndef QT_NO_COP
    QString pixmap = Resource::findPixmap( icon );
    if ( pixmap.isNull() ) {
	pixmap = Resource::findPixmap( icon.mid( exe.length() + 1 ) );
	if ( pixmap.isNull() )
	    return; // give up
    }
    QString realicon = pixmap.mid( pixmap.find("pics/") + 5 );
    QFile f( pixmap );
    f.open( IO_ReadOnly );
    QByteArray data = f.readAll();
    QCopEnvelope e( "QPE/Desktop", "docLinkIcon(QString,QString,QByteArray)" );
    e << type << realicon << data;
#endif
}

// ====================================================================

DocumentListPrivate::DocumentListPrivate( ServerInterface *gui )
{
    serverGui = gui;
    if ( serverGui ) {
	sendAppLnks = serverGui->requiresApplications();
	sendDocLnks = serverGui->requiresDocuments();
    } else {
	sendAppLnks = false;
	sendDocLnks = false;
    }
    for ( int i = 0; i < MAX_SEARCH_DEPTH; i++ ) {
	listDirs[i] = 0;
	lists[i] = 0;
	listPositions[i] = 0;
    }
    tid = 0;
#ifdef QTOPIA_PHONE
    hasDocBeenLoaded = FALSE;
#else
    hasDocBeenLoaded = TRUE;
#endif
}


void DocumentListPrivate::initialize()
{
    // Reset
    dls.clear();
    docPaths.clear();
    reference.clear();

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


DocumentListPrivate::~DocumentListPrivate()
{
    for ( int i = 0; i < MAX_SEARCH_DEPTH; i++ ) 
	if ( listDirs[i] ) 
	    delete listDirs[i];
    delete dit;
}


void DocumentListPrivate::estimatedPercentScanned()
{
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

    if ( serverGui )
	serverGui->documentScanningProgress( (int)overallProgress );
}


const QString DocumentListPrivate::nextFile()
{
    while ( TRUE ) {
	while ( searchDepth < 0 ) {
	    // go to next base path
	    if ( docPathsSearched >= docPaths.count() ) {
		// end of base paths
		return QString::null;
	    } else {
		QDir dir( docPaths[docPathsSearched] );
		// qDebug("now using base path: %s", docPaths[docPathsSearched].latin1() );
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
	    lists[searchDepth] = 0;
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
			// qDebug("now going in to path: %s", bn.latin1() );
			if ( !dir.exists( ".Qtopia-ignore" ) ) {
			    if ( searchDepth < MAX_SEARCH_DEPTH - 1) {
				searchDepth++;
				listDirs[searchDepth] = new QDir( dir );
				lists[searchDepth] = listDirs[searchDepth]->entryInfoList();
				listPositions[searchDepth] = 0;
			    }
			}
		    }
		} else {
		    estimatedPercentScanned();
		    return fl->at(pos)->filePath();
		}
	    }
	}
    }

    return QString::null;
}


bool DocumentListPrivate::store( DocLnk* dl )
{
    // if ( dl->fileKnown() && !dl->file().isEmpty() ) {
    if ( dl && dl->fileKnown() ) {
	dls.add( dl ); // store
	return TRUE;
    }

    // don't store - delete
    delete dl;
    return FALSE;
}


#define MAGIC_NUMBER	((void*)2)

const DocLnk *DocumentListPrivate::iterate(bool &finished)
{
    int filesScanned = 0;
    finished = false;
    if ( state == Find ) {
	//qDebug("state Find");
	QString file = nextFile();
	while ( !file.isNull() ) {
	    if ( file.right(8) == ".desktop" ) { // No tr
		DocLnk* dl = new DocLnk( file );
		if ( store(dl) )
		    return dl;
	    } else {
		reference.insert( file, MAGIC_NUMBER );
		filesScanned++;
		if ( filesScanned > 10 )
		    return 0;
	    }
	    file = nextFile();
	}
	state = RemoveKnownFiles;

	if ( serverGui )
	    serverGui->documentScanningProgress( 75 );
    }

    static int iterationI;
    static int iterationCount;

    if ( state == RemoveKnownFiles ) {
	//qDebug("state RemoveKnownFiles");
	const QList<DocLnk> &list = dls.children();
	for ( QListIterator<DocLnk> it( list ); it.current(); ++it ) {
	    reference.remove( (*it)->file() );
	    // ### does this need to be deleted?
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
		    if ( serverGui ) 
			serverGui->documentScanningProgress( 75 + (25*iterationI)/iterationCount );
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


DocumentList::DocumentList( ServerInterface *serverGui, bool scanDocs,
			    QObject *parent, const char *name )
 : QObject( parent, name )
{
    appLnkSet = new AppLnkSet( MimeType::appsFolderName() );
    d = new DocumentListPrivate( serverGui );
    d->storage = new StorageInfo( this );
    d->initialize();
    d->scanDocs = scanDocs;
    d->needToSendAllDocLinks = false;

    QTimer::singleShot( 10, this, SLOT( startInitialScan() ) );
}


void DocumentList::startInitialScan()
{
    reloadAppLnks();
    if( d->hasDocBeenLoaded )
	reloadDocLnks();
}


DocumentList::~DocumentList()
{
    delete appLnkSet;
    delete d;
}


void DocumentList::add( const DocLnk& doc )
{
    if ( d->serverGui && QFile::exists( doc.file() ) )
	d->serverGui->documentAdded( doc );
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
	d->tid = startTimer( 10 );
}

/*
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
*/

void DocumentList::rescan()
{
    pause();
    d->initialize();
    resume();
}


void DocumentList::timerEvent( QTimerEvent *te )
{
    if ( te->timerId() == d->tid ) {
	// Do 3 at a time
	for (int i = 0; i < 3; i++ ) {
	    bool finished;
	    const DocLnk *lnk = d->iterate(finished);
	    if ( lnk ) {
		add( *lnk );
	    } else if ( finished ) {
		// stop when done
		pause();
		if ( d->serverGui )
		    d->serverGui->documentScanningProgress( 100 );
		if ( d->needToSendAllDocLinks )
		    sendAllDocLinks();
		d->hasDocBeenLoaded = TRUE;
		break;
	    }
	}
    }
}

