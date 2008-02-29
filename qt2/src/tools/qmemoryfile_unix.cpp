/****************************************************************************
** $Id: qt/src/tools/qmemoryfile_unix.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QMemoryFile to suit the unix platform
**
** Created : 020813
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include <qglobal.h>
#ifndef QT_NO_QMEMORYFILE

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


/*!
  Initialize the memory map returning non-null if sucessful
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
    bool shmCreator = FALSE;
    key_t shmKey = 0;

    if (fileName.left(2) == "\\\\"){
	// We have a named memory map
	QString memoryName = fileName.mid(2);
	QString tmpFile("/tmp/");
	tmpFile.append(memoryName).append(".txt");
	int f = ::open(tmpFile.latin1(), O_WRONLY);

	if (!f)
	    f = ::open(tmpFile.latin1(), O_CREAT | O_WRONLY);	
	
	if (f){
	    fstat(f, &st);
	    shmKey = st.st_ino;
	    qDebug(QString("QMemoryfile using shm key :") + 
		   QString::number(shmKey));
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

	    shmId =  shmget(shmKey, size, shmGetFlag);
	    if (shmId == -1){
		if (flags & QMemoryFile::Create){
		    // Create a block of shared memory
		    shmCreator = TRUE;
		    shmId = shmget(shmKey, size, IPC_CREAT | shmGetFlag);
		    if (shmId != -1)
			block = (char*)shmat(shmId, NULL, shmAtFlag );
		    else
		      qDebug(QString("QMemoryFile error: ") + strerror(errno));
		}else
		    qDebug("QMemoryFile: No such named memory created : %s", 
			   (char*)fileName.latin1());
	    }else{
		// attach to previously created shared memory
		block = (char*)shmat(shmId, NULL, shmAtFlag );
		shmCreator = FALSE;
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


/*!
  \internal
  Close memory file and free any memory used
 */
void QMemoryFile::closeData(QMemoryFileData *memoryFile)
{
    delete memoryFile;
}

#endif // QT_NO_QMEMORYFILE
