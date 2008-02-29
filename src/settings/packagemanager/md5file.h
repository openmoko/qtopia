/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef MD5FILE_H
#define MD5FILE_H

#include <md5.h>
#include <QFile>


class Md5File : public QFile
{
public:
    Md5File( QObject *parent = 0 );
    Md5File( const QString &fileName, QObject *parent = 0 );

    QString md5Sum() const;

    virtual bool open( OpenMode mode );
    virtual void close();

protected:
    virtual qint64 writeData( const char *data, qint64 maxSize );

private:
    MD5Context m_context;
    QString m_md5Sum;
};

#endif
