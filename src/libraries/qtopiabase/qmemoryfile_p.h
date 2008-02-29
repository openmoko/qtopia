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


#include <qglobal.h>
#include <qstring.h>
#include <qtopiaglobal.h>

class QMemoryFileData;

class QTOPIA_AUTOTEST_EXPORT QMemoryFile
{
public:

    // Documented in qmemoryfile.cpp
    enum Flags {
        Write           = 0x00000001,
        Shared          = 0x00000002,
        Create          = 0x00000004,
    };

    explicit QMemoryFile(const QString &fileName, int flags=-1, uint size=0);
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
