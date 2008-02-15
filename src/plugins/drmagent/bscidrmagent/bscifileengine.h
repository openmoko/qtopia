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

#ifndef BSCIFILEENGINE_H
#define BSCITFILEENGINE_H

#include <qdrmcontentplugin.h>
#include <QFSFileEngine>

class BSciDrmFileEngine : public QFSFileEngine
{
public:
    BSciDrmFileEngine( QDrmContentPlugin *plugin, const QString &filePath );
    BSciDrmFileEngine( QDrmContentPlugin *plugin, const QString &filePath, const QString &dcfPath, const QString &baseName );
    virtual ~BSciDrmFileEngine();

    virtual bool close();

    virtual bool copy( const QString & newName );

    virtual QStringList entryList( QDir::Filters filters, const QStringList &filterNames ) const;

    virtual FileFlags fileFlags( FileFlags type = FileInfoAll ) const;

    virtual QString fileName( FileName file = DefaultName ) const;

    virtual bool mkdir( const QString & dirName, bool createParentDirectories ) const;

    virtual bool open( QIODevice::OpenMode mode );

    virtual qint64 pos() const;

    virtual qint64 read( char * data, qint64 maxlen );

    virtual bool seek ( qint64 offset );

    virtual bool setPermissions( uint permissions );

    virtual bool setSize( qint64 size );

    virtual qint64 size() const;

    virtual qint64 write( const char *data, qint64 len );

protected:
    virtual QString baseName() const;

    virtual QString defaultName() const;

    virtual QString contentId() const;

    virtual QStringList contentIds() const;

    virtual bool isMultipart() const;

    virtual bool isLink() const;

private:
    QDrmContentPlugin *m_plugin;
    QString m_filePath;
    QString m_baseName;
    QIODevice *m_io;
};

#endif
