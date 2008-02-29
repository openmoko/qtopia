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

#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#include "qmemoryfile_p.h"
#include "qfile.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <errno.h>
#include <qtopia/global.h>

class QMemoryFileData
{ 
public:
    QMemoryFileData(int fd, char* data, uint length);
    QMemoryFileData(int shmId, char* data, bool shmCreator, uint length);
    ~QMemoryFileData();
    operator char*() { return data;}

private:
    int fd;
    char* data;
    uint length;
    int shmId;
    bool shmCreator;
};


/*!
  Constructs the memory file data 
 */
QMemoryFileData::QMemoryFileData(int fd, char* data, uint length)
{
    this->fd = fd;
    this->data = data;
    this->length = length;
    shmCreator = FALSE;
    shmId = -1;
}


/*!
  Constructs the memory file data 
 */
QMemoryFileData::QMemoryFileData(int shmId, char* data, bool shmCreator, 
				 uint length)
{
    this->shmId = shmId;   
    this->data = data;
    this->length = length;
    this->shmCreator = shmCreator;
}


/*!
  Destructs the memory file data 
*/
QMemoryFileData::~QMemoryFileData()
{
    
    if (data != NULL){
	if (shmId == -1){
	    munmap(data, length);
	}else{
	    // unattach and free the shared memory if needed
	    if (shmdt(data) != -1){
		if (shmCreator == TRUE){
		    if (shmctl(shmId, IPC_RMID, 0) == -1)
			qDebug("QMemoryFile unable to free shared memory");
		}
	    }else
		qDebug("Unable to unattatch QMemoryFile from shared memory");
	}	
    }
}


/*
 As this function is in both _unix and _win, its documentation is in qmemoryfile.cpp
 */
QMemoryFileData * QMemoryFile::openData (const QString &fileName, int flags, 
					 uint size )
{
    QMemoryFileData *data = NULL;
    struct stat st;
    int fileMode ;
    int f;
    uint protFlags;
    uint memFlags;
    int shmId = -1, shmAtFlag, shmGetFlag;
    key_t shmKey = 0;

    if (fileName[0] == '\\'){
	// We have a named memory map
	QString memoryName = fileName.mid(1);
	QString tmpFile(Global::tempDir());
	tmpFile.append(memoryName).append(".txt");
	int f = ::open(tmpFile.latin1(), O_WRONLY);

	if (!f)
	    f = ::open(tmpFile.latin1(), O_CREAT | O_WRONLY);	
	
	if (f){
	    fstat(f, &st);
	    shmKey = st.st_ino;
	    qDebug("QMemoryfile using shm key: %i", shmKey);
	    ::close(f);
	}else{
	    qWarning(QString("QMemoryFile result: ") + strerror(errno));
	    qWarning("QMemoryfile: Unable to create shared key via id file");
	    return data;
	}

	if (size == 0){
	  qDebug("QMemoryFile: No size specified");
	}	      

	if (size && shmKey){
	    flags |= QMemoryFile::Shared;

	    if (flags & QMemoryFile::Write){
		shmGetFlag = 0666;
		shmAtFlag = 0;
	    }  else{	    
		shmGetFlag = 0444;
		shmAtFlag = SHM_RDONLY;
	    }

	    bool shmCreator = FALSE;
	    shmId =  shmget(shmKey, size, shmGetFlag);
	    if (shmId == -1){
		if (flags & QMemoryFile::Create){
		    // Create a block of shared memory
		    shmCreator = TRUE;
		    shmId = shmget(shmKey, size, IPC_CREAT | shmGetFlag);
		    if (shmId != -1)
			block = (char*)shmat(shmId, NULL, shmAtFlag );
		    else
		      qDebug("QMemoryFile error: %s", strerror(errno));
		}else
		    qDebug("QMemoryFile: No such named memory created : %s", 
			   (char*)fileName.latin1());
	    }else{
		// attach to previously created shared memory
		block = (char*)shmat(shmId, NULL, shmAtFlag );
		if (block == (void*)-1)
		  qWarning(QString("QMemoryFile : ") + strerror(errno));
	    }
	    
	    if (block != NULL){
		this->flags = flags;
		this->length = size;
		data = new QMemoryFileData(shmId, block, shmCreator, size);
		qDebug("Created QMemoryFile for %s, id:%d, size:%d",
		       (char*)fileName.latin1(), shmId, size);
	    }else
		qWarning("QMemoryFile: Failed to get shared memory");
	}

    }else{
	// We are mapping a real file
	if ((flags & QMemoryFile::Shared) == 0)
	    memFlags = MAP_PRIVATE;
	else
	    memFlags = MAP_SHARED;

	if (flags & QMemoryFile::Write){
	    fileMode = O_RDWR;
	    protFlags = PROT_READ | PROT_WRITE;
	}else{
	    fileMode = O_RDONLY;
	    protFlags = PROT_READ; 
	}

	if (size == 0){
	    f = ::open(fileName.local8Bit(), fileMode);
	    if ( fstat( f, &st ) == 0 )
		size = st.st_size;
	
	    if (size == 0){
		qWarning("QMemoryFile: No size not specified nor" \
		       " able to determined from file to be mapped");
		::close(f);
	    }
	}else{
	    f = ::open(fileName.local8Bit(), fileMode);	  
	    if ((f == -1) && (flags & QMemoryFile::Create)){
		// create an empty file with a zero at the end
		f = ::open(fileName.local8Bit(), fileMode | O_CREAT, 00644);

		if ((::lseek(f, size, SEEK_SET) == -1) || (::write(f, "", 1) == -1)){
		  qWarning(QString("QMemoryFile result: ") + strerror(errno));
		  qWarning("QMemoryFile: Unable to initialize new file");
		}else
		  lseek(f, 0L, SEEK_SET);
	    }
	}

	if (size != 0){

#if !defined(_OS_SOLARIS_)
	    memFlags |= MAP_FILE; // swap-backed map from file
#endif
	    block = (char*)mmap( 0, // any address
				 size,
				 protFlags,
				 memFlags,
				 f, 0 ); // from offset of 0 of f
	    if ( !block || block == (char*)MAP_FAILED ){
		qWarning("QMemoryFile: Failed to mmap %s", fileName.latin1());
		block = NULL;
	    }else{
		this->flags = flags;
		this->length = size;
		data = new QMemoryFileData(f, block, this->length);
		qDebug("Created QMemoryfile for %s with a size of %d", 
                         fileName.latin1(), size);
	    }
	    ::close(f);
	}
    }

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
