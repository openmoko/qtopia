/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIODEVICE_P_H
#define QIODEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QIODevice. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qiodevice.h"
#include "QtCore/qbytearray.h"
#include "QtCore/qobjectdefs.h"
#include "QtCore/qstring.h"
#include "private/qringbuffer_p.h"
#ifndef QT_NO_QOBJECT
#include "private/qobject_p.h"
#endif

class Q_CORE_EXPORT QIODevicePrivate
#ifndef QT_NO_QOBJECT
    : public QObjectPrivate
#endif
{
    Q_DECLARE_PUBLIC(QIODevice)

public:
    QIODevicePrivate();
    virtual ~QIODevicePrivate();

    QIODevice::OpenMode openMode;
    QString errorString;

    QRingBuffer buffer;
    qint64 pos;
    qint64 devicePos;
    bool baseReadLineDataCalled;

    virtual bool putCharHelper(char c);

    enum AccessMode {
        Unset,
        Sequential,
        RandomAccess
    };
    mutable AccessMode accessMode;
    inline bool isSequential() const
    {
        if (accessMode == Unset)
            accessMode = q_func()->isSequential() ? Sequential : RandomAccess;
        return accessMode == Sequential;
    }
    

#ifdef QT_NO_QOBJECT
    QIODevice *q_ptr;
#endif
};

#endif // QIODEVICE_P_H
