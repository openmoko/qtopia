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

#ifndef QTEXTOBJECT_P_H
#define QTEXTOBJECT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qtextobject.h"
#include "private/qobject_p.h"

class QTextDocumentPrivate;

class QTextObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextObject)
public:
    QTextDocumentPrivate *pieceTable;
    int objectIndex;
};

class QTextBlockGroupPrivate : public QTextObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextBlockGroup)
public:

    typedef QList<QTextBlock> BlockList;
    BlockList blocks;
};

class QTextFrameLayoutData;

class QTextFramePrivate : public QTextObjectPrivate
{
    friend class QTextDocumentPrivate;
    Q_DECLARE_PUBLIC(QTextFrame)
public:

    virtual void fragmentAdded(const QChar &type, uint fragment);
    virtual void fragmentRemoved(const QChar &type, uint fragment);
    void remove_me();

    uint fragment_start;
    uint fragment_end;

    QTextFrame *parentFrame;
    QList<QTextFrame *> childFrames;
    QTextFrameLayoutData *layoutData;
};

#endif // QTEXTOBJECT_P_H
