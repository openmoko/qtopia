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

#ifndef QCONTENTPLUGIN_H
#define QCONTENTPLUGIN_H

#include <qcontent.h>
#include <qtopiaglobal.h>

class QTOPIA_EXPORT QContentPlugin
{
public:
    virtual ~QContentPlugin();

    virtual QStringList keys() const = 0;

    virtual bool installContent( const QString &path, QContent *content ) = 0;
    virtual bool updateContent( QContent *content );
};

class QTOPIA_EXPORT QContentFactory
{
public:
    static bool installContent( const QString &fileName, QContent *content );
    static bool updateContent( QContent *content );
};

Q_DECLARE_INTERFACE( QContentPlugin, "com.trolltech.Qtopia.QContentPlugin/1.0" );

#endif
