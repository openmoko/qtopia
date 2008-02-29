/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef DBOPERATION_H
#define DBOPERATION_H

#include <QObject>
#include <qcontent.h>

#include "schema.h"

class DbRestoreOperation;

class DbOperation : public QObject
{
    Q_OBJECT
public:
    static DbOperation *factory();

    DbRestoreOperation *restoreOp( const QString &srcDb, QObject *parent = 0 );

    const DbSchema *schemaForDb( QtopiaDatabaseId db ) const;
protected:
    DbOperation( QObject *parent = 0 );
    virtual ~DbOperation();
private:
    void extractSchema( QtopiaDatabaseId db );
    void extractSystemSchema();
    void connectToSystemDatabase();

    QHash<QtopiaDatabaseId,DbSchema*> schemaList;
};

inline const DbSchema *DbOperation::schemaForDb( QtopiaDatabaseId db ) const
{
    return schemaList[db];
}

#endif
