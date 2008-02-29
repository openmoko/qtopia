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

#ifndef _ID3CONTENTPLUGIN_H
#define _ID3CONTENTPLUGIN_H

#include <qcontentplugin.h>
#include <qtopiaglobal.h>

class Id3Tag;

class QTOPIA_PLUGIN_EXPORT Id3ContentPlugin : public QObject, public QContentPlugin
{
    Q_OBJECT
    Q_INTERFACES(QContentPlugin)
public:
    Id3ContentPlugin();
    ~Id3ContentPlugin();

    virtual QStringList keys() const;
    virtual bool installContent( const QString &filePath, QContent *content );
    virtual bool updateContent( QContent *content );

private:
    QString name( const QString &filePath, Id3Tag &tag ) const;

    QString string( Id3Tag &tag, qint64 framePosition ) const;
    QString genre( Id3Tag &tag, qint64 framePosition ) const;

};

#endif
