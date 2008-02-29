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

#include "qmemoryfile_p.h"
#include <windows.h>
#include <winbase.h>
#include <qdir.h>
#include <qfile.h>
#include <stdio.h>

class QMemoryFileData
{
public:
    QMemoryFileData(HANDLE hFile, HANDLE hMapeFile, LPVOID pMapFile);
    ~QMemoryFileData();
    operator char*() { return (char*) pMapFile;}

private:
    HANDLE hFile, hMapFile;
    LPVOID pMapFile;
};

/*!
  Contructs the memory map data
 */
QMemoryFileData::QMemoryFileData(HANDLE hFile, HANDLE hMapFile, LPVOID pMapFile)
{
    this->hFile = hFile;
    this->hMapFile = hMapFile;
    this->pMapFile = pMapFile;
}


/*!
  Destructs the memory map data
 */
QMemoryFileData::~QMemoryFileData()
{
    if (pMapFile != NULL){
        UnmapViewOfFile(pMapFile);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
    }
}


/*
 As this function is in both _unix and _win, its documentation is in qmemoryfile.cpp
 */
QMemoryFileData * QMemoryFile::openData (const QString &fileName, int flags,
                                         uint size )
{
    HANDLE hFile =(HANDLE)INVALID_HANDLE_VALUE, hMapFile = NULL;
    DWORD dwCreationDisposition = OPEN_EXISTING;
    LPVOID pMapFile = NULL;
    DWORD fileSize;
    QMemoryFileData *data = NULL;
    DWORD dwDesiredAccess;
    DWORD dwShareMode;
    DWORD dwDesiredMapAccess;
    DWORD dwPageAccess;
    QString memoryName = QString();

    // Do we have a named memory map
    if (fileName.left(2) == "\\\\"){
        flags |= QMemoryFile::Shared;
        memoryName = fileName.mid(2);
    }
    fileSize = size;

    if (flags & QMemoryFile::Write){
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
        dwDesiredMapAccess = FILE_MAP_WRITE;
        if ((flags & QMemoryFile::Shared) != 0)
            dwPageAccess = PAGE_READWRITE;
        else{
            dwPageAccess = PAGE_WRITECOPY;
            dwDesiredMapAccess = FILE_MAP_COPY;
        }
    }else{
        dwDesiredAccess = GENERIC_READ;
        dwShareMode = FILE_SHARE_READ;
        dwDesiredMapAccess = FILE_MAP_READ;
        dwPageAccess = PAGE_READONLY;
    }
    if (!memoryName.isNull()){
        if (size == 0){
            qWarning("Requested operation on named memory map without " \
                     "supplying a size of memory to map ");
            return data;
      }
      if (flags & QMemoryFile::Create){
          if (hMapFile == NULL){
              QT_WA ( {
                      hMapFile = CreateFileMappingW(hFile,
                          NULL,
                          dwPageAccess,
                          0,
                          fileSize,
                          (const TCHAR*)memoryName.utf16());
                      }, {
                      hMapFile = CreateFileMappingA(hFile,
                          NULL,
                          dwPageAccess,
                          0,
                          fileSize,
                          memoryName.toLocal8Bit());
                      });
          }
      }else{
          QT_WA( {
                  hMapFile = OpenFileMappingW(dwDesiredAccess, true,
                      (const TCHAR*)memoryName.utf16());
                  }, {
                  hMapFile = OpenFileMappingA(dwDesiredAccess, true,
                      memoryName.toLocal8Bit());
                  });
      }
    }else{
        const QString convertedName = QDir::convertSeparators(fileName);
        bool fillFile = false;
        if  (flags & QMemoryFile::Create){
            if (size == 0){
                qWarning("Requested to create memory file without "
                         "supplying a size of memory to map ");
                return data;
            }
            QDir d;
            if (!d.exists(convertedName)){
                dwCreationDisposition = CREATE_NEW;
                // fill file with zeros
                fillFile = true;
            }else
                dwCreationDisposition = OPEN_EXISTING;
        }else {
            dwCreationDisposition = OPEN_EXISTING;
            QT_WA ( {
                    hFile = CreateFileW( (const TCHAR*)convertedName.utf16(),
                        dwDesiredAccess,
                        dwShareMode,
                        NULL,
                        dwCreationDisposition,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
                    }, {
                    hFile = CreateFileA(convertedName.toLocal8Bit(),
                        dwDesiredAccess,
                        dwShareMode,
                        NULL,
                        dwCreationDisposition,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
                    });
            }
        if (hFile != (HANDLE)INVALID_HANDLE_VALUE){
            if (fillFile){
                if ((SetFilePointer(hFile, size, NULL, FILE_BEGIN) !=   0xFFFFFFFF) &&
                        WriteFile(hFile, "",  1L, NULL, NULL))
                    fileSize = size;
                else
                    fileSize = 0xFFFFFFFF;
            }else
                fileSize = GetFileSize(hFile, NULL);
            if (fileSize != 0xFFFFFFFF){
                QT_WA( {
                        hMapFile = CreateFileMapping(hFile,
                            NULL,
                            dwPageAccess,
                            0,
                            fileSize,
                            NULL);
                        }, {
                        hMapFile = CreateFileMappingA(hFile,
                            NULL,
                            dwPageAccess,
                            0,
                            fileSize,
                            NULL);
                        });
            }
        }else{
            if (memoryName.isNull())
                qWarning("QMemory File unable to open real file %s",convertedName.toLatin1());
            else
                qwarning("QMemory File unable to open memory block named %s",memoryName.toLatin1());
        }
    }
    if (hMapFile != NULL){
        pMapFile = MapViewOfFile(hMapFile, dwDesiredMapAccess, 0, 0, 0);
        if (pMapFile == NULL){
            qWarning("Memory map failed (Map of view : error number %d )", GetLastError());
            CloseHandle(hMapFile);
            hMapFile = NULL;
            CloseHandle(hFile);
            hFile = NULL;
        }else{
            data = new QMemoryFileData(hFile, hMapFile, pMapFile);
            this->flags = flags;
            this->length = fileSize;
        }
    }else{
        qWarning("Memory map failed (Create/Open file : error number %d", GetLastError());
    }

    if (data == NULL)
        qWarning("Failed to mmap %s", fileName.toLatin1());
    else
        block = *data;

    return data;
}

/*
  \internal
 As this function is in both _unix and _win, its documentation is in qmemoryfile.cpp
 */
void QMemoryFile::closeData(QMemoryFileData *memoryFile)
{
    delete memoryFile;
}
