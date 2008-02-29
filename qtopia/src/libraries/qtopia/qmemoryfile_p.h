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

#ifndef QMEMORYFILE_P_H
#define QMEMORYFILE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QDawg class in Qtopia
// This header file may change from version to version without notice,
// or even be removed.
//
//


#ifndef QT_H
#include <qglobal.h>
#include <qstring.h>
#endif // QT_H

class QMemoryFileData;

#ifdef QTOPIA_MAKEDLL
#include <qtopia/qpeglobal.h>
class QTOPIA_EXPORT QMemoryFile
#else
class Q_EXPORT QMemoryFile
#endif
{
public:
   
    // Documented in qmemoryfile.cpp 
    enum Flags {
	Write		= 0x00000001,
	Shared		= 0x00000002,
	Create          = 0x00000004,
    };

    QMemoryFile(const QString &fileName, int flags=-1, uint size=0);
    ~QMemoryFile();

    bool isShared() {return (flags & QMemoryFile::Shared) != 0;} 
    bool isWritable() { return (flags & QMemoryFile::Write) != 0;}
    uint size() { return  length;}   
    char *data() { return block; }

private:
    QMemoryFileData *openData (const QString &fileName, int flags, uint size);
    void closeData(QMemoryFileData *memoryFile);
    
protected:
    char* block;
    uint length;
    uint flags;
    QMemoryFileData* d;
};

#endif
