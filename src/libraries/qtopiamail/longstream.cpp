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
#include <longstream.h>
#include <QApplication>
#include <QIODevice>
#include <QTextStream>
#include <QTemporaryFile>
#ifdef Q_OS_LINUX
#include <sys/vfs.h>
#endif

/*  Helper class to reduce memory usage while downloading large mails */
LongStream::LongStream()
{
    lastLine = QString::null;
    QString tmpName( LongStream::tempDir() + QLatin1String( "/qtmail" ) );
    tmpFile = new QTemporaryFile( tmpName + QLatin1String( ".XXXXXX" ));
    tmpFile->open(); // todo error checking
    ts = new QTextStream( tmpFile );
    ts->setCodec( "UTF-8" ); // Mail should be 7bit ascii
    len = 0;
}

LongStream::~LongStream()
{
    tmpFile->close();
    delete ts;
    delete tmpFile;
}

void LongStream::reset()
{
    lastLine = QString::null;
    ts->seek( 0 );
    tmpFile->resize( 0 );
    len = 0;
    resetStatus();
}

void LongStream::append(QString str)
{
    *ts << str << flush; //todo error checking - out of disk
    len += str.length();
    updateStatus();
}

int LongStream::length()
{
    return len;
}

QString LongStream::fileName()
{
    return tmpFile->fileName();
}

QString LongStream::readLine()
{
    QString s;

    if (!c.isNull() && (c != '\r') && (c != '\n'))
        s += c;
    while (!ts->atEnd() && (c != '\r') && (c != '\n')) {
        *ts >> c;
        if ((c == '\r') || (c == '\n'))
            break;
        s += c;
    }
    if ((!ts->atEnd()) && (c == '\r')) {
        *ts >> c;
        if (c == '\n')
            *ts >> c;
    } else if ((!ts->atEnd()) && (c == '\n')) {
        *ts >> c;
        if (c == '\r')
            *ts >> c;
    }
    if (s.isNull() && !ts->atEnd())
        return "";
    return s;
}

QString LongStream::first()
{
    ts->seek( 0 );
    // todo uncomment when QTextStream::readLine is memory efficient
//  lastLine = ts->readLine();
    lastLine = readLine();
    if (!lastLine.isEmpty())
        lastLine += "\n";
    return lastLine;
}

QString LongStream::next()
{
    // todo uncomment when QTextStream::readLine is memory efficient
//  lastLine = ts->readLine();
    lastLine = readLine();
    if (!lastLine.isNull())
        lastLine += "\n";
    return lastLine;
}

QString LongStream::current()
{
    return lastLine;
}

LongStream::Status LongStream::status()
{
    return mStatus;
}

void LongStream::resetStatus()
{
    mStatus = Ok;
}

void LongStream::updateStatus()
{
    if (!freeSpace())
        setStatus( LongStream::OutOfSpace );
}

void LongStream::setStatus( Status status )
{
    mStatus = status;
}

bool LongStream::freeSpace( const QString &path, int min)
{
#ifdef Q_OS_LINUX
    unsigned long long boundary = minFree;
    if (min >= 0)
        boundary = min;
    struct statfs stats;
    QString partitionPath = tempDir() + "/.";
    if (!path.isEmpty())
        partitionPath = path;

    statfs( QString( partitionPath ).toLocal8Bit(), &stats);
    unsigned long long bavail = ((unsigned long long)stats.f_bavail);
    unsigned long long bsize = ((unsigned long long)stats.f_bsize);
    return (bavail * bsize) > boundary;
#else
    return true;
#endif
}

QString LongStream::errorMessage( const QString &prefix )
{
    QString str = QApplication::tr( "Storage for messages is full. Some new "
                                    "messages could not be retrieved." );
    if (!prefix.isEmpty())
        return prefix + str;
    return str;
}

QString LongStream::tempDir()
{
    QString path( QDir::homePath() + "/Applications/qtmail/temp/" );
    if (path.isEmpty())
        path = Qtopia::tempDir();
    QDir dir;
    if (!dir.exists( path ))
        dir.mkpath( path );
    return path;
}

void LongStream::cleanupTempFiles()
{
    QDir dir( LongStream::tempDir(), "qtmail.*" );
    QStringList list = dir.entryList();
    for (int i = 0; i < list.size(); ++i) {
        QFile file( LongStream::tempDir() + list.at(i) );
        if (file.exists())
            file.remove();
    }
}
