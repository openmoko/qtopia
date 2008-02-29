/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTFILEENGINE_P_H
#define QABSTRACTFILEENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qabstractfileengine.h"
#include "QtCore/qfile.h"

class QAbstractFileEnginePrivate
{
public:
    inline QAbstractFileEnginePrivate()
        : fileError(QFile::UnspecifiedError)
    {
    }
    inline virtual ~QAbstractFileEnginePrivate() { }

    QFile::FileError fileError;
    QString errorString;

    QAbstractFileEngine *q_ptr;
    Q_DECLARE_PUBLIC(QAbstractFileEngine)
};

#endif // QABSTRACTFILEENGINE_P_H
