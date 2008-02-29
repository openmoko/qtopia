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

#ifndef _EXIFCONTENTPLUGIN_H
#define _EXIFCONTENTPLUGIN_H

#include <qcontentplugin.h>
#include <qtopiaglobal.h>
#include <QDataStream>
#include <QFile>
#include <QList>
#include "ifd.h"

class QTOPIA_PLUGIN_EXPORT ExifContentPlugin : public QObject, public QContentPlugin
{
    Q_OBJECT
    Q_INTERFACES(QContentPlugin)
public:
    ExifContentPlugin();
    ~ExifContentPlugin();

    virtual QStringList keys() const;
    virtual bool installContent( const QString &filePath, QContent *content );

private:
    bool readFile( QFile *file, QContent *content );
    QList< Ifd::Header > readIfdHeaders( QDataStream &stream, int baseOffset );
    void readProperties( QDataStream &stream, int baseOffset, QContent *content );

};


#endif
