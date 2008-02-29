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

#ifndef __QMAILFOLDER_H
#define __QMAILFOLDER_H

#include "qmailid.h"
#include <QString>
#include <QList>
#include <QSharedData>
#include <qtopiaglobal.h>

class QMailFolderPrivate;

class QTOPIAMAIL_EXPORT QMailFolder 
{
public:
	QMailFolder(const QMailId& id);
    QMailFolder(const QString& name, const QMailId& parentFolderId = QMailId());
    QMailFolder(const QMailFolder& other);
    virtual ~QMailFolder();
    
    QMailId id() const;
    
    QString name() const;
    void setName(const QString& name);
    
    QMailId parentId() const;   
    void setParentId(const QMailId& id); 
    
    bool isRoot() const;
    
    QMailFolder& operator=(const QMailFolder& other);  
 
private:
	QMailFolder();
    void setId(const QMailId& id);

private:
	friend class QMailStore;
	friend class QMailStorePrivate;

private: 
    QSharedDataPointer<QMailFolderPrivate> d;

};

typedef QList<QMailFolder> QMailFolderList;

#endif //QMAILFOLDER_H

