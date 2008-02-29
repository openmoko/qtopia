/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QEVENT_P_H
#define QEVENT_P_H

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

// ### Remove for Qt 5
class Q_GUI_EXPORT QKeyEventEx : public QKeyEvent
{
public:
    QKeyEventEx(Type type, int key, Qt::KeyboardModifiers modifiers,
                const QString &text, bool autorep, ushort count,
                quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers);
    QKeyEventEx(const QKeyEventEx &other);

    ~QKeyEventEx();

protected:
    quint32 nScanCode;
    quint32 nVirtualKey;
    quint32 nModifiers;
    friend class QKeyEvent;
};

#endif // QEVENT_P_H
