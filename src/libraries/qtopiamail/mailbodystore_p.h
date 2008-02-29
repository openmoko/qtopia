/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __MAILBODYSTORE_H
#define __MAILBODYSTORE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>

class QMailMessage;

class MailBodyStore
{
public:
	MailBodyStore();
	~MailBodyStore();
	
	bool insert(const QMailMessage& m, QString* newFile);
	bool remove(const QString& fileName);
	bool update(const QString& fileName, const QMailMessage& data);
	bool load(const QString& fileName, QMailMessage* out) const;

private:
	QString generateUniqueFileName() const;
	QString randomString(int length) const;

private:
	QString _storePath;
};

#endif //__MAILBODYSTORE_H
