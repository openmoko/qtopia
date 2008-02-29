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

#include "contentserver.h"
#include "qtopiaserverapplication.h"

#include <qstorage.h>
#include <qtopiaapplication.h>
#include <qmimetype.h>
#include <qcontent.h>
#include <qtopiaipcenvelope.h>
#include <qtopialog.h>
#include <qcategorymanager.h>
#include <qtopiasql.h>
#include <qdrmcontentplugin.h>

#include <QTimer>
#include <QDebug>
#include <QStyle>
#include <QSemaphore>
#include <QValueSpaceObject>

// Making this larger will more greatly paralellize scanning at a cost
// of creating lots of threads
static const int MaxDirThreads = 2;

// Making this larger will cause the scanner to go more deeply into
// subdirectories
static const int MaxSearchDepth = 10;

/*
  Recursive threaded background directory scanner for advanced use only.
  This class is used behind the scenes in the ContentServer and should
  not be needed unless undertaking advanced customization.
 */

class DirectoryScanner : public QThread
{
    Q_OBJECT
public:
    DirectoryScanner( int pr );
    ~DirectoryScanner();
    virtual void run();

public slots:
    void startScan(const QString& path, int depth);

signals:
    void scanFinished();

private:
    QContentList adds;
    int priority;
};

/*
  Manager for recursive threaded background directory scanner.
  This class is used behind the scenes in the ContentServer and should
  not be needed unless undertaking advanced customization.

  See also: DirectoryScanner
 */

class DirectoryScannerManager : public QObject
{
    Q_OBJECT
public:
    DirectoryScannerManager(QObject *parent);

    static void scan(const QString &path, int priority);

    void addPath(const QString &path, int priority, int depth);

    bool isScanning();

private slots:
    void scannerDone();
    void scannerCleanup();

private:
    void startScanner(DirectoryScanner *scanner = NULL);

private:
    struct PendingPath {
        QString path;
        int priority;
        int depth;

        bool operator<(const PendingPath &pp) const {
            return priority < pp.priority;
        }
    };
    QList<PendingPath> pendingPaths;
    QList<DirectoryScanner*> scanners;
    QSemaphore threadCount;
    QMutex mutex;
};

static DirectoryScannerManager *scannerManager = 0;

class AppLoaderPrivate
{
public:
    AppLoaderPrivate() : appCategories(QLatin1String("Applications")) {}

    QSet<QContentId> serversApps;
    QSet<QContentId> serversDocs;
    QList<QContentSet> docs;
    bool requiresApps;
    QCategoryManager appCategories;
    QHash<QString,QContentSet*> apps;
    static QMutex guardMutex;
};

class ContentProcessor : public QThread
{
public:
    ContentProcessor() : QThread() {}
    virtual ~ContentProcessor() {}
    virtual void run();
};

QMutex AppLoaderPrivate::guardMutex(QMutex::Recursive);

// This value controls how many doclinks are processed at one go before firing off a single shot timer to yield processing
// before continuing processing
const int docsPerShot = 100;
/*!
  \class ContentServer
  \brief The ContentServer class provides the Documents API.
  \internal

  Server process implementing the ContentServerInterface, running in
  a separate thread.  Handles updating ContentSet objects in reponse
  to database updates.
*/

////////////////////////////////////////////////////////////////////////
/////
/////  ContentServer implementation

ContentServer::ContentServer( QObject *parent )
    : ContentServerInterface( parent )
{
    qLog(DocAPI) << "content server constructed";
    qLog(Performance) << "DocAPI : " << "content server constructed : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    connect(&contentTimer, SIGNAL(timeout()), this, SLOT(processContent()));
    contentTimer.setSingleShot(true);
    scannerVSObject = new QValueSpaceObject("/Documents", this);
    // force early initialisation of QMimeType to remove the chance of issues later on when we're in threads.
    QDrmContentPlugin::initialize();
    QMimeType::updateApplications();
}

ContentServer::~ContentServer()
{
    if(scannerVSObject)
        delete scannerVSObject;
}

/*!
  \internal
  Return a pointer to the instance of the ContentServer object.
  There can be only one.
  NEVER call this from outside the Qtopia server
*/
ContentServer *ContentServer::getInstance()
{
    static ContentServer theInstance;

    if ( ContentServerInterface::serverInstance == 0 )
        ContentServerInterface::serverInstance = &theInstance;

    return &theInstance;
}


/*!
  \internal
  This code runs in a separate thread to that in which the ContentServer
  object is created.  It sets up the initial scanning requests by adding
  them to the queue object, then enters the thread's event loop.

  Inside the event loop changes to the QContentSet's internal QStorageMetaInfo
  object, linkChanged(...) messages, and new requests will cause new scans
  to be launched.  New requests may be posted by sending
  QtopiaIpcAdaptor::send(SIGNAL(scanPath(const QString&,int), path, priority)
  to send to the QPE/DocAPI channel, with path being the path to be scanned.
*/
void ContentServer::run()
{
    RequestQueue rq;

    rq.publish(SLOT(scanPath(const QString &,int)));
    rq.publish(SLOT(registerCLS(const QString&, const QContentFilterSet&, int)));
    rq.publish(SLOT(unregisterCLS(const QString&, int)));

    QTimer::singleShot(4000, this, SLOT(initDocScan()));

    exec();
}

void ContentServer::initDocScan()
{
    // Scan all locations that can hold Documents but only if they're connected
    QStorageMetaInfo storage;
    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, storage.fileSystems( &fsf, true )) {
        // FIXME Storage.conf should be more flexible so that this magic is not required
        QString path = fs->path();
        if ( fs->disk() == "HOME" )
            path = fs->path() + "/Documents";
        DirectoryScannerManager::scan(path, 0);
    }
}

void ContentServer::removeContent(QContentIdList cids)
{
    QMutexLocker guard(&AppLoaderPrivate::guardMutex);
    if (removeList.isEmpty())
        QMetaObject::invokeMethod(&contentTimer, "start", Qt::QueuedConnection, Q_ARG(int,10));
    removeList += cids;
}

void ContentServer::addContent(const QFileInfo &fi)
{
    QMutexLocker guard(&AppLoaderPrivate::guardMutex);
    if (addList.isEmpty())
        QMetaObject::invokeMethod(&contentTimer, "start", Qt::QueuedConnection, Q_ARG(int,10));
    addList.append(fi);
}

void ContentServer::processContent()
{
    static ContentProcessor contentProcessor;
    if (removeList.count() || addList.count())
    {
        if(!contentProcessor.isRunning())
            contentProcessor.start(QThread::NormalPriority);
        contentTimer.start(10);
    }
}

void ContentProcessor::run()
{
    setTerminationEnabled(false);
    QMutexLocker guard(&AppLoaderPrivate::guardMutex);
    int processed = 0;
    QList<QContentId> deletes;
    QList<QFileInfo> installs;

    while (ContentServer::getInstance()->removeList.count() && ++processed < docsPerShot)
        deletes.append(ContentServer::getInstance()->removeList.takeFirst());

    while (ContentServer::getInstance()->addList.count() && ++processed < docsPerShot)
        installs.append(ContentServer::getInstance()->addList.takeFirst());

    if(!deletes.isEmpty())
        QContent::uninstallBatch(deletes);
    if(!installs.isEmpty())
        QContent::installBatch(installs);
    setTerminationEnabled(true);
}


////////////////////////////////////////////////////////////////////////
/////
/////  RequestQueue Implementation


RequestQueue::RequestQueue()
    : QtopiaIpcAdaptor(QLatin1String("QPE/DocAPI"))
{
}

RequestQueue::~RequestQueue()
{
}


/*!
  The server works by keeping a mirror of each QContentSet object
  created.  The mirror is registered during construction, and
  deregistered when destructed.  Changes to the filter expression
  cause the mirror to be deregistered and recreated with the new
  expression.  When the QContentSet mirror is changed, it propagates
  the change out to the original.

  This way changes to the database are only notified if an application
  is interested in them, and only those applications are notified.
*/

void RequestQueue::scanPath(const QString &newPath, int priority)
{
    DirectoryScannerManager::scan(newPath, priority);
}

void RequestQueue::registerCLS(const QString& CLSid, const QContentFilter& filters, int index)
{
    Q_UNUSED(CLSid);
    Q_UNUSED(filters);
    Q_UNUSED(index);
/*
    QContentSet *mirror = new QContentSet( filters );
    QString mkey = QString( "%1-%2" ).arg( CLSid ).arg( index );
    mirrors[ mkey ] = mirror;
    mirrorsByPtr[ mirror ] = CLSid;
    connect( mirror, SIGNAL( changed( QContentId, QContent::ChangeType )),
             this, SLOT( reflectMirror( QContentId, QContent::ChangeType )), Qt::QueuedConnection);
*/
}

void RequestQueue::unregisterCLS(const QString& CLSid, int index)
{
    QString mkey = QString( "%1-%2" ).arg( CLSid ).arg( index );
    if ( !mirrors.contains( mkey ))
        return;
    QContentSet *mirror = mirrors[ mkey ];
    mirrorsByPtr.remove( mirror );
    delete mirrors.take( mkey );
}


void RequestQueue::receiveUpdate()
{

}

/*!
  \internal
  Send out an update message reflecting the fact that the
  cid has changed - this is picked up by the app that is interested
  in it, which in turn fetches the fresh data from the database.

  TODO - Optimise for case of a large number of updates, eg
  when scanning large directories of files.  Suggest on sending an
  update, set a timer for 50 milliseconds.  If the next update is
  due to be sent before the timer expires, instead batch it up and
  reset the timer.  Keep accumulating batched updates until either
  the timer times out (no updates for 50 ms) or 50 updates
  occur.  Maybe also a "mass update" message is required.
*/
void RequestQueue::reflectMirror( QContentId cid, QContent::ChangeType ctype )
{
    QContentSet *m = static_cast<QContentSet *>( sender() );
    //Q_ASSERT( mirrorsByPtr.contains( m ));    // things are badly broken if not
    qLog(DocAPI) << "RequestQueue::reflectMirror" << mirrorsByPtr[m] << cid;
    QtopiaIpcEnvelope e( mirrorsByPtr[m].prepend( QLatin1String("QPE/Application/") ), QLatin1String("updateContent(QContentId,QContent::ChangeType)") );
    e << cid;
    e << ctype;
}

////////////////////////////////////////////////////////////////
//
// DirectoryScanner threaded directory scanner implementation


DirectoryScannerManager::DirectoryScannerManager(QObject *parent)
    : QObject(parent), threadCount(MaxDirThreads), mutex(QMutex::Recursive)
{
    QMetaObject::invokeMethod(ContentServer::getInstance()->scannerVSObject, "setAttribute", Q_ARG(QString, QLatin1String("Scanning")), Q_ARG(QVariant, isScanning()));
}

void DirectoryScannerManager::scan(const QString &path, int priority)
{
    if (!scannerManager)
        scannerManager = new DirectoryScannerManager(0);
    if(path == QLatin1String("all"))
    {
        QTimer::singleShot(0, ContentServer::getInstance(), SLOT(initDocScan()));
    }
    else
        scannerManager->addPath(path, priority, 0);
}

void DirectoryScannerManager::addPath(const QString &path, int priority, int depth)
{
    QMutexLocker lock(&mutex);
    qLog(DocAPI) << "Adding path to scan:" << path << ", priority:" << priority << "depth" << depth;

    PendingPath pp;
    pp.path = path;
    pp.priority = priority;
    pp.depth = depth;
    for (int i = 0; i < pendingPaths.count(); ++i) {
        if (pendingPaths.at(i).path == path) {
            pendingPaths.removeAt(i);
            break;
        }
    }
    pendingPaths.append(pp);
    qSort(pendingPaths);
    startScanner();
}

void DirectoryScannerManager::scannerDone()
{
    QMutexLocker lock(&mutex);
    int s = scanners.indexOf((DirectoryScanner*)(sender()));
    if (s < 0) {
        // this would be a programmer error...
        qWarning() << "signalled by an unknown scanner!!";
        return;
    }
    DirectoryScanner *ds = scanners.at(s);
    //qLog(DocAPI) << "finished scanning" << ds->path() << ", thread count" << threadCount.available();

    if(pendingPaths.count() == 0) {
        QMetaObject::invokeMethod(ds, "quit", Qt::QueuedConnection);
        qLog(Performance) << "DocAPI : " << "finished scanning : "
                          << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    }
    else
        startScanner(ds);
}

void DirectoryScannerManager::scannerCleanup()
{
    int s=scanners.indexOf((DirectoryScanner*)(sender()));
    if(s >= 0)
        scanners.takeAt(s);
    delete (DirectoryScanner*)(sender());
    threadCount.release();
    QMetaObject::invokeMethod(ContentServer::getInstance()->scannerVSObject, "setAttribute", Q_ARG(QString, QLatin1String("Scanning")), Q_ARG(QVariant, isScanning()));
}


void DirectoryScannerManager::startScanner(DirectoryScanner *scanner)
{
    QMutexLocker lock(&mutex);
    if (pendingPaths.count()) {
        DirectoryScanner *scan=scanner;
        PendingPath pp = pendingPaths.takeLast();
        qLog(DocAPI) << "preparing to scan" << pp.path << "priority" << pp.priority << ", thread count" << threadCount.available();
        if(scan == NULL)
        {
            if(threadCount.tryAcquire())
            {
                scan = new DirectoryScanner(pp.priority);

                connect(scan, SIGNAL(scanFinished()), this, SLOT(scannerDone()), Qt::QueuedConnection);
                connect(scan, SIGNAL(finished()), this, SLOT(scannerCleanup()), Qt::QueuedConnection);
                connect(scan, SIGNAL(terminated()), this, SLOT(scannerCleanup()), Qt::QueuedConnection);

                scanners.append(scan);

                scan->start(QThread::LowestPriority);

                QMetaObject::invokeMethod(ContentServer::getInstance()->scannerVSObject, "setAttribute", Q_ARG(QString, QLatin1String("Scanning")), Q_ARG(QVariant, isScanning()));
            }
            else
            {
                pendingPaths.append( pp );
                return;
            }
        }

        QMetaObject::invokeMethod(scan, "startScan", Qt::QueuedConnection, Q_ARG(QString, pp.path), Q_ARG(int, pp.depth));
    }
    else if(scanner!=NULL)
        QMetaObject::invokeMethod(scanner, "quit", Qt::QueuedConnection);
}

bool DirectoryScannerManager::isScanning()
{
    qLog(DocAPI) << __PRETTY_FUNCTION__ << "threadCount.available() =" << threadCount.available();
    qLog(DocAPI) << __PRETTY_FUNCTION__ << "returning" << (threadCount.available() != MaxDirThreads);
    return threadCount.available() != MaxDirThreads;
}
/*!
   Construct a directory scanner object representing one thread scanning
   one directory
*/
DirectoryScanner::DirectoryScanner( int pr )
    : priority(pr)
{
    moveToThread(this);
}

/*!
    Destroy this directory scanner object, releasing a thread resource so
    that other scans may start up.  A semaphore is used to ensure no more
    than MaxDirThreads (typically 10) threads are scanning at once.
*/
DirectoryScanner::~DirectoryScanner()
{
}

/*!
  \internal
  Do a breadth first scan of this DirectoryScanners dirPath, starting a new DirectoryScanner
  for any directories encountered during the search.  If there are already
  MaxDirThreads scanner threads running, block until one finishes.

  The finished/terminate signals from the sub-DirectoryScanners will be delivered
  immediately because those objects live in the same thread as this one.  quit()
  however will not be received unless running in the event loop.
*/
void DirectoryScanner::run()
{
    exec();
}

void DirectoryScanner::startScan(const QString& path, int depth)
{
    setTerminationEnabled(false);
    QString dirPath=QDir::cleanPath(path);
    qLog(DocAPI) << "thread" << QThread::currentThreadId() << "scan called for path" << dirPath << "depth" << depth;
    QDir dir( dirPath );

    QContentSet dirContents( QContentFilter( QContentFilter::Directory, dirPath ) );
    QContentSetModel dirModel(&dirContents);
    QFileInfoList ents = dir.entryInfoList( QDir::Files | QDir::Hidden | QDir::Dirs | QDir::NoDotAndDotDot );
    QContentIdList removeList;
    for (int i = 0; i < dirModel.rowCount(); i++)
    {
        QContent content = dirModel.content(i);
        if(content.fileKnown() && QFileInfo(content.file()).path() == dir.path())
        {
            int idx = ents.indexOf(content.file());
            if (idx >= 0 &&
                !(content.type() == "application/octet-stream" && QMimeType(content.file()).id() != "application/octet-stream") )
                ents.removeAt(idx);
        }
        if(content.linkFileKnown() && QFileInfo(content.linkFile()).path() == dir.path())
        {
            if(ents.contains(content.linkFile()))
            {
                if( content.linkFile().endsWith(QLatin1String(".desktop"))
                   && (content.lastUpdated() == QDateTime()
                   || content.lastUpdated() < QFileInfo(content.linkFile()).lastModified()))
                    /*not removing, so don't do anything*/;
                else
                    ents.removeAll(content.linkFile());
            }
        }
    }

    if(depth==0) // only do it for the top level of this directory
    {
        // Now scan for all files under this location, and check if they're valid or not, if not, add them to the removelist too.
        dirContents.setCriteria( QContentFilter( QContentFilter::Location, dirPath ) );
        for (int i = 0; i < dirModel.rowCount(); i++)
        {
            QContent content = dirModel.content(i);
            if( !content.isValid(true) && content.property( "Preloaded" ) != "y")
            {
                removeList.append(content.id());
            }
        }
    }

    ContentServer::getInstance()->removeContent(removeList);

    // By now we should have removed all the files that already exist in
    // the database from the ents list, and uninstalled all the files in the database that don't
    // exist any more.
    foreach( QFileInfo entry, ents )
        if( !QtopiaSql::isDatabase( entry.absoluteFilePath() ) )
            ContentServer::getInstance()->addContent( entry );

    ents = dir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );

    foreach( QFileInfo entry, ents )
        if (depth < MaxSearchDepth && !entry.fileName().startsWith(QLatin1String(".")))
            scannerManager->addPath(dir.path()+'/' + entry.fileName(), priority-1, depth+1);

    emit scanFinished();
    setTerminationEnabled(true);
}

// define ContentServerTask
/*!
  \class ContentServerTask
  \ingroup QtopiaServer::Task
  \brief The ContentServerTask class manages the backend for the Documents API.

  The ContentServerTask is required for the Documents API to function correctly.
  The content server is responsible for periodically scanning document and
  application directories to maintain the integrity of the documents database.
  The content server is also responsible for scanning newly detected media,
  such as extenal memory cards, for content and integrating them into the
  document model.

  The ContentServerTask class provides the \c {ContentServer} task.
 */
QTOPIA_TASK(ContentServer, ContentServerTask);
QTOPIA_TASK_PROVIDES(ContentServer, SystemShutdownHandler);

/*! \internal */
ContentServerTask::ContentServerTask()
: m_finished(false)
{
    ContentServer *dup = ContentServer::getInstance();
    dup->start( QThread::LowPriority );
}

/*! \internal */
void ContentServerTask::finished()
{
    if(m_finished) return;

    m_finished = true;
    emit proceed();
}

/*! \internal */
bool ContentServerTask::systemRestart()
{
    doShutdown();
    return false;
}

/*! \internal */
bool ContentServerTask::systemShutdown()
{
    doShutdown();
    return false;
}

/*! \internal */
void ContentServerTask::doShutdown()
{
    ContentServer *s = ContentServer::getInstance();
    QObject::connect(s, SIGNAL(finished()), this, SLOT(finished()));
    QTimer::singleShot(500, this, SLOT(finished()));
    s->quit();
}

#include "contentserver.moc"

