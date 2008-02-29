/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3LOCALFS_H
#define Q3LOCALFS_H

#include <Qt3Support/q3networkprotocol.h>
#include <QtCore/qdir.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_NETWORKPROTOCOL

class Q_COMPAT_EXPORT Q3LocalFs : public Q3NetworkProtocol
{
    Q_OBJECT

public:
    Q3LocalFs();
    virtual int supportedOperations() const;

protected:
    virtual void operationListChildren( Q3NetworkOperation *op );
    virtual void operationMkDir( Q3NetworkOperation *op );
    virtual void operationRemove( Q3NetworkOperation *op );
    virtual void operationRename( Q3NetworkOperation *op );
    virtual void operationGet( Q3NetworkOperation *op );
    virtual void operationPut( Q3NetworkOperation *op );

private:
    int calcBlockSize( int totalSize ) const;
    QDir dir;

};

#endif // QT_NO_NETWORKPROTOCOL

QT_END_HEADER

#endif // Q3LOCALFS_H
