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

#ifndef QMODEMSIMFILES_H
#define QMODEMSIMFILES_H

#include <qsimfiles.h>

class QModemService;
class QModemSimFilesPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemSimFiles : public QSimFiles
{
    Q_OBJECT
public:
    explicit QModemSimFiles( QModemService *service );
    ~QModemSimFiles();

public slots:
    void requestFileInfo( const QString& reqid, const QString& fileid );
    void readBinary( const QString& reqid, const QString& fileid,
                     int pos, int len );
    void writeBinary( const QString& reqid, const QString& fileid,
                      int pos, const QByteArray& data );
    void readRecord( const QString& reqid, const QString& fileid,
                     int recno, int recordSize );
    void writeRecord( const QString& reqid, const QString& fileid,
                      int recno, const QByteArray& data );

protected:
    virtual bool useCSIM() const;

private:
    QModemSimFilesPrivate *d;
};

#endif /* QMODEMSIMFILES_H */
