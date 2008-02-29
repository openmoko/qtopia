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

#ifndef DBRESTOREOP_H
#define DBRESTOREOP_H

#include "dboperation.h"

class DbRestoreOperation : public DbOperation
{
    Q_OBJECT
public:
    virtual ~DbRestoreOperation();
    void restoreToSystem();

private:
    DbRestoreOperation( QtopiaDatabaseId id, QObject *parent );

    QtopiaDatabaseId dbId;

    friend DbRestoreOperation *DbOperation::restoreOp( const QString &srcDb, QObject *parent );
};

#endif
