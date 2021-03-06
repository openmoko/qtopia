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
#ifndef TARGZ_H
#define TARGZ_H

#include <QString>
#include <libtar.h>

bool targz_extract_all( const QString &tarfile, const QString &destpath = QString(), bool verbose = true );
bool targz_archive_all( const QString &tarfile, const QString &srcpath, bool gzip = true, bool verbose = true );

TAR * get_tar_ptr( const QString &tarfile );
qlonglong targz_archive_size( const QString &tarfile );
bool check_tar_valid( const QString &tarfile );
#endif
