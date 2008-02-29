/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef LONGSTREAM_H
#define LONGSTREAM_H

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

#include <QString>
#include <QtopiaApplication>

class QTemporaryFile;
class QTextStream;

class QTOPIAMAIL_EXPORT LongStream
{
public:
    LongStream();
    virtual ~LongStream() ;
    void reset();
    void append(QString str);
    int length();
    QString fileName();
    QString readLine();
    QString first();
    QString next();
    QString current();

    enum Status { Ok, OutOfSpace };
    Status status();
    void resetStatus();
    void setStatus( Status );
    void updateStatus();
    static bool freeSpace( const QString &path = QString::null, int min = -1);

    static QString errorMessage( const QString &prefix = QString::null );
    static QString tempDir();
    static void cleanupTempFiles();

private:
    QTemporaryFile *tmpFile;
    QTextStream *ts;
    QString lastLine;
    qint64 pos;
    QChar c;
    int len;
    Status mStatus;
    static const unsigned long long minFree = 1024*100;
};
#endif
