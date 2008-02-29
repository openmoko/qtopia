/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qaccessiblewidgets.h"
#include "qabstracttextdocumentlayout.h"
#include "qtextedit.h"
#include "qtextdocument.h"
#include "qtextobject.h"
#include "qscrollbar.h"
#include "qdebug.h"

#if !defined(QT_NO_ACCESSIBILITY) && !defined(QT_NO_TEXTEDIT)

/*!
  \class QAccessibleTextEdit qaccessiblewidget.h
  \brief The QAccessibleTextEdit class implements the QAccessibleInterface for richtext editors.
  \internal
*/

static QTextBlock qTextBlockAt(const QTextDocument *doc, int pos)
{
    Q_ASSERT(pos >= 0);

    QTextBlock block = doc->begin();
    int i = 0;
    while (block.isValid() && i < pos) {
        block = block.next();
        ++i;
    }
    return block;
}

static int qTextBlockPosition(QTextBlock block)
{
    int child = 0;
    while (block.isValid()) {
        block = block.previous();
        ++child;
    }

    return child;
}

/*!
  \fn QAccessibleTextEdit::QAccessibleTextEdit(QWidget* widget)

  Constructs a QAccessibleTextEdit object for a \a widget.
*/
QAccessibleTextEdit::QAccessibleTextEdit(QWidget *o)
: QAccessibleWidgetEx(o, EditableText)
{
    Q_ASSERT(widget()->inherits("QTextEdit"));
    childOffset = QAccessibleWidgetEx::childCount();
}

/*! Returns the text edit. */
QTextEdit *QAccessibleTextEdit::textEdit() const
{
    return static_cast<QTextEdit *>(widget());
}

QRect QAccessibleTextEdit::rect(int child) const
{
    if (child <= childOffset)
        return QAccessibleWidgetEx::rect(child);

     QTextEdit *edit = textEdit();
     QTextBlock block = qTextBlockAt(edit->document(), child - childOffset - 1);
     if (!block.isValid())
         return QRect();

     QRect rect = edit->document()->documentLayout()->blockBoundingRect(block).toRect();
     rect.translate(-edit->horizontalScrollBar()->value(), -edit->verticalScrollBar()->value());

     rect = edit->viewport()->rect().intersect(rect);
     if (rect.isEmpty())
         return QRect();

     return rect.translated(edit->viewport()->mapToGlobal(QPoint(0, 0)));
}

int QAccessibleTextEdit::childAt(int x, int y) const
{
    QTextEdit *edit = textEdit();

    QPoint point = edit->viewport()->mapFromGlobal(QPoint(x, y));
    QTextBlock block = edit->cursorForPosition(point).block();
    if (block.isValid())
        return qTextBlockPosition(block) + childOffset;

    return QAccessibleWidgetEx::childAt(x, y);
}

/*! \reimp */
QString QAccessibleTextEdit::text(Text t, int child) const
{
    if (t == Value) {
        if (child > childOffset)
            return qTextBlockAt(textEdit()->document(), child - childOffset - 1).text();
        if (!child)
            return textEdit()->toPlainText();
    }

    return QAccessibleWidgetEx::text(t, child);
}

/*! \reimp */
void QAccessibleTextEdit::setText(Text t, int child, const QString &text)
{
    if (t != Value || (child > 0 && child <= childOffset)) {
        QAccessibleWidgetEx::setText(t, child, text);
        return;
    }
    if (textEdit()->isReadOnly())
        return;

    if (!child) {
        textEdit()->setText(text);
        return;
    }
    QTextBlock block = qTextBlockAt(textEdit()->document(), child - childOffset - 1);
    if (!block.isValid())
        return;

    QTextCursor cursor(block);
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.insertText(text);
}

/*! \reimp */
QAccessible::Role QAccessibleTextEdit::role(int child) const
{
    if (child > childOffset)
        return EditableText;
    return QAccessibleWidgetEx::role(child);
}

QVariant QAccessibleTextEdit::invokeMethodEx(QAccessible::Method method, int child,
                                                     const QVariantList &params)
{
    if (child)
        return QVariant();

    switch (method) {
    case ListSupportedMethods: {
        QVariantList list;
        list << ListSupportedMethods << SetCursorPosition << GetCursorPosition;
        return list;
    }
    case SetCursorPosition: {
        QTextCursor cursor = textEdit()->textCursor();
        cursor.setPosition(params.value(0).toInt());
        textEdit()->setTextCursor(cursor);
        return true; }
    case GetCursorPosition:
        return textEdit()->textCursor().position();
    }

    return QVariant();
}

int QAccessibleTextEdit::childCount() const
{
    return childOffset + textEdit()->document()->blockCount();
}

#endif // QT_NO_ACCESSIBILITY
