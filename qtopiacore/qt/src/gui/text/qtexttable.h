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

#ifndef QTEXTTABLE_H
#define QTEXTTABLE_H

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QTextCursor;
class QTextTable;
class QTextTablePrivate;

class Q_GUI_EXPORT QTextTableCell
{
public:
    QTextTableCell() : table(0) {}
    ~QTextTableCell() {}
    QTextTableCell(const QTextTableCell &o) : table(o.table), fragment(o.fragment) {}
    QTextTableCell &operator=(const QTextTableCell &o)
    { table = o.table; fragment = o.fragment; return *this; }

    void setFormat(const QTextCharFormat &format);
    QTextCharFormat format() const;

    int row() const;
    int column() const;

    int rowSpan() const;
    int columnSpan() const;

    inline bool isValid() const { return table != 0; }

    QTextCursor firstCursorPosition() const;
    QTextCursor lastCursorPosition() const;
    int firstPosition() const;
    int lastPosition() const;

    inline bool operator==(const QTextTableCell &other) const
    { return table == other.table && fragment == other.fragment; }
    inline bool operator!=(const QTextTableCell &other) const
    { return !operator==(other); }

    QTextFrame::iterator begin() const;
    QTextFrame::iterator end() const;

private:
    friend class QTextTable;
    QTextTableCell(const QTextTable *t, int f)
        : table(t), fragment(f) {}

    const QTextTable *table;
    int fragment;
};

class Q_GUI_EXPORT QTextTable : public QTextFrame
{
    Q_OBJECT
public:
    explicit QTextTable(QTextDocument *doc);
    ~QTextTable();

    void resize(int rows, int cols);
    void insertRows(int pos, int num);
    void insertColumns(int pos, int num);
    void removeRows(int pos, int num);
    void removeColumns(int pos, int num);

    void mergeCells(int row, int col, int numRows, int numCols);
    void mergeCells(const QTextCursor &cursor);
    void splitCell(int row, int col, int numRows, int numCols);

    int rows() const;
    int columns() const;

    QTextTableCell cellAt(int row, int col) const;
    QTextTableCell cellAt(int position) const;
    QTextTableCell cellAt(const QTextCursor &c) const;

    QTextCursor rowStart(const QTextCursor &c) const;
    QTextCursor rowEnd(const QTextCursor &c) const;

    inline void setFormat(const QTextTableFormat &format);
    QTextTableFormat format() const { return QTextObject::format().toTableFormat(); }

private:
    Q_DISABLE_COPY(QTextTable)
    Q_DECLARE_PRIVATE(QTextTable)
    friend class QTextTableCell;
};

inline void QTextTable::setFormat(const QTextTableFormat &aformat)
{ QTextObject::setFormat(aformat); }

QT_END_HEADER

#endif // QTEXTTABLE_H
