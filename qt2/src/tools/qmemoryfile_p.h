/****************************************************************************
** $Id: qt/src/tools/qmemoryfile_p.h   2.3.12   edited 2005-10-27 $
**
** Definition of QMemoryFileWin class
**
** Created : 020813
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
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

#ifndef QT_NO_QMEMORYFILE
class QMemoryFileData;

class QMemoryFile
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

#endif // QT_NO_QMEMORYFILE
#endif
