/****************************************************************************
** $Id: qt/src/tools/qmemoryfile.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of the QMemoryFile class
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

/*****************************************************************************
  QMemoryFile member functions
 *****************************************************************************/

/*!
  \class QMemoryFile qmemoryfile_p.h
  \brief The QMemoryFile class provide means to map a file info memory or refere to a block or memory by name.

  Warning: This class is not part of the Qt APi and subject to change.
             This file file may change from version to version without
             notice, or even be removed.


Currently this
         class supports read only access on all platforms; and read plus write
         accces provided on unix and windows based plaforms.
	 The size of mapped file can not be changed.
*/


/*! \ enum QMemoryFile::Flags
  This enum specifies the possible types of connections made to the file to
  be memory mapped.

   The currently defined values are :

  \value Write  Allow write access to file.
  \value Shared Allow file to be shared with other proceses.
  \value Create Create file named.
 */



/*!
  Construct a memory mapped to an actual file or named memory block.
  If \a fileName is not preceeded by an \\ then it is treated as a real
     file name.
  Otherwise the characters after the \\ are used as the name for the memory block required.
    nb: You will need to escape the \'s so it you need to use \\\\ within a literal string 
  A value for \a size should always be provided, though if the size is not provided it will be determined if possible.

 If a file is being mapped to be written to, then the \a flags of subsequent QMemoryFiles mapped to the same file should include QMemoryFile::Write.

 Example:
 \code
 // Open a maping to file text.dat
 QMemoryFile memoryFile("text.dat", QMemoryFile:Read, 20);
 char *data = memoryFile.data();
 int sum = 0;
 for (int i = 0; i < 20; i++){
   sum = sum + data[i];
 }
 qDebug("Sum =%d", sum);
 \endcode

 Example for creating named memory block:
 \code

 QMemoryFile block1("\\\\block1", QMemoryFile::Create | QMemoryFile::Write, 20);
 char *dataBlock = block.data();
 for (int i = 0; i < 19; i++){
   dataBlock[i] = i +'a';
 }
 dataBlock[20] = '\0';
 qDebug("Data block is %s", dataBlock);
 \endcode


*/
QMemoryFile::QMemoryFile(const QString &fileName, int flags, uint size)
{
    block = NULL;
    length = 0;
    if (flags == -1)
      flags = QMemoryFile::Shared; // read only shared file mapping

    this->flags = flags;
    d = openData(fileName, flags, size);
}

/*!
  Destructs the memory mapped file
*/
QMemoryFile::~QMemoryFile()
{
    closeData(d);
}


/*!
 \fn char* QMemoryFile::data()

 Returns a pointer to memory that this QMemoryFile is associated with
*/


/*!
  \fn bool QMemoryFile::isShared()

  Returns true if memory is shared
*/


/*!
 \fn bool QMemoryFile::isWritable()
   
 Returns true if memory is writable
*/
#endif

