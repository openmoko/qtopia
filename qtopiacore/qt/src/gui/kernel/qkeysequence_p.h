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

#ifndef QKEYSEQUENCE_P_H
#define QKEYSEQUENCE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qkeysequence.h"

#ifndef QT_NO_SHORTCUT
struct Q_AUTOTEST_EXPORT QKeyBinding
{
    QKeySequence::StandardKey standardKey;
    uchar priority;
    uint shortcut;
    uint platform;
};

class Q_AUTOTEST_EXPORT QKeySequencePrivate
{
public:
    inline QKeySequencePrivate()
    {
        ref = 1;
        key[0] = key[1] = key[2] = key[3] =  0;
    }
    inline QKeySequencePrivate(const QKeySequencePrivate &copy)
    {
        ref = 1;
        key[0] = copy.key[0];
        key[1] = copy.key[1];
        key[2] = copy.key[2];
        key[3] = copy.key[3];
    }
    QAtomic ref;
    int key[4];
    static QString encodeString(int key, QKeySequence::SequenceFormat format);
    static int decodeString(const QString &keyStr, QKeySequence::SequenceFormat format);

    static const QKeyBinding keyBindings[];
    static const uint numberOfKeyBindings;
    
};
#endif // QT_NO_SHORTCUT

#endif //QKEYSEQUENCE_P_H
