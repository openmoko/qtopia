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

#ifndef __QMAILFOLDERKEYPRIVATE_H
#define __QMAILFOLDERKEYPRIVATE_H

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

class QMailFolderKeyPrivate : public QSharedData
{
public:
    enum LogicalOp
    {
        None,
        And,
        Or
    };

    class Argument
    {
    public:
		QMailFolderKey::Property property;
        QMailFolderKey::Operand op; 
        QVariant value;
        bool operator==(const Argument& other) const
        {
            return property == other.property &&
                   op == other.op &&
                   value == other.value;
        }   
    };

public:
    QMailFolderKeyPrivate() : QSharedData() {};

    LogicalOp logicalOp;
    bool negated;
    QList<Argument> arguments;
    QList<QMailFolderKey> subKeys;
};



#endif
