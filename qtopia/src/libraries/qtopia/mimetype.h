/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MIMETYPE_H
#define MIMETYPE_H

#include <qtopia/qpeglobal.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qlist.h>

class AppLnk;
class DocLnk;
class QStringList;
class MimeTypeData;
class AppLnkSet;

class QTOPIA_EXPORT MimeType
{
public:
    MimeType( const QString& ext_or_id );
    MimeType( const DocLnk& );

    QString id() const;
    QString description() const;
    QPixmap pixmap() const;
    QPixmap bigPixmap() const;

// DON'T define this yourself!
#ifdef QTOPIA_INTERNAL_MIMEEXT
    QString extension() const;
    QStringList extensions() const;
    QList<AppLnk> applications() const;
#endif

    const AppLnk* application() const;

    static QString appsFolderName();
    static void updateApplications();

    // These shouldn't be needed
    static void clear();
    static void registerApp( const AppLnk& );
#ifdef QTOPIA_INTERNAL_MIMEEXT
    static bool hasAppRegistered( const QString &mimetype );
#endif // QTOPIA_INTERNAL_MIMEEXT

private:
    static void updateApplications(AppLnkSet* folder);
    static void loadExtensions();
    static void loadExtensions(const QString&);
    void init( const QString& ext_or_id );
    class Private;
    static Private* d;
    static Private& data();
    static MimeTypeData* data(const QString& id);
    QString i;
};

#endif
