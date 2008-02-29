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

#ifndef QTEXTCURSOR_P_H
#define QTEXTCURSOR_P_H

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

#include "qtextcursor.h"
#include "qtextdocument.h"
#include "qtextdocument_p.h"
#include <private/qtextformat_p.h>
#include "qtextobject.h"


class QTextCursorPrivate : public QSharedData
{
public:
    QTextCursorPrivate(QTextDocumentPrivate *p);
    QTextCursorPrivate(const QTextCursorPrivate &rhs);
    ~QTextCursorPrivate();

    enum AdjustResult { CursorMoved, CursorUnchanged };
    AdjustResult adjustPosition(int positionOfChange, int charsAddedOrRemoved, QTextUndoCommand::Operation op);

    void adjustCursor(QTextCursor::MoveOperation m);

    void remove();
    void clearCells(QTextTable *table, int startRow, int startCol, int numRows, int numCols, QTextUndoCommand::Operation op);
    inline void setPosition(int newPosition) {
        Q_ASSERT(newPosition >= 0 && newPosition < priv->length());
        position = newPosition;
        currentCharFormat = -1;
    }
    void setX();
    bool canDelete(int pos) const;

    void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);
    bool movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    inline QTextBlock block() const
        { return QTextBlock(priv, priv->blockMap().findNode(position)); }
    inline QTextBlockFormat blockFormat() const
        { return block().blockFormat(); }

    QTextTable *complexSelectionTable() const;
    void selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const;

    void setBlockCharFormat(const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode);
    void setBlockFormat(const QTextBlockFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode);
    void setCharFormat(const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode);

    QTextDocumentPrivate *priv;
    qreal x;
    int position;
    int anchor;
    int adjusted_anchor;
    int currentCharFormat;
};

#endif // QTEXTCURSOR_P_H
