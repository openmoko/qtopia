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

#include <qtextstream.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdebug.h>

#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <quuid.h>
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#ifndef Q_OS_MACX
#include <sys/vfs.h>
#endif
#endif

#include <qsettings.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#ifdef QTOPIA_DESKTOP
#include <qcopenvelope_qd.h>
#else
#include <qtopiaipcenvelope.h>
#endif

#include "emailfolderaccess.h"
#include "mailmessage.h"

class AccessPrivate
{
public:
    AccessPrivate() {
        version = 0x01abde70;
    }

    void uuidToBinary(QUuid id, QDataStream &stream ) {
        stream << id.data1 << id.data2 << id.data3;
        for (int i = 0; i < 8; i++) {
            stream << id.data4[i];
        }
    }

    QUuid binaryToUuid(QDataStream &stream) {
        QUuid id;
        stream >> id.data1 >> id.data2 >> id.data3;
        for (int i = 0; i < 8; i++) {
            stream >> id.data4[i];
        }

        return id;
    }

    void resizeCache(QByteArray b) {

        uint previousSize = headersCached.size();
        headersCached.resize( previousSize + b.size() );

        uint count = b.size();
        char *hPtr = headersCached.data();
        hPtr += previousSize;

        char *bPtr = b.data();
        while ( count > 0 ) {
            *hPtr = *bPtr;
            hPtr++;
            bPtr++;
            count--;
        }

    }

    quint32 version;
    QByteArray headersCached;
};

/*  EmailFolderAccess */
EmailFolderAccess::EmailFolderAccess(QString mailbox, QObject *parent)
    : QObject(parent)
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/Mail", this);
    connect (channel, SIGNAL( received(const QString&,const QByteArray&) ),
            this, SLOT(channelMsg(const QString&,const QByteArray&) ) );

    d = new AccessPrivate();

    _status = Closed;

    _mailbox = mailbox.toLower();

    QString location = Qtopia::applicationFileName("qtmail", "");
    curDirPath = location + _mailbox + "/cur";
    tmpDirPath = location + _mailbox + "/tmp";
    indexfn = location + _mailbox + "i.dat";
    headerfn = location + _mailbox + "h.dat";

    if ( ensureExists() ) {
        _status = Open;
        ensureCurrent();
    }

    //find and remove any dangling mail and temp files
    ensureSynced();

    _sendMsgTimer.setSingleShot(true);
    _dirtyIndexTimer.setSingleShot(true);
    _dirtyIndexTimer.setInterval(0);
    _indexWriteSkip = 0;
    connect(&_dirtyIndexTimer,SIGNAL(timeout()),this,SLOT(writeDirtyIndex()));
    connect(&_sendMsgTimer,SIGNAL(timeout()),this,SLOT(sentMailBoxChanged()));
}

EmailFolderAccess::~EmailFolderAccess()
{
    //modified the mailbox but not had enough
    //time to write the index, so force it.
    if(_dirtyIndexTimer.isActive())
        writeDirtyIndex();
    delete d;
}

bool EmailFolderAccess::hasMail(const QUuid &id)
{
    if ( !_status == Open )
        return false;

    return mappedMails.contains( id );
}

QList<QUuid> EmailFolderAccess::mailUuidList() const
{
    return _uuidList;
}

QUuid EmailFolderAccess::generateUuid() const
{
    return QUuid::createUuid();
}

bool EmailFolderAccess::addMail(MailMessage &m)
{
    if ( !ensureCurrent() )
        return false;

    // Exit if list contains the uuid
    if ( mappedMails.contains( m.uuid() ) )
        return false;

    IndexItem newRef;
    if ( !appendMailString(m, &newRef) ) {
        //remove the created mail file
        QString newMailString = curDirPath + "/" + newRef.mailFile;

        if(!QFile::remove(newMailString))
            qWarning("Unable to remove the failed mail create %s",qPrintable(newMailString));
        return false;
    }

    if ( !saveHeader(m, newRef) ) {
        //remove the created mail file
        QString newMailString = curDirPath + "/" + newRef.mailFile;

        if(!QFile::remove(newMailString))
            qWarning("Unable to remove the failed mail save %s",qPrintable(newMailString));
        return false;
    }

    mappedMails.insert( m.uuid(), newRef);
    _uuidList.append(m.uuid());

    tryWriteIndex();
    sendMsg("mailboxChanged");
    return true;
}

bool EmailFolderAccess::removeMail(const QUuid &id)
{
    if ( !ensureCurrent() )
        return false;

    QMap<QUuid, IndexItem>::Iterator ref = mappedMails.find( id );
    if ( ref == mappedMails.end() )
        return false;

    IndexItem oldRef = *ref;
    mappedMails.erase(ref);

    _uuidList.removeAll(id);

    tryWriteIndex();

    //remove the mail file

    QString mailFile = curDirPath + "/" + oldRef.mailFile;
    if(!QFile::remove(mailFile))
    {
        qWarning("Removing the mail file %s failed",qPrintable(mailFile));
        return false;
    }

    sendMsg("mailboxChanged");
    return true;
}

bool EmailFolderAccess::updateFullMail(MailMessage &m)
{
    if ( !ensureCurrent() )
        return false;

    QMap<QUuid, IndexItem>::Iterator ref = mappedMails.find( m.uuid() );
    if ( ref == mappedMails.end() )
        return false;

    //remove the old mail file

    IndexItem newItem;
    if ( !appendMailString(m, &newItem) )
    {
        qWarning("Unable to add the updated mail file %s",qPrintable(newItem.mailFile));
        QFile::remove( curDirPath + "/" + newItem.mailFile );
        return false;
    }

    QString newPath = curDirPath + "/" + newItem.mailFile;

    if ( !saveHeader(m, newItem) )
    {
        //remove the updated file
        if(!QFile::remove(newPath))
            qWarning("Unable to remove updated mail file %s",qPrintable(newPath));
        return false;
    }

    QString oldpath = curDirPath + "/" + (*ref).mailFile;;
    if(!QFile::remove(oldpath))
    {
        qWarning("Unable to remove the old mail file %s",qPrintable(oldpath));
        if(!QFile::remove(newPath))
            qWarning("Unable to remove updated mail file %s",qPrintable(newPath));
        return false;
    }

    IndexItem oldItem = *ref;
    *ref = newItem;

    tryWriteIndex();
    sendMsg("mailboxChanged");
    return true;
}

bool EmailFolderAccess::updateMailContextInfo(const MailMessage &m)
{
    if ( !ensureCurrent() )
        return false;

    QMap<QUuid, IndexItem>::Iterator ref = mappedMails.find( m.uuid() );
    if ( ref == mappedMails.end() )
        return false;

    IndexItem newItem = *ref;

    if ( !saveHeader(m, newItem) )
        return false;

    IndexItem oldItem = *ref;
    *ref = newItem;

    tryWriteIndex();
    sendMsg("mailboxChanged");
    return true;
}

MailMessage EmailFolderAccess::fullMail(const QUuid &id)
{
    MailMessage m;

    if ( !_status == Open )
        return m;

    QMap<QUuid, IndexItem>::ConstIterator ref = mappedMails.find( id );
    if ( ref == mappedMails.end() )
        return m;

    QString mailFile = curDirPath + "/" + (*ref).mailFile;

    QFile f( mailFile );
    if ( !f.open(QIODevice::ReadOnly) ) {
        qWarning("could not open mail file: %s", qPrintable(mailFile) );
        return m;
    }

    LongString ls( f.fileName(), false );
    m.fromRFC822( ls );

    QByteArray h(d->headersCached.data() + (*ref).headerFilePos, (*ref).headerFileSize);

    setContextInfo(&m, h, true );
    m.setUuid( id );

    return m;
}

MailMessage EmailFolderAccess::mailContextInfo(const QUuid &id)
{
    MailMessage m;

    if ( !_status == Open )
        return m;

    QMap<QUuid, IndexItem>::Iterator ref = mappedMails.find( id );
    if ( ref != mappedMails.end() ) {
        QByteArray b( d->headersCached.data() + (*ref).headerFilePos, (*ref).headerFileSize);

        setContextInfo(&m, b );
        m.setUuid( id );
    }

    return m;
}

void EmailFolderAccess::channelMsg(const QString &msg, const QByteArray &args)
{
    if ( msg == "mailboxChanged(int,QString)") {
        int pid;
        QString mailbox;
        QDataStream stream(args);
        stream >> pid >> mailbox;

        //If we sent the message, or it's another mailbox, just ignore the message
        if ( (getpid() == pid) || (mailbox != _mailbox ) )
            return;

        // force reload here
        setDirty();
        emit mailboxChanged();

    } else {
        qWarning("could not recognize msg: %s", msg.toLocal8Bit().constData());
    }
}

/*  We know the mailbox has been modified, so we need to force a reload.  Lastmodified
    time of files operates on seconds, so invalidate the datetime to ensure it gets
    reloaded even if a second hasn't passed since the modification time */
void EmailFolderAccess::setDirty()
{
    QDateTime d;
    indexDate = d;
    headerDate = d;

    ensureCurrent();
}

QString EmailFolderAccess::createUniqueFileName()
{
    //format: seconds_epoch.pid.randomcrap

    bool exists = true;
    QString filename;
    while(exists)
    {
        qint64 pid = 0;

#ifdef Q_OS_WIN32
    //TODO add win32 code
#else
    pid = getpid();
#endif

    filename.sprintf("%ld.%ld.",(unsigned long)time(0),(long)pid);
    filename += randomString(5);

    //check if it exists
    exists = QFile::exists(curDirPath + "/" + filename);

    }
    return filename;
}

QString EmailFolderAccess::randomString(int length)
{
    if (length <=0 ) return QString::null;

    QString str;

    str.resize( length );

    int i = 0;
    while (length--){
        int r=random() % 62;
        r+=48;
        if (r>57) r+=7;
        if (r>90) r+=6;
        str[i++] =  char(r);
    }
    return str;
}

bool EmailFolderAccess::appendMailString(MailMessage &m, IndexItem *newRef)
{
    QString filename = createUniqueFileName();
    QString path = curDirPath + "/" + filename;
    QFile mailFile(path);

#if 0
    // Just try to rename the file, currently doesn't work, because the file is mmap'd?
    if ( !m.fileName().isEmpty() ) {
        if (!QDir().rename(m.fileName(),path)) {
            qWarning( "Unable to rename mail file from %s to %s",
                      qPrintable( m.fileName() ),
                      qPrintable( path  ) );
            return false;
        }
        return true;
    }
#endif

    if ( isWriteLocked(mailFile) )
    {
        qWarning("could not obtain lock on mail file %s, aborting..",qPrintable(path));
        return false;
    }
    if (! mailFile.open(QIODevice::ReadWrite) ) {
        qWarning("could not open mail file: %s", qPrintable(path));
        return false;
    }
    if ( !lockDataFile(mailFile) ) {
        qWarning("could not obtain lock on mail file %s, aborting..",qPrintable(path));
        mailFile.close();
        return false;
    }

    //strip out the unecessary new lines
    //unnecessary
//     int strip = str.indexOf("\n.\n", 0);
//     while( strip != -1 ) {
//         if ( strip < str.length() - 3 ) {
//             str.replace(strip, 3, "\n..\n");
//             strip = str.indexOf("\n.\n", 0);
//         } else
//             strip = -1;
//     }
//
//     strip = str.indexOf("\n.\n", -3);
//     if ( strip == -1 )
//         str += "\n.\n";

    newRef->mailFile = filename;
    QTextStream out(&mailFile);

    //dont encode attachments for messages generated by us
    //do encode private headers

    QString checkPath = curDirPath + "/.";
    bool result = m.saveMail( out,checkPath );

    // If this one fails, we need to remove the appended junk so that
    // any recreation of indexfiles doesn't get confused
//     if ( mailFile.write(str.toLatin1(), str.length()) != str.length() ) {
//         unlockDataFile(mailFile);
//         mailFile.close();
//         mailFile.remove();
//         return false;
//     }

    unlockDataFile(mailFile);
    mailFile.close();

    return result;
}

bool EmailFolderAccess::saveHeader(const MailMessage &m, IndexItem &item)
{
    QFile f( headerfn );
    if ( isWriteLocked(f) )
        return false;
    if (! f.open(QIODevice::ReadWrite) ) {
        qWarning("could not save: %s", headerfn.toLocal8Bit().constData());
        return false;
    }
    if ( !lockDataFile(f) ) {
        qWarning("could not obtain lock on file %s, aborting..", (const char *)headerfn.toLatin1() );
        f.close();
        return false;
    }

    QByteArray b = contextInfo(m);
    item.headerFilePos = f.size();
    item.headerFileSize = b.size();

    f.seek( item.headerFilePos );
    if ( f.write(b, b.size() ) != int(b.size()) ) {
        unlockDataFile(f);
        f.close();

        // remove trailing junk
        Qtopia::truncateFile( f, item.headerFilePos );
        return false;
    }

    d->resizeCache( b );

    unlockDataFile(f);
    f.close();

    QFileInfo fi( f.fileName() );
    headerDate = fi.lastModified();

    return true;
}

void EmailFolderAccess::readIndexFile()
{
    QFile f( indexfn );
    if (! f.open(QIODevice::ReadOnly) ) {
        qWarning("could not read indexfile: %s", indexfn.toLocal8Bit().constData() );
        _status = Closed;
        return;
    }

    if ( !lockDataFile(f) ) {
        qWarning("could not obtain lock on file %s, aborting..", (const char *)indexfn.toLatin1() );
        f.close();
        _status = Locked;
        return;
    }

    mappedMails.clear();
    _uuidList.clear();

    QDataStream stream(&f);
    quint32 version;
    int count;
    stream >> version >> count;

    if ( version == d->version ) {
        QUuid id;
        for (int i = 0; i < count; i++ ) {
            IndexItem mRef;
            id = d->binaryToUuid( stream );
            stream >> mRef.mailFile;
            stream >> mRef.headerFilePos >> mRef.headerFileSize;
            _uuidList.append(id);
            mappedMails.insert( id, mRef );
        }
    } else {
        qWarning("incorrect index file version");
        unlockDataFile(f);
        f.close();
        _status = OldVersion;
        return;
    }

    unlockDataFile(f);
    f.close();

    QFileInfo fi( f.fileName() );
    indexDate = fi.lastModified();

    _status = Open;
}

bool EmailFolderAccess::writeIndexFile(QString filename)
{
    QFile f( filename );
    if ( isWriteLocked(f) )
        return false;
    if (! f.open(QIODevice::ReadWrite) ) {
        qWarning("could not write indexfile: %s", filename.toLocal8Bit().constData() );
        return false;
    }
    if ( !lockDataFile(f) ) {
        qWarning("could not obtain lock on file %s, aborting..", filename.toLocal8Bit().constData() );
        f.close();
        return false;
    }

    QByteArray b;
    QDataStream stream(&b, QIODevice::WriteOnly);
    stream << d->version;
    stream << mappedMails.count();

    QMap<QUuid, IndexItem>::ConstIterator it;

    for(int z = 0; z < _uuidList.count(); z++)
    {
        QUuid id = _uuidList[z];
        it = mappedMails.find(id);
        d->uuidToBinary( it.key(), stream );
        stream << (*it).mailFile;
        stream << (*it).headerFilePos << (*it).headerFileSize;
    }


    QFile newf( filename + ".new");
    if ( !newf.open(QIODevice::WriteOnly) ) {
        unlockDataFile(f);
        f.close();
        return false;
    }

    if ( newf.write(b, b.size() ) != int(b.size()) ) {
        unlockDataFile(f);
        f.close();

        newf.remove();  //might as well delete it
        return false;
    }

#if QT_VERSION >= 0x040100
//qt41 checks for existance of file before allowing rename
    QString tempName = f.fileName() + "-temp";
    if(!f.rename(tempName)){
        unlockDataFile(f);
        f.close();
        newf.remove();
        return false;
    }
#endif
    if ( !QFile::rename(newf.fileName(), f.fileName()) ) {
        unlockDataFile(f);
        f.close();
#if QT_VERSION >= 0x040100
    f.rename(f.fileName());
#endif
        newf.remove();
        return false;
    }
#if QT_VERSION >= 0x040100
    QFile::remove(tempName);
#endif
    newf.remove();

    unlockDataFile(f);
    f.close();


    QFileInfo fi( f.fileName() );
    indexDate = fi.lastModified();

    return true;
}

void EmailFolderAccess::readHeaderFile()
{
    QFile f( headerfn );
    if (! f.open(QIODevice::ReadOnly) ) {
        _status = Closed;
        return;
    }

    if ( !lockDataFile(f) ) {
        qWarning("could not obtain lock on file %s, aborting..", (const char *)headerfn.toLatin1() );
        f.close();
        _status = Locked;
        return;
    }

    d->headersCached = f.readAll();

    unlockDataFile(f);
    f.close();

    QFileInfo fi( f.fileName() );
    headerDate = fi.lastModified();

    _status = Open;
}

void EmailFolderAccess::setContextInfo(MailMessage *m, const QByteArray &b, bool keepOld)
{
    int i;
    QString s;
    QStringList l;
    QDateTime d;
    uint u;

    QDataStream stream(b);

    stream >> i; m->setAllStatusFields( i );
    stream >> i; m->setSize( i );
    stream >> s;
    if ( !keepOld )
        m->setFrom( s );

    stream >> s;
    if ( !keepOld )
        m->setSubject( s );

    stream >> s; m->setServerUid( s );
    stream >> d;
    if ( !keepOld )
        m->setDateTime( d );

    stream >> s; m->setFromAccount(s );
    stream >> s; m->setFromMailbox( s );
    stream >> l;
    if ( !keepOld)
        m->setTo( l );
    stream >> u;

    //  stored count of messageparts must always override the count returned from the mailmessage, otherwise
    // we could attempt to read beyond bounds.
    for (uint c = 0; c < m->messagePartCount(); c++) {
        if ( c < m->messagePartCount() && c < u) {
            MailMessagePart &part = m->messagePartAt( c );
            stream >> s; part.setStoredFilename( s );
            stream >> s; part.setLinkFile( s );
        }

    }
}

QByteArray EmailFolderAccess::contextInfo(const MailMessage &m)
{
    QByteArray b;
    QDataStream stream(&b, QIODevice::WriteOnly );

    stream << m.allStatusFields();
    stream << m.size();
    stream << m.from();
    stream << m.subject();
    stream << m.serverUid();
    stream << m.dateTime();
    stream << m.fromAccount();
    stream << m.fromMailbox();
    stream << m.to();
    stream << m.messagePartCount();

    for (uint i = 0; i < m.messagePartCount(); i++) {
        MailMessagePart part = m.messagePartAt( i );
        stream << part.storedFilename();
        stream << part.linkFile();
    }

    return b;
}

/*  Should only be called from the constructor.  If we cannot create
    the empty mailbox files, no further operations are possible */
bool EmailFolderAccess::ensureExists(bool forceNew)
{
    bool newMailbox = false;

    QDir curDir(curDirPath);
    QDir tmpDir(tmpDirPath);

    if(!curDir.exists() || !tmpDir.exists())
    {
        newMailbox = true;
        if(!curDir.mkpath(curDirPath) || ! tmpDir.mkpath(tmpDirPath))
        {
            _status = Invalid;
            return false;
        }

    }

    //we can treat it as new because any entries in the indexfiles would be invalid
    if ( curDir.entryList(QDir::Files).count() == 0 )
        newMailbox = true;

    if ( !newMailbox && !forceNew) {
        return ensureValid();
    }

    QFile f(indexfn);
    if ( !f.exists() || newMailbox ) {
        if ( !f.open(QIODevice::WriteOnly) ) {
            _status = Invalid;
            return false;
        }

        //Create an empty index file
        QDataStream stream(&f);
        stream << d->version;
        stream << 0;

        f.close();
    }

    f.setFileName(headerfn);
    if ( !f.exists() || newMailbox ) {
        if ( !f.open(QIODevice::WriteOnly) ) {
            _status = Invalid;
            return false;
        }

        f.close();
    }

    return true;
}

/*  Should only be called from ensureExists().  Verifies that the indexfiles
    are alive, and that all contextinformation is the correct version.  All older
    version will have an eg. inbox.txt file, but not the indexfiles.  If the indexfile
    or headerfile is missing we need to recreate the data.  */
bool EmailFolderAccess::ensureValid()
{
    QFileInfo fi(indexfn);
    QFileInfo fi2(headerfn);

    if ( fi.size() < 8 || fi2.size() == 0 ) {
        _status = IndexFileDeleted;
        return false;
    }

    return true;
}

bool EmailFolderAccess::ensureCurrent()
{
    //Invalid means the mailbox did not exist and we could not create it
    if ( _status != Open )
        return false;

    QFileInfo fi(indexfn);
    if ( fi.lastModified() != indexDate ) {
        readIndexFile();
        if ( _status != Open )
            qWarning("could not read indexFile %s", (const char *)indexfn.toLatin1() );
    }

    fi.setFile( headerfn );
    if ( fi.lastModified() != headerDate ) {
        readHeaderFile();
        if ( _status != Open )
            qWarning("could not read headerFile %s", (const char *)headerfn.toLatin1() );
    }

    return ( _status == Open );
}

bool EmailFolderAccess::isWriteLocked(QFile &file)
{
  bool result = false;
#ifndef Q_OS_WIN32
    struct flock fileLock;

    fileLock.l_type = F_WRLCK;

    if ( fcntl( file.handle(), F_GETLK, &fileLock) ) {
        result =  (fileLock.l_type == F_UNLCK);
    }
#else
    result = file.isOpen();

#endif
    return result;
}

/*  lock methods stolen from QPEPIM  */
bool EmailFolderAccess::lockDataFile(QFile& file)
{
    int lockMode;

    if (file.openMode() == QIODevice::ReadOnly)
        lockMode = 0;
    else
        lockMode = Qtopia::LockWrite;

    uint count = 0;
    while ( count < 10 ) {
        if ( Qtopia::lockFile(file, lockMode)){
            return true;
        }
        count++;
        Qtopia::sleep(2);
    }

  return false;
}

bool EmailFolderAccess::unlockDataFile(QFile& file)
{
    bool result = Qtopia::unlockFile(file);
#if !defined (Q_OS_WIN32) && defined (DEBUG)
    // "work" over NFS
    if (!result && errno == ENOLCK)
        result = true;
#endif
    return result;
}

/*  I don't believe it's feasible to send a signal indicating what
    has changed, as the backlog could easily expand so fast that another
    app could not reliably keep track   */
void EmailFolderAccess::sendMsg(const QString msg)
{
    Q_UNUSED(msg);
    _sendMsgTimer.start(0);
}

void EmailFolderAccess::sentMailBoxChanged()
{
    int pid = getpid();
    QtopiaIpcEnvelope e("QPE/Mail", "mailboxChanged(int,QString)");
    e << pid << _mailbox;
}

/*  Determine wheter it's time for compacting the header file or the mailbox file   */
void EmailFolderAccess::autoCompact()
{
    if ( !ensureCurrent() )
        return;

    uint usedHeaderSize= 0;

    QMap<QUuid, IndexItem>::ConstIterator it;
    for ( it = mappedMails.begin(); it != mappedMails.end(); ++it ) {
        usedHeaderSize += (*it).headerFileSize;
    }

    // Headers
    if ( d->headersCached.size() > 0 ) {
        uint wasted = 100 - (100 * usedHeaderSize/d->headersCached.size() );
        if ( wasted > 30 || (d->headersCached.size() - usedHeaderSize > 15000) ) {
            qLog(Messaging) << "Compacting " << headerfn.toLatin1() << ", used"
                            << d->headersCached.size() << ", needed" << usedHeaderSize;
            compactHeaders();
        }
    }
}

//  This function assumes the data is current.
void EmailFolderAccess::compactHeaders()
{
    QFile f( headerfn );
    if ( isWriteLocked(f) )
        return;

    if ( !f.open(QIODevice::WriteOnly) ) {
        return;
    }

    if ( !lockDataFile(f) ) {
        qWarning("could not obtain lock on file %s, aborting..", (const char *)headerfn.toLatin1() );
        f.close();
        return;
    }

    int pos = 0;

    QMap<QUuid, IndexItem>::Iterator it;
    for(int i = 0; i < _uuidList.count(); i++)
    {
        QUuid id = _uuidList[i];
        it = mappedMails.find(id);
        f.write(d->headersCached.data() + (*it).headerFilePos, (*it).headerFileSize);

        (*it).headerFilePos = pos;

        pos+= (*it).headerFileSize;
    }

    f.close();

    QFileInfo fi( f.fileName() );
    headerDate = fi.lastModified();

    writeIndexFile( indexfn );
    readHeaderFile();
}

bool EmailFolderAccess::recreateIndexFiles()
{
    qWarning("recreating index files for mailbox: %s", _mailbox.toLocal8Bit().constData() ); //no tr

    QDir dir(curDirPath);
    QStringList fileList = dir.entryList(QDir::Files);

    if(QFile::exists(indexfn) && !QFile::remove(indexfn))
    {
        qWarning("unable to remove index file"); //no tr
        return false;
    }

    if(QFile::exists(headerfn) && !QFile::remove(headerfn))
    {
        qWarning("unable to remove header file"); //no tr
        return false;
    }

    mappedMails.clear();
    _uuidList.clear();

    ensureExists(true);

    foreach(QString filename,fileList )
    {
        QString filepath = curDirPath + "/" + filename;
        QFile file(filepath);

        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning("unable to open file %s",qPrintable(filepath)); //no tr
            return false;
        }

        if ( isWriteLocked(file) )
            return false;
        if ( !lockDataFile(file) ) {
            qWarning("could not obtain lock on mail file %s, aborting..",qPrintable(filepath)); //no tr
            return false;
        }

        //TODO convert to mmap or mailmessage using mmap
        QString data = file.readAll();

        //only need the header
        QString headerDelim= "\n\n";
        int begin, end;
        begin = end = 0;
        end = data.indexOf(headerDelim,begin);

        if(end != -1)
        {
            QString header = data.mid(begin,end-begin);
            MailMessage msg(data);
            msg.setUuid( generateUuid() );
            IndexItem index;
            index.mailFile = filename;
            saveHeader(msg,index);
            mappedMails.insert( msg.uuid(), index);
            _uuidList.append(msg.uuid());
            tryWriteIndex();
            unlockDataFile(file);
            file.close();
        }
        else
        {
            qWarning("unable to find header section of file %s, either corrupt or incomplete.",qPrintable(filepath)); //no tr
            qWarning("removing the mail file %s",qPrintable(filepath)); //no tr
            unlockDataFile(file);
            file.close();
            file.remove();
        }
    }
    _status = Open;
    sendMsg("mailboxChanged");

    return true;
}

void EmailFolderAccess::writeDirtyIndex()
{
    _indexWriteSkip = 0;
    if(!writeIndexFile(indexfn))
    {
        qWarning("Writing the index file failed.");
        //attempt to reload the old index file
        readIndexFile();
    }
}

void EmailFolderAccess::tryWriteIndex()
{
    if(++_indexWriteSkip > MAX_WRITESKIP)
        writeDirtyIndex();
    else
        _dirtyIndexTimer.start(); //set to run at the end of each batch
}

bool EmailFolderAccess::moveMail(const QUuid& id, EmailFolderAccess& dest)
{
    return transferMail(id,dest,false,0);
}

bool EmailFolderAccess::copyMail(const QUuid& id, EmailFolderAccess& dest, QUuid& newid)
{
    return transferMail(id,dest,true,&newid);
}

bool EmailFolderAccess::transferMail(const QUuid& id, EmailFolderAccess& dest, bool copy, QUuid* newid)
{
    //move the context information across

    QMap<QUuid, IndexItem>::Iterator ref = mappedMails.find( id );

    if(ref == mappedMails.end())
    {
        qWarning("Cannot move mail that does not exist in folder %s",qPrintable(curDirPath));
        return false;
    }

    IndexItem copyIndex = (*ref);

    MailMessage m = mailContextInfo(id);

    if(copy)
    {
        //create a new uuid for the copied mail
        Q_ASSERT(newid);
        (*newid) = QUuid::createUuid();
        m.setUuid(*newid);
    }

    if(!dest.saveHeader(m,copyIndex))
    {
        qWarning("Unable to copy mail context info across");
        return false;
    }

    //copy or move mail file across
    QString oldPath = curDirPath + "/" + (*ref).mailFile;

    if(copy)
    {
        QString mailFile = dest.createUniqueFileName();
        QString newPath = dest.curDirPath + "/" + mailFile;

    //TODO paranoid file exists check

        if(!QFile::copy(oldPath,newPath))
        {
            qWarning("Unable to copy mail data across");
            return false;
        }

        copyIndex.mailFile = mailFile;
        dest.mappedMails.insert((*newid),copyIndex);
        dest._uuidList.append((*newid));
        dest.tryWriteIndex();
    }
    else
    {
        QString newPath = dest.curDirPath + "/" + copyIndex.mailFile;

    //TODO paranoid file exists check

        if(!QDir().rename(oldPath,newPath))
        {
            qWarning("Unable to move mail data across");
            return false;
        }

         //TODO fugly

        mappedMails.erase(ref);
        _uuidList.removeAll(id);
        dest.mappedMails.insert(id,copyIndex);
        dest._uuidList.append(id);
        tryWriteIndex();
        dest.tryWriteIndex();
    }
    return true;
}

void EmailFolderAccess::ensureSynced()
{
    //Check that the number of files in the mail folder matches the number of index entries.
    //Remove any files from the mail folder that are not contained in the index.

    QDir maildir(curDirPath);
    QStringList files = maildir.entryList(QDir::Files);

    if(files.count() != _uuidList.count())
    {
        QMap<QUuid,IndexItem>::const_iterator it =  mappedMails.begin();
        while(it != mappedMails.end())
        {
            IndexItem currentIndex = *it;
            QString mailFile = curDirPath + "/" + currentIndex.mailFile;
            if(QFile::exists(mailFile))
                files.removeAll(currentIndex.mailFile);
            ++it;
        }
        //remaining files are dangling
        foreach(QString file,files)
        {
            QString mailFile = curDirPath + "/" + file;
            qWarning("removing dangling file %s",qPrintable(mailFile)); //no tr
            if(!QFile::remove(mailFile))
                qWarning("failed to remove dangling file %s",qPrintable(mailFile)); //no tr
        }

    }

    //Remove any unfiled temp mails.

    QDir tmpDir(tmpDirPath);
    QStringList tmpFiles = tmpDir.entryList(QDir::Files);

    foreach(QString tmpMail,tmpFiles)
    {
        QString path = tmpDirPath + "/" + tmpMail;
        qWarning("removing dangling temp file %s",qPrintable(path)); //no tr
        if(!QFile::remove(path))
            qWarning("unable to remove dangling temp file %s",qPrintable(path)); //no tr
    }
}
