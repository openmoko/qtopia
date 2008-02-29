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

#ifndef QSIMICONREADER_P_H
#define QSIMICONREADER_P_H

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

#include <qsimiconreader.h>
#include <qbinarysimfile.h>

class QSimMonoIconReader : public QObject
{
    Q_OBJECT
public:
    QSimMonoIconReader( const QString& service, int iconId,
                        const QString& fileid, int offset,
                        int length, QObject *parent = 0 );
    ~QSimMonoIconReader();

signals:
    void iconFetched( int iconId, const QImage& image );
    void iconFetchFailed( int iconId );

private slots:
    void error();
    void readDone( const QByteArray& data );

private:
    int iconId;
    int length;
    QBinarySimFile *file;
    QByteArray contents;
    bool errorReported;
};

class QSimColorIconReader : public QObject
{
    Q_OBJECT
public:
    QSimColorIconReader( const QString& service, int iconId,
                         const QString& fileid, int offset,
                         int length, QObject *parent = 0 );
    ~QSimColorIconReader();

signals:
    void iconFetched( int iconId, const QImage& image );
    void iconFetchFailed( int iconId );

private slots:
    void error();
    void readDone( const QByteArray& data );

private:
    int iconId;
    int length;
    int clutLength;
    QBinarySimFile *file;
    QByteArray contents;
    QByteArray clut;
    bool errorReported;

    void read( int offset, int length );
    static int fetchPixel( const char *data, int start, int size );
};

#endif /* QSIMICONREADER_P_H */
