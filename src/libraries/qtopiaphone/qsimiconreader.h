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

#ifndef QSIMICONREADER_H
#define QSIMICONREADER_H

#include <qtopiaglobal.h>
#include <QImage>

class QSimIconReaderPrivate;

class QTOPIAPHONE_EXPORT QSimIconReader : public QObject
{
    Q_OBJECT
public:
    explicit QSimIconReader( const QString& service = QString(), QObject *parent = 0 );
    ~QSimIconReader();

    bool haveIcon( int iconId ) const;
    QImage icon( int iconId ) const;
    void requestIcon( int iconId );

signals:
    void iconAvailable( int iconId );
    void iconNotFound( int iconId );

private slots:
    void indexError();
    void indexFileInfo( int numRecords, int recordSize );
    void indexRead( const QByteArray& data, int recno );
    void emitPendingAvailable();
    void emitPendingNotFound();
    void processPendingOnIndex();
    void iconFetched( int iconId, const QImage& image );
    void iconFetchFailed( int iconId );

private:
    QSimIconReaderPrivate *d;
};

#endif /* QSIMICONREADER_H */
