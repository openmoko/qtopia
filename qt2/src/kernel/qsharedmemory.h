/****************************************************************************
** $Id: qt/src/kernel/qsharedmemory.h   2.3.12   edited 2005-10-27 $
**
** Declaration of Qt/Embedded shared memory class
**
** Created : 000101
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. 
// This header file may change from version to version without notice,
// or even be removed.
//
//


#if !defined(QT_NO_QSHM)

#if !defined(QT_QSHM_H)
#define QT_QSHM_H

#ifndef QT_H
#include <qstring.h>
#endif // QT_H

#include <sys/types.h>
#include <sys/ipc.h>

class QSharedMemory {
public:
	QSharedMemory(){};
	QSharedMemory(int, QString);
	~QSharedMemory(){};

	bool create();
	void destroy();

	bool attach();
	void detach();

	void setPermissions(mode_t mode);
	void * base() { return shmBase; };

private:
	void *shmBase;
	int shmSize;
	QString shmFile;
#if defined(QT_POSIX_QSHM)
	int shmFD;
#else
	int shmId;
	key_t key;
	int idInitted;
#endif
};

#endif
#endif
