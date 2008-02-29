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

#ifndef QTEXTCONTROL_P_P_H
#define QTEXTCONTROL_P_P_H

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

#include "QtGui/qtextdocumentfragment.h"
#include "QtGui/qscrollbar.h"
#include "QtGui/qtextcursor.h"
#include "QtGui/qtextformat.h"
#include "QtGui/qmenu.h"
#include "QtGui/qabstracttextdocumentlayout.h"
#include "QtCore/qbasictimer.h"
#include "QtCore/qpointer.h"
#include "private/qobject_p.h"

class QMimeData;
class QAbstractScrollArea;

class QTextControlPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextControl)
public:
    QTextControlPrivate();

    bool cursorMoveKeyEvent(QKeyEvent *e);

    void updateCurrentCharFormat();

    void indent();
    void outdent();

    void gotoNextTableCell();
    void gotoPreviousTableCell();

    void createAutoBulletList();

    void setContent(Qt::TextFormat format = Qt::RichText, const QString &text = QString(),
                    QTextDocument *document = 0);
    void startDrag();

    void paste(const QMimeData *source);

    void setCursorPosition(const QPointF &pos);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void repaintCursor();
    inline void repaintSelection()
    { repaintOldAndNewSelection(QTextCursor()); }
    void repaintOldAndNewSelection(const QTextCursor &oldSelection);

    void selectionChanged();

    void _q_updateCurrentCharFormatAndSelection();

    void setClipboardSelection();

    void _q_emitCursorPosChanged(const QTextCursor &someCursor);

    void setBlinkingCursorEnabled(bool enable);

    void extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition);
    void extendLinewiseSelection(int suggestedNewPosition);

    void _q_deleteSelected();

    void _q_setCursorAfterUndoRedo(int undoPosition, int charsAdded, int charsRemoved);

    QRectF rectForPosition(int position) const;
    QRectF selectionRect(const QTextCursor &cursor) const;
    inline QRectF selectionRect() const
    { return selectionRect(this->cursor); }

    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(Qt::MouseButton button, const QPointF &pos,
                         Qt::KeyboardModifiers modifiers,
                         Qt::MouseButtons buttons,
                         const QPoint &globalPos = QPoint());
    void mouseMoveEvent(Qt::MouseButtons buttons, const QPointF &pos);
    void mouseReleaseEvent(Qt::MouseButton button, const QPointF &pos);
    void mouseDoubleClickEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos);
    void contextMenuEvent(const QPoint &screenPos, const QPointF &docPos, QWidget *contextWidget);
    void focusEvent(QFocusEvent *e);

    bool dragEnterEvent(QEvent *e, const QMimeData *mimeData);
    void dragLeaveEvent();
    bool dragMoveEvent(QEvent *e, const QMimeData *mimeData, const QPointF &pos);
    bool dropEvent(const QMimeData *mimeData, const QPointF &pos, Qt::DropAction dropAction, QWidget *source);

    void inputMethodEvent(QInputMethodEvent *);

    bool findNextPrevAnchor(bool next, int &start, int &end);

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    bool cursorIsFocusIndicator;
    QTextCharFormat lastCharFormat;

    QTextCursor dndFeedbackCursor;

    Qt::TextInteractionFlags interactionFlags;

    QBasicTimer cursorBlinkTimer;
    QBasicTimer trippleClickTimer;
    QPointF trippleClickPoint;

    bool mousePressed;

    bool mightStartDrag;
    QPoint dragStartPos;
    QPointer<QWidget> contextWidget;

    bool lastSelectionState;

    bool ignoreAutomaticScrollbarAdjustement;

    QTextCursor selectedWordOnDoubleClick;
    QTextCursor selectedLineOnDoubleClick;

    bool overwriteMode;
    bool acceptRichText;

    int preeditCursor;
    bool hideCursor; // used to hide the cursor in the preedit area

    QVector<QAbstractTextDocumentLayout::Selection> extraSelections;

    QPalette palette;
    bool hasFocus;
    Qt::LayoutDirection layoutDirection;
    bool isEnabled;

    QString highlightedAnchor; // Anchor below cursor
    QString anchorOnMousePress;
    bool hadSelectionOnMousePress;

    bool openExternalLinks;

    QString linkToCopy;
    void _q_copyLink();
};

#endif // QTEXTCONTROL_P_H
