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

#include "qtexttable.h"
#include "qtextcursor.h"
#include "qtextformat.h"
#include <qdebug.h>
#include "qtexttable_p.h"
#include "qvarlengtharray.h"

#include <stdlib.h>

/*!
    \class QTextTableCell qtexttable.h
    \brief The QTextTableCell class represents the properties of a
    cell in a QTextTable.

    \ingroup text

    Table cells are pieces of document structure that belong to a table.
    The table orders cells into particular rows and columns; cells can
    also span multiple columns and rows.

    Cells are usually created when a table is inserted into a document with
    QTextCursor::insertTable(), but they are also created and destroyed when
    a table is resized.

    Cells contain information about their location in a table; you can
    obtain the row() and column() numbers of a cell, and its rowSpan()
    and columnSpan().

    The format() of a cell describes the default character format of its
    contents. The firstCursorPosition() and lastCursorPosition() functions
    are used to obtain the extent of the cell in the document.

    \sa QTextTable QTextTableFormat
*/

/*!
    \fn QTextTableCell::QTextTableCell()

    Constructs an invalid table cell.

    \sa isValid()
*/

/*!
    \fn QTextTableCell::QTextTableCell(const QTextTableCell &other)

    Copy constructor. Creates a new QTextTableCell object based on the
    \a other cell.
*/

/*!
    \fn QTextTableCell& QTextTableCell::operator=(const QTextTableCell &other)

    Assigns the \a other table cell to this table cell.
*/

/*!
    \since 4.2

    Sets the cell's character format to \a format. This can for example be used to change
    the background color of the entire cell:

    QTextTableCell cell = table->cellAt(2, 3);
    QTextCharFormat format = cell.format();
    format.setBackground(Qt::blue);
    cell.setFormat(format);

    \sa format()
*/
void QTextTableCell::setFormat(const QTextCharFormat &format)
{
    QTextCharFormat fmt = format;
    fmt.clearProperty(QTextFormat::ObjectIndex);
    QTextDocumentPrivate *p = table->docHandle();
    QTextDocumentPrivate::FragmentIterator frag(&p->fragmentMap(), fragment);
    p->setCharFormat(frag.position(), 1, fmt, QTextDocumentPrivate::SetFormatAndPreserveObjectIndices);
}

/*!
    Returns the cell's character format.
*/
QTextCharFormat QTextTableCell::format() const
{
    QTextDocumentPrivate *p = table->docHandle();
    QTextFormatCollection *c = p->formatCollection();
    return c->charFormat(QTextDocumentPrivate::FragmentIterator(&p->fragmentMap(), fragment)->format);
}

/*!
    Returns the number of the row in the table that contains this cell.

    \sa column()
*/
int QTextTableCell::row() const
{
    const QTextTablePrivate *tp = table->d_func();
    if (tp->dirty)
        tp->update();

    int idx = tp->findCellIndex(fragment);
    if (idx == -1)
        return idx;
    return tp->cellIndices.at(idx) / tp->nCols;
}

/*!
    Returns the number of the column in the table that contains this cell.

    \sa row()
*/
int QTextTableCell::column() const
{
    const QTextTablePrivate *tp = table->d_func();
    if (tp->dirty)
        tp->update();

    int idx = tp->findCellIndex(fragment);
    if (idx == -1)
        return idx;
    return tp->cellIndices.at(idx) % tp->nCols;
}

/*!
    Returns the number of rows this cell spans. The default is 1.

    \sa columnSpan()
*/
int QTextTableCell::rowSpan() const
{
    return format().tableCellRowSpan();
}

/*!
    Returns the number of columns this cell spans. The default is 1.

    \sa rowSpan()
*/
int QTextTableCell::columnSpan() const
{
    return format().tableCellColumnSpan();
}

/*!
    \fn bool QTextTableCell::isValid() const

    Returns true if this is a valid table cell; otherwise returns
    false.
*/


/*!
    Returns the first valid cursor position in this cell.

    \sa lastCursorPosition()
*/
QTextCursor QTextTableCell::firstCursorPosition() const
{
    return QTextCursor(table->d_func()->pieceTable, firstPosition());
}

/*!
    Returns the last valid cursor position in this cell.

    \sa firstCursorPosition()
*/
QTextCursor QTextTableCell::lastCursorPosition() const
{
    return QTextCursor(table->d_func()->pieceTable, lastPosition());
}


/*!
    \internal

    Returns the first valid position in the document occupied by this cell.
*/
int QTextTableCell::firstPosition() const
{
    QTextDocumentPrivate *p = table->docHandle();
    return p->fragmentMap().position(fragment) + 1;
}

/*!
    \internal

    Returns the last valid position in the document occupied by this cell.
*/
int QTextTableCell::lastPosition() const
{
    QTextDocumentPrivate *p = table->docHandle();
    const QTextTablePrivate *td = table->d_func();
    int index = table->d_func()->findCellIndex(fragment);
    int f;
    if (index != -1)
        f = td->cells.value(index + 1, td->fragment_end);
    else
        f = td->fragment_end;
    return p->fragmentMap().position(f);
}


/*!
    Returns a frame iterator pointing to the beginning of the table's cell.

    \sa end()
*/
QTextFrame::iterator QTextTableCell::begin() const
{
    QTextDocumentPrivate *p = table->docHandle();
    int b = p->blockMap().findNode(firstPosition());
    int e = p->blockMap().findNode(lastPosition()+1);
    return QTextFrame::iterator(const_cast<QTextTable *>(table), b, b, e);
}

/*!
    Returns a frame iterator pointing to the end of the table's cell.

    \sa begin()
*/
QTextFrame::iterator QTextTableCell::end() const
{
    QTextDocumentPrivate *p = table->docHandle();
    int b = p->blockMap().findNode(firstPosition());
    int e = p->blockMap().findNode(lastPosition()+1);
    return QTextFrame::iterator(const_cast<QTextTable *>(table), e, b, e);
}


/*!
    \fn QTextCursor QTextTableCell::operator==(const QTextTableCell &other) const

    Returns true if this cell object and the \a other cell object
    describe the same cell; otherwise returns false.
*/

/*!
    \fn QTextCursor QTextTableCell::operator!=(const QTextTableCell &other) const

    Returns true if this cell object and the \a other cell object
    describe different cells; otherwise returns false.
*/

/*!
    \fn QTextTableCell::~QTextTableCell()

    Destroys the table cell.
*/

QTextTablePrivate::~QTextTablePrivate()
{
    if (grid)
        free(grid);
}


QTextTable *QTextTablePrivate::createTable(QTextDocumentPrivate *pieceTable, int pos, int rows, int cols, const QTextTableFormat &tableFormat)
{
    QTextTableFormat fmt = tableFormat;
    fmt.setColumns(cols);
    QTextTable *table = qobject_cast<QTextTable *>(pieceTable->createObject(fmt));
    Q_ASSERT(table);

    pieceTable->beginEditBlock();

//     qDebug("---> createTable: rows=%d, cols=%d at %d", rows, cols, pos);
    // add block after table
    QTextCharFormat charFmt;
    charFmt.setObjectIndex(table->objectIndex());
    int charIdx = pieceTable->formatCollection()->indexForFormat(charFmt);
    int cellIdx = pieceTable->formatCollection()->indexForFormat(QTextBlockFormat());

    QTextTablePrivate *d = table->d_func();
    d->blockFragmentUpdates = true;

    d->fragment_start = pieceTable->insertBlock(QTextBeginningOfFrame, pos, cellIdx, charIdx);
    d->cells.append(d->fragment_start);
    ++pos;

    for (int i = 1; i < rows*cols; ++i) {
        d->cells.append(pieceTable->insertBlock(QTextBeginningOfFrame, pos, cellIdx, charIdx));
// 	    qDebug("      addCell at %d", pos);
        ++pos;
    }

    d->fragment_end = pieceTable->insertBlock(QTextEndOfFrame, pos, cellIdx, charIdx);
// 	qDebug("      addEOR at %d", pos);
    ++pos;

    d->blockFragmentUpdates = false;
    d->dirty = true;

    pieceTable->endEditBlock();

    return table;
}

struct QFragmentFindHelper
{
    inline QFragmentFindHelper(int _pos, const QTextDocumentPrivate::FragmentMap &map)
        : pos(_pos), fragmentMap(map) {}
    uint pos;
    const QTextDocumentPrivate::FragmentMap &fragmentMap;
};

static inline bool operator<(int fragment, const QFragmentFindHelper &helper)
{
    return helper.fragmentMap.position(fragment) < helper.pos;
}

static inline bool operator<(const QFragmentFindHelper &helper, int fragment)
{
    return helper.pos < helper.fragmentMap.position(fragment);
}

int QTextTablePrivate::findCellIndex(int fragment) const
{
    QFragmentFindHelper helper(pieceTable->fragmentMap().position(fragment),
                              pieceTable->fragmentMap());
    QList<int>::ConstIterator it = qBinaryFind(cells.begin(), cells.end(), helper);
    if (it == cells.end())
        return -1;
    return it - cells.begin();
}

void QTextTablePrivate::fragmentAdded(const QChar &type, uint fragment)
{
    dirty = true;
    if (blockFragmentUpdates)
        return;
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(cells.indexOf(fragment) == -1);
        const uint pos = pieceTable->fragmentMap().position(fragment);
        QFragmentFindHelper helper(pos, pieceTable->fragmentMap());
        QList<int>::Iterator it = qLowerBound(cells.begin(), cells.end(), helper);
        cells.insert(it, fragment);
        if (!fragment_start || pos < pieceTable->fragmentMap().position(fragment_start))
            fragment_start = fragment;
        return;
    }
    QTextFramePrivate::fragmentAdded(type, fragment);
}

void QTextTablePrivate::fragmentRemoved(const QChar &type, uint fragment)
{
    dirty = true;
    if (blockFragmentUpdates)
        return;
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(cells.indexOf(fragment) != -1);
        cells.removeAll(fragment);
        if (fragment_start == fragment && cells.size()) {
            fragment_start = cells.at(0);
        }
        if (fragment_start != fragment)
            return;
    }
    QTextFramePrivate::fragmentRemoved(type, fragment);
}

void QTextTablePrivate::update() const
{
    Q_Q(const QTextTable);
    nCols = q->format().columns();
    nRows = (cells.size() + nCols-1)/nCols;
//     qDebug(">>>> QTextTablePrivate::update, nRows=%d, nCols=%d", nRows, nCols);

    grid = (int *)realloc(grid, nRows*nCols*sizeof(int));
    memset(grid, 0, nRows*nCols*sizeof(int));

    QTextDocumentPrivate *p = pieceTable;
    QTextFormatCollection *c = p->formatCollection();

    cellIndices.resize(cells.size());

    int cell = 0;
    for (int i = 0; i < cells.size(); ++i) {
        int fragment = cells.at(i);
        QTextCharFormat fmt = c->charFormat(QTextDocumentPrivate::FragmentIterator(&p->fragmentMap(), fragment)->format);
        int rowspan = fmt.tableCellRowSpan();
        int colspan = fmt.tableCellColumnSpan();

        // skip taken cells
        while (cell < nRows*nCols && grid[cell])
            ++cell;

        int r = cell/nCols;
        int c = cell%nCols;
        cellIndices[i] = cell;

        if (r + rowspan > nRows) {
            grid = (int *)realloc(grid, sizeof(int)*(r + rowspan)*nCols);
            memset(grid + (nRows*nCols), 0, sizeof(int)*(r+rowspan-nRows)*nCols);
            nRows = r + rowspan;
        }

        Q_ASSERT(c + colspan <= nCols);
        for (int ii = 0; ii < rowspan; ++ii) {
            for (int jj = 0; jj < colspan; ++jj) {
                Q_ASSERT(grid[(r+ii)*nCols + c+jj] == 0);
                grid[(r+ii)*nCols + c+jj] = fragment;
//  		    qDebug("    setting cell %d span=%d/%d at %d/%d", fragment, rowspan, colspan, r+ii, c+jj);
            }
        }
    }
//     qDebug("<<<< end: nRows=%d, nCols=%d", nRows, nCols);

    dirty = false;
}





/*!
    \class QTextTable qtexttable.h
    \brief The QTextTable class represents a table in a QTextDocument.

    \ingroup text

    A table is a group of cells ordered into rows and columns. Each table
    contains at least one row and one column. Each cell contains a block, and
    is surrounded by a frame.

    Tables are usually created and inserted into a document with the
    QTextCursor::insertTable() function.
    For example, we can insert a table with three rows and two columns at the
    current cursor position in an editor using the following lines of code:

    \quotefromfile snippets/textdocument-tables/mainwindow.cpp
    \skipto QTextCursor cursor(editor
    \printuntil cursor.movePosition(QTextCursor::Start);
    \skipto QTextTable *table = cursor
    \printuntil QTextTable *table = cursor

    The table format is either defined when the table is created or changed
    later with setFormat().

    The table currently being edited by the cursor is found with
    QTextCursor::currentTable(). This allows its format or dimensions to be
    changed after it has been inserted into a document.

    A table's size can be changed with resize(), or by using
    insertRows(), insertColumns(), removeRows(), or removeColumns().
    Use cellAt() to retrieve table cells.

    The starting and ending positions of table rows can be found by moving
    a cursor within a table, and using the rowStart() and rowEnd() functions
    to obtain cursors at the start and end of each row.

    \sa QTextTableFormat
*/

/*! \internal
 */
QTextTable::QTextTable(QTextDocument *doc)
    : QTextFrame(*new QTextTablePrivate, doc)
{
}

/*! \internal

Destroys the table.
 */
QTextTable::~QTextTable()
{
}


/*!
    \fn QTextTableCell QTextTable::cellAt(int row, int column) const

    Returns the table cell at the given \a row and \a column in the table.

    \sa columns() rows()
*/
QTextTableCell QTextTable::cellAt(int row, int col) const
{
    Q_D(const QTextTable);
    if (d->dirty)
        d->update();

    if (row < 0 || row >= d->nRows || col < 0 || col >= d->nCols)
        return QTextTableCell();

    return QTextTableCell(this, d->grid[row*d->nCols + col]);
}

/*!
    \overload

    Returns the table cell that contains the character at the given \a position
    in the document.
*/
QTextTableCell QTextTable::cellAt(int position) const
{
    Q_D(const QTextTable);
    if (d->dirty)
        d->update();

    uint pos = (uint)position;
    const QTextDocumentPrivate::FragmentMap &m = d->pieceTable->fragmentMap();
    if (position < 0 || m.position(d->fragment_start) >= pos || m.position(d->fragment_end) < pos)
        return QTextTableCell();

    QFragmentFindHelper helper(position, m);
    QList<int>::ConstIterator it = qLowerBound(d->cells.begin(), d->cells.end(), helper);
    if (it != d->cells.begin())
        --it;

    return QTextTableCell(this, *it);
}

/*!
    \fn QTextTableCell QTextTable::cellAt(const QTextCursor &cursor) const

    \overload

    Returns the table cell containing the given \a cursor.
*/
QTextTableCell QTextTable::cellAt(const QTextCursor &c) const
{
    return cellAt(c.position());
}

/*!
    \fn void QTextTable::resize(int rows, int columns)

    Resizes the table to contain the required number of \a rows and \a columns.

    \sa insertRows() insertColumns() removeRows() removeColumns()
*/
void QTextTable::resize(int rows, int cols)
{
    Q_D(QTextTable);
    if (d->dirty)
        d->update();

    d->pieceTable->beginEditBlock();

    int nRows = this->rows();
    int nCols = this->columns();

    if (rows == nRows && cols == nCols)
	return;

    if (nCols < cols)
        insertColumns(nCols, cols - nCols);
    else if (nCols > cols)
        removeColumns(cols, nCols - cols);

    if (nRows < rows)
        insertRows(nRows, rows-nRows);
    else if (nRows > rows)
        removeRows(rows, nRows-rows);

    d->pieceTable->endEditBlock();
}

/*!
    \fn void QTextTable::insertRows(int index, int rows)

    Inserts a number of \a rows before the row with the specified \a index.

    \sa resize() insertColumns() removeRows() removeColumns()
*/
void QTextTable::insertRows(int pos, int num)
{
    Q_D(QTextTable);
    if (num <= 0)
	return;

    if (d->dirty)
        d->update();

    if (pos > d->nRows || pos < 0)
        pos = d->nRows;

//     qDebug() << "-------- insertRows" << pos << num;
    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *c = p->formatCollection();
    p->beginEditBlock();

    int extended = 0;
    int insert_before = 0;
    if (pos > 0 && pos < d->nRows) {
        for (int i = 0; i < d->nCols; ++i) {
            int cell = d->grid[pos*d->nCols + i];
            if (cell == d->grid[(pos-1)*d->nCols+i]) {
                // cell spans the insertion place, extend it
                QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
                QTextCharFormat fmt = c->charFormat(it->format);
                fmt.setTableCellRowSpan(fmt.tableCellRowSpan() + num);
                p->setCharFormat(it.position(), 1, fmt);
                extended++;
            } else if (!insert_before) {
                insert_before = cell;
            }
        }
    } else {
        insert_before = (pos == 0 ? d->grid[0] : d->fragment_end);
    }
    if (extended < d->nCols) {
        Q_ASSERT(insert_before);
        QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), insert_before);
        QTextCharFormat fmt = c->charFormat(it->format);
        fmt.setTableCellRowSpan(1);
        fmt.setTableCellColumnSpan(1);
        Q_ASSERT(fmt.objectIndex() == objectIndex());
        int pos = it.position();
        int cfmt = p->formatCollection()->indexForFormat(fmt);
        int bfmt = p->formatCollection()->indexForFormat(QTextBlockFormat());
//         qDebug("inserting %d cells, nCols=%d extended=%d", num*(d->nCols-extended), d->nCols, extended);
        for (int i = 0; i < num*(d->nCols-extended); ++i)
            p->insertBlock(QTextBeginningOfFrame, pos, bfmt, cfmt, QTextUndoCommand::MoveCursor);
    }

//     qDebug() << "-------- end insertRows" << pos << num;
    p->endEditBlock();
}

/*!
    \fn void QTextTable::insertColumns(int index, int columns)

    Inserts a number of \a columns before the column with the specified \a index.

    \sa insertRows() resize() removeRows() removeColumns()
*/
void QTextTable::insertColumns(int pos, int num)
{
    Q_D(QTextTable);
    if (num <= 0)
	return;

    if (d->dirty)
        d->update();

    if (pos > d->nCols || pos < 0)
        pos = d->nCols;

//     qDebug() << "-------- insertCols" << pos << num;
    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *c = p->formatCollection();
    p->beginEditBlock();

    for (int i = 0; i < d->nRows; ++i) {
        int cell;
        if (i == d->nRows - 1 && pos == d->nCols)
            cell = d->fragment_end;
        else
            cell = d->grid[i*d->nCols + pos];
        QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
        QTextCharFormat fmt = c->charFormat(it->format);
        if (pos > 0 && pos < d->nCols && cell == d->grid[i*d->nCols + pos - 1]) {
            // cell spans the insertion place, extend it
            fmt.setTableCellColumnSpan(fmt.tableCellColumnSpan() + num);
            p->setCharFormat(it.position(), 1, fmt);
        } else {
            fmt.setTableCellRowSpan(1);
            fmt.setTableCellColumnSpan(1);
            Q_ASSERT(fmt.objectIndex() == objectIndex());
            int position = it.position();
            int cfmt = p->formatCollection()->indexForFormat(fmt);
            int bfmt = p->formatCollection()->indexForFormat(QTextBlockFormat());
            for (int i = 0; i < num; ++i)
                p->insertBlock(QTextBeginningOfFrame, position, bfmt, cfmt, QTextUndoCommand::MoveCursor);
        }
    }

    QTextTableFormat tfmt = format();
    tfmt.setColumns(tfmt.columns()+num);
    setFormat(tfmt);

//     qDebug() << "-------- end insertCols" << pos << num;
    p->endEditBlock();
}

/*!
    \fn void QTextTable::removeRows(int index, int rows)

    Removes a number of \a rows starting with the row at the specified \a index.

    \sa insertRows(), insertColumns(), resize(), removeColumns()
*/
void QTextTable::removeRows(int pos, int num)
{
    Q_D(QTextTable);
//     qDebug() << "-------- removeRows" << pos << num;

    if (num <= 0 || pos < 0)
        return;
    if (d->dirty)
        d->update();
    if (pos >= d->nRows)
        return;
    if (pos+num > d->nRows)
        num = d->nRows - pos;

    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *collection = p->formatCollection();
    p->beginEditBlock();

    // delete whole table?
    if (pos == 0 && num == d->nRows) {
        const int pos = p->fragmentMap().position(d->fragment_start);
        p->remove(pos, p->fragmentMap().position(d->fragment_end) - pos + 1);
        p->endEditBlock();
        return;
    }

    for (int r = pos; r < pos + num; ++r) {
        for (int c = 0; c < d->nCols; ++c) {
            int cell = d->grid[r*d->nCols + c];
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
            QTextCharFormat fmt = collection->charFormat(it->format);
            int span = fmt.tableCellRowSpan();
            if (span > 1) {
                fmt.setTableCellRowSpan(span - 1);
                p->setCharFormat(it.position(), 1, fmt);
            } else {
                // remove cell
                int index = d->cells.indexOf(cell) + 1;
                int f_end = index < d->cells.size() ? d->cells.at(index) : d->fragment_end;
                p->remove(it.position(), p->fragmentMap().position(f_end) - it.position());
            }
        }
    }

    p->endEditBlock();
//     qDebug() << "-------- end removeRows" << pos << num;
}

/*!
    \fn void QTextTable::removeColumns(int index, int columns)

    Removes a number of \a columns starting with the column at the specified
    \a index.

    \sa insertRows() insertColumns() removeRows() resize()
*/
void QTextTable::removeColumns(int pos, int num)
{
    Q_D(QTextTable);
//     qDebug() << "-------- removeCols" << pos << num;

    if (num <= 0 || pos < 0)
	return;
    if (d->dirty)
        d->update();
    if (pos >= d->nCols)
        return;
    if (pos + num > d->nCols)
        pos = d->nCols - num;

    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *collection = p->formatCollection();
    p->beginEditBlock();

    // delete whole table?
    if (pos == 0 && num == d->nCols) {
        const int pos = p->fragmentMap().position(d->fragment_start);
        p->remove(pos, p->fragmentMap().position(d->fragment_end) - pos + 1);
        p->endEditBlock();
        return;
    }

    for (int r = 0; r < d->nRows; ++r) {
        for (int c = pos; c < pos + num; ++c) {
            int cell = d->grid[r*d->nCols + c];
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
            QTextCharFormat fmt = collection->charFormat(it->format);
            int span = fmt.tableCellColumnSpan();
            if (span > 1) {
                fmt.setTableCellColumnSpan(span - 1);
                p->setCharFormat(it.position(), 1, fmt);
            } else {
                // remove cell
                int index = d->cells.indexOf(cell) + 1;
                int f_end = index < d->cells.size() ? d->cells.at(index) : d->fragment_end;
                p->remove(it.position(), p->fragmentMap().position(f_end) - it.position());
            }
        }
    }

    QTextTableFormat tfmt = format();
    tfmt.setColumns(tfmt.columns()-num);
    setFormat(tfmt);

    p->endEditBlock();
//     qDebug() << "-------- end removeCols" << pos << num;
}

/*!
    \since 4.1

    Merges the cell at the specified \a row and \a column with the adjacent cells
    into one cell. The new cell will span \a numRows rows and \a numCols columns.
    If \a numRows or \a numCols is less than the current number of rows or columns
    the cell spans then this method does nothing.
*/
void QTextTable::mergeCells(int row, int column, int numRows, int numCols)
{
    Q_D(QTextTable);

    if (d->dirty)
        d->update();

    QTextDocumentPrivate *p = d->pieceTable;

    const QTextTableCell cell = cellAt(row, column);
    if (!cell.isValid())
        return;
    row = cell.row();
    column = cell.column();

    QTextCharFormat fmt = cell.format();
    const int rowSpan = fmt.tableCellRowSpan();
    const int colSpan = fmt.tableCellColumnSpan();

    numRows = qMin(numRows, rows() - cell.row());
    numCols = qMin(numCols, columns() - cell.column());

    // nothing to merge?
    if (numRows < rowSpan || numCols < colSpan)
        return;

    p->beginEditBlock();

    const int origCellPosition = cell.firstPosition() - 1;

    const int cellFragment = d->grid[row * d->nCols + column];

    QVarLengthArray<int> cellMarkersToDelete((numCols - colSpan) * rowSpan
                                             + (numRows - rowSpan) * numCols);

    int idx = 0;

    d->blockFragmentUpdates = true;

    for (int r = row; r < row + rowSpan; ++r) {

        const int fragment = d->grid[r * d->nCols + column + colSpan];
        const uint pos = p->fragmentMap().position(fragment);
        QFragmentFindHelper helper(pos, p->fragmentMap());
        QList<int>::Iterator it = qBinaryFind(d->cells.begin(), d->cells.end(), helper);
        Q_ASSERT(it != d->cells.end());
        Q_ASSERT(*it == fragment);
        d->cellIndices.remove(it - d->cells.begin(), numCols - colSpan);
        d->cells.erase(it, it + numCols - colSpan);

        for (int c = column + colSpan; c < column + numCols; ++c) {
            const int cell = d->grid[r * d->nCols + c];
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
            cellMarkersToDelete[idx++] = it.position();
            d->grid[r * d->nCols + c] = cellFragment;
        }
    }

    for (int r = row + rowSpan; r < row + numRows; ++r) {

        const int fragment = d->grid[r * d->nCols + column];
        const uint pos = p->fragmentMap().position(fragment);
        QFragmentFindHelper helper(pos, p->fragmentMap());
        QList<int>::Iterator it = qBinaryFind(d->cells.begin(), d->cells.end(), helper);
        Q_ASSERT(it != d->cells.end());
        Q_ASSERT(*it == fragment);
        d->cellIndices.remove(it - d->cells.begin(), numCols);
        d->cells.erase(it, it + numCols);

        for (int c = column; c < column + numCols; ++c) {
            const int cell = d->grid[r * d->nCols + c];
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
            cellMarkersToDelete[idx++] = it.position();
            d->grid[r * d->nCols + c] = cellFragment;
        }
    }

    for (int i = 0; i < cellMarkersToDelete.size(); ++i)
        p->remove(cellMarkersToDelete[i] - i, 1);

    d->fragment_start = d->cells.first();

    fmt.setTableCellRowSpan(numRows);
    fmt.setTableCellColumnSpan(numCols);
    p->setCharFormat(origCellPosition, 1, fmt);

    d->blockFragmentUpdates = false;
    d->dirty = false;

    p->endEditBlock();
}

/*!
    \overload
    \since 4.1

    Merges the cells selected by the provided \a cursor.
*/
void QTextTable::mergeCells(const QTextCursor &cursor)
{
    if (!cursor.hasComplexSelection())
        return;

    int firstRow, numRows, firstColumn, numColumns;
    cursor.selectedTableCells(&firstRow, &numRows, &firstColumn, &numColumns);
    mergeCells(firstRow, firstColumn, numRows, numColumns);
}

/*!
    \since 4.1

    Splits the specified cell at \a row and \a column into an array of multiple
    cells with dimensions specified by \a numRows and \a numCols.
*/
void QTextTable::splitCell(int row, int column, int numRows, int numCols)
{
    Q_D(QTextTable);

    if (d->dirty)
        d->update();

    QTextDocumentPrivate *p = d->pieceTable;
    QTextFormatCollection *c = p->formatCollection();

    const QTextTableCell cell = cellAt(row, column);
    if (!cell.isValid())
        return;
    row = cell.row();
    column = cell.column();

    QTextCharFormat fmt = cell.format();
    const int rowSpan = fmt.tableCellRowSpan();
    const int colSpan = fmt.tableCellColumnSpan();

    // nothing to split?
    if (numRows > rowSpan || numCols > colSpan)
        return;

    p->beginEditBlock();

    const int origCellPosition = cell.firstPosition() - 1;

    QVarLengthArray<int> rowPositions(rowSpan);

    for (int r = row; r < row + numRows; ++r) {
        const QTextTableCell cell = cellAt(r, column + numCols);
        rowPositions[r - row] = cell.lastPosition();
    }

    for (int r = row + numRows; r < row + rowSpan; ++r) {
        const QTextTableCell cell = cellAt(r, column);
        rowPositions[r - row] = cell.lastPosition();
    }

    fmt.setTableCellColumnSpan(1);
    fmt.setTableCellRowSpan(1);
    const int fmtIndex = c->indexForFormat(fmt);
    const int blockIndex = p->blockMap().find(cell.lastPosition())->format;

    int insertAdjustement = 0;
    for (int i = 0; i < numRows; ++i) {
        for (int c = 0; c < colSpan - numCols; ++c)
            p->insertBlock(QTextBeginningOfFrame, rowPositions[i] + insertAdjustement + c, blockIndex, fmtIndex);
        insertAdjustement += colSpan - numCols;
    }

    for (int i = numRows; i < rowSpan; ++i) {
        for (int c = 0; c < colSpan; ++c)
            p->insertBlock(QTextBeginningOfFrame, rowPositions[i] + insertAdjustement + c, blockIndex, fmtIndex);
        insertAdjustement += colSpan;
    }

    fmt.setTableCellRowSpan(numRows);
    fmt.setTableCellColumnSpan(numCols);
    p->setCharFormat(origCellPosition, 1, fmt);

    p->endEditBlock();
}

/*!
    Returns the number of rows in the table.

    \sa columns()
*/
int QTextTable::rows() const
{
    Q_D(const QTextTable);
    if (d->dirty)
        d->update();

    return d->nRows;
}

/*!
    Returns the number of columns in the table.

    \sa rows()
*/
int QTextTable::columns() const
{
    Q_D(const QTextTable);
    if (d->dirty)
        d->update();

    return d->nCols;
}

#if 0
void QTextTable::mergeCells(const QTextCursor &selection)
{
}
#endif

/*!
    \fn QTextCursor QTextTable::rowStart(const QTextCursor &cursor) const

    Returns a cursor pointing to the start of the row that contains the
    given \a cursor.

    \sa rowEnd()
*/
QTextCursor QTextTable::rowStart(const QTextCursor &c) const
{
    Q_D(const QTextTable);
    QTextTableCell cell = cellAt(c);
    if (!cell.isValid())
        return QTextCursor();

    int row = cell.row();
    QTextDocumentPrivate *p = d->pieceTable;
    QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), d->grid[row*d->nCols]);
    return QTextCursor(p, it.position());
}

/*!
    \fn QTextCursor QTextTable::rowEnd(const QTextCursor &cursor) const

    Returns a cursor pointing to the end of the row that contains the given
    \a cursor.

    \sa rowStart()
*/
QTextCursor QTextTable::rowEnd(const QTextCursor &c) const
{
    Q_D(const QTextTable);
    QTextTableCell cell = cellAt(c);
    if (!cell.isValid())
        return QTextCursor();

    int row = cell.row() + 1;
    int fragment = row < d->nRows ? d->grid[row*d->nCols] : d->fragment_end;
    QTextDocumentPrivate *p = d->pieceTable;
    QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), fragment);
    return QTextCursor(p, it.position() - 1);
}

/*!
    \fn void QTextTable::setFormat(const QTextTableFormat &format)

    Sets the table's \a format.

    \sa format()
*/

/*!
    \fn QTextTableFormat QTextTable::format() const

    Returns the table's format.

    \sa setFormat()
*/

