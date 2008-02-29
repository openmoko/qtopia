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

#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextcursor_p.h"
#include "qtextlist.h"

#include <qdebug.h>
#include <qtextcodec.h>
#include <qbytearray.h>
#include <qdatastream.h>

QTextCopyHelper::QTextCopyHelper(const QTextCursor &_source, const QTextCursor &_destination, bool forceCharFormat, const QTextCharFormat &fmt)
    : formatCollection(*_destination.d->priv->formatCollection()), originalText(_source.d->priv->buffer())
{
    src = _source.d->priv;
    dst = _destination.d->priv;
    insertPos = _destination.position();
    this->forceCharFormat = forceCharFormat;
    primaryCharFormatIndex = convertFormatIndex(fmt);
    cursor = _source;
}

int QTextCopyHelper::convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet)
{
    QTextFormat fmt = oldFormat;
    if (objectIndexToSet != -1) {
        fmt.setObjectIndex(objectIndexToSet);
    } else if (fmt.objectIndex() != -1) {
        int newObjectIndex = objectIndexMap.value(fmt.objectIndex(), -1);
        if (newObjectIndex == -1) {
            QTextFormat objFormat = src->formatCollection()->objectFormat(fmt.objectIndex());
            Q_ASSERT(objFormat.objectIndex() == -1);
            newObjectIndex = formatCollection.createObjectIndex(objFormat);
            objectIndexMap.insert(fmt.objectIndex(), newObjectIndex);
        }
        fmt.setObjectIndex(newObjectIndex);
    }
    int idx = formatCollection.indexForFormat(fmt);
    Q_ASSERT(formatCollection.format(idx).type() == oldFormat.type());
    return idx;
}

int QTextCopyHelper::appendFragment(int pos, int endPos, int objectIndex)
{
    QTextDocumentPrivate::FragmentIterator fragIt = src->find(pos);
    const QTextFragmentData * const frag = fragIt.value();

    Q_ASSERT(objectIndex == -1
             || (frag->size == 1 && src->formatCollection()->format(frag->format).objectIndex() != -1));

    int charFormatIndex;
    if (forceCharFormat)
       charFormatIndex = primaryCharFormatIndex;
    else
       charFormatIndex = convertFormatIndex(frag->format, objectIndex);

    const int inFragmentOffset = qMax(0, pos - fragIt.position());
    int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

    QTextBlock nextBlock = src->blocksFind(pos + 1);

    int blockIdx = -2;
    if (nextBlock.position() == pos + 1) {
        blockIdx = convertFormatIndex(nextBlock.blockFormat());
    } else if (pos == 0 && insertPos == 0) {
        dst->setBlockFormat(dst->blocksBegin(), dst->blocksBegin(), convertFormat(src->blocksBegin().blockFormat()).toBlockFormat());
        dst->setCharFormat(-1, 1, convertFormat(src->blocksBegin().charFormat()).toCharFormat());
    }

    QString txtToInsert(originalText.constData() + frag->stringPosition + inFragmentOffset, charsToCopy);
    if (txtToInsert.length() == 1
        && (txtToInsert.at(0) == QChar::ParagraphSeparator
            || txtToInsert.at(0) == QTextBeginningOfFrame
            || txtToInsert.at(0) == QTextEndOfFrame
           )
       ) {
        dst->insertBlock(txtToInsert.at(0), insertPos, blockIdx, charFormatIndex);
        ++insertPos;
    } else {
        dst->insert(insertPos, txtToInsert, charFormatIndex);
        insertPos += txtToInsert.length();
    }

    return charsToCopy;
}

void QTextCopyHelper::appendFragments(int pos, int endPos)
{
    Q_ASSERT(pos < endPos);

    while (pos < endPos)
        pos += appendFragment(pos, endPos);
}

void QTextCopyHelper::copy()
{
    if (cursor.hasComplexSelection()) {
        QTextTable *table = cursor.currentTable();
        int row_start, col_start, num_rows, num_cols;
        cursor.selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        QTextTableFormat tableFormat = table->format();
        tableFormat.setColumns(num_cols);
        tableFormat.clearColumnWidthConstraints();
        const int objectIndex = dst->formatCollection()->createObjectIndex(tableFormat);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                const int rspan = cell.rowSpan();
                const int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                // add the QTextBeginningOfFrame
                QTextCharFormat cellFormat = cell.format();
                if (r + rspan >= row_start + num_rows) {
                    cellFormat.setTableCellRowSpan(row_start + num_rows - r);
                }
                if (c + cspan >= col_start + num_cols) {
                    cellFormat.setTableCellColumnSpan(col_start + num_cols - c);
                }
                const int charFormatIndex = convertFormatIndex(cellFormat, objectIndex);

                int blockIdx = -2;
                const int cellPos = cell.firstPosition();
                QTextBlock block = src->blocksFind(cellPos);
                if (block.position() == cellPos) {
                    blockIdx = convertFormatIndex(block.blockFormat());
                }

                dst->insertBlock(QTextBeginningOfFrame, insertPos, blockIdx, charFormatIndex);
                ++insertPos;

                // nothing to add for empty cells
                if (cell.lastPosition() > cellPos) {
                    // add the contents
                    appendFragments(cellPos, cell.lastPosition());
                }
            }
        }

        // add end of table
        int end = table->lastPosition();
        appendFragment(end, end+1, objectIndex);
    } else {
        appendFragments(cursor.selectionStart(), cursor.selectionEnd());
    }
}

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate()
    : ref(1), doc(new QTextDocument), containsCompleteDocument(false), importedFromPlainText(false)
{
    doc->setUndoRedoEnabled(false);
}

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextCursor &_cursor)
    : ref(1), doc(0), containsCompleteDocument(false), importedFromPlainText(false)
{
    doc = new QTextDocument;
    doc->setUndoRedoEnabled(false);

    if (!_cursor.hasSelection())
        return;

    doc->docHandle()->beginEditBlock();

    if (_cursor.selectionStart() == 0 && _cursor.selectionEnd() == _cursor.d->priv->length() - 1) {
        containsCompleteDocument = true;
        doc->rootFrame()->setFrameFormat(_cursor.d->priv->rootFrame()->frameFormat());
        doc->setMetaInformation(QTextDocument::DocumentTitle, _cursor.d->priv->document()->metaInformation(QTextDocument::DocumentTitle));
    }
    doc->setDefaultFont(_cursor.d->priv->defaultFont());

    QTextCursor destCursor(doc);
    QTextCopyHelper(_cursor, destCursor).copy();
    doc->docHandle()->endEditBlock();
}

void QTextDocumentFragmentPrivate::insert(QTextCursor &_cursor) const
{
    if (_cursor.isNull())
        return;

    QTextDocumentPrivate *destPieceTable = _cursor.d->priv;
    destPieceTable->beginEditBlock();

    QTextCursor sourceCursor(doc);
    sourceCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QTextCopyHelper(sourceCursor, _cursor, importedFromPlainText, _cursor.charFormat()).copy();

    if (containsCompleteDocument) {
        destPieceTable->document()->rootFrame()->setFrameFormat(doc->rootFrame()->frameFormat());
        destPieceTable->document()->setMetaInformation(QTextDocument::DocumentTitle, doc->metaInformation(QTextDocument::DocumentTitle));
    }

    destPieceTable->endEditBlock();
}

/*!
    \class QTextDocumentFragment
    \brief The QTextDocumentFragment class represents a piece of formatted text
    from a QTextDocument.

    \ingroup text
    \ingroup shared

    A QTextDocumentFragment is a fragment of rich text, that can be inserted into
    a QTextDocument. A document fragment can be created from a
    QTextDocument, from a QTextCursor's selection, or from another
    document fragment. Document fragments can also be created by the
    static functions, fromPlainText() and fromHTML().

    The contents of a document fragment can be obtained as plain text
    by using the toPlainText() function, or it can be obtained as HTML
    with toHtml().
*/


/*!
    Constructs an empty QTextDocumentFragment.

    \sa isEmpty()
*/
QTextDocumentFragment::QTextDocumentFragment()
    : d(0)
{
}

/*!
    Converts the given \a document into a QTextDocumentFragment.
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextDocument *document)
    : d(0)
{
    if (!document)
        return;

    QTextCursor cursor(const_cast<QTextDocument *>(document));
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    d = new QTextDocumentFragmentPrivate(cursor);
}

/*!
    Creates a QTextDocumentFragment from the \a{cursor}'s selection.
    If the cursor doesn't have a selection, the created fragment is empty.

    \sa isEmpty() QTextCursor::selection()
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextCursor &cursor)
    : d(0)
{
    if (!cursor.hasSelection())
        return;

    d = new QTextDocumentFragmentPrivate(cursor);
}

/*!
    \fn QTextDocumentFragment::QTextDocumentFragment(const QTextDocumentFragment &other)

    Copy constructor. Creates a copy of the \a other fragment.
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextDocumentFragment &rhs)
    : d(rhs.d)
{
    if (d)
        d->ref.ref();
}

/*!
    \fn QTextDocumentFragment &QTextDocumentFragment::operator=(const QTextDocumentFragment &other)

    Assigns the \a other fragment to this fragment.
*/
QTextDocumentFragment &QTextDocumentFragment::operator=(const QTextDocumentFragment &rhs)
{
    QTextDocumentFragmentPrivate *x = rhs.d;
    if (x)
        x->ref.ref();
    x = qAtomicSetPtr(&d, x);
    if (x && !x->ref.deref())
        delete x;
    return *this;
}

/*!
    Destroys the document fragment.
*/
QTextDocumentFragment::~QTextDocumentFragment()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
    Returns true if the fragment is empty; otherwise returns false.
*/
bool QTextDocumentFragment::isEmpty() const
{
    return !d || !d->doc || d->doc->docHandle()->length() <= 1;
}

/*!
    Returns the document fragment's text as plain text (i.e. with no
    formatting information).

    \sa toHtml()
*/
QString QTextDocumentFragment::toPlainText() const
{
    if (!d)
        return QString();

    QString result = d->doc->toPlainText();

    if (d->containsCompleteDocument
        && !result.isEmpty()
        && result.at(0) == QLatin1Char('\n'))
        result.remove(0, 1);

    return result;
}

// #### Qt 5: merge with other overload
/*!
    \overload
*/
QString QTextDocumentFragment::toHtml() const
{
    return toHtml(QByteArray());
}

/*!
    \since 4.2

    Returns the contents of the document fragment as HTML,
    using the specified \a encoding (e.g., "UTF-8", "ISO 8859-1").

    \sa toPlainText(), QTextDocument::toHtml(), QTextCodec
*/
QString QTextDocumentFragment::toHtml(const QByteArray &encoding) const
{
    if (!d)
        return QString();

    QTextHtmlExporter exporter(d->doc);
    if (!d->containsCompleteDocument)
        exporter.setFragmentMarkers(true);
    return exporter.toHtml(encoding);
}

/*!
    Returns a document fragment that contains the given \a plainText.

    When inserting such a fragment into a QTextDocument the current char format of
    the QTextCursor used for insertion is used as format for the text.
*/
QTextDocumentFragment QTextDocumentFragment::fromPlainText(const QString &plainText)
{
    QTextDocumentFragment res;

    res.d = new QTextDocumentFragmentPrivate;
    res.d->importedFromPlainText = true;
    QTextCursor cursor(res.d->doc);
    cursor.insertText(plainText);
    return res;
}

QTextHtmlImporter::QTextHtmlImporter(QTextDocument *_doc, const QString &_html, const QTextDocument *resourceProvider)
    : indent(0), setNamedAnchorInNextOutput(false), doc(_doc), containsCompleteDoc(false)
{
    cursor = QTextCursor(doc);

    QString html = _html;
    const int startFragmentPos = html.indexOf(QLatin1String("<!--StartFragment-->"));
    if (startFragmentPos != -1) {
        const int endFragmentPos = html.indexOf(QLatin1String("<!--EndFragment-->"));
        if (startFragmentPos < endFragmentPos)
            html = html.mid(startFragmentPos, endFragmentPos - startFragmentPos);
        else
            html = html.mid(startFragmentPos);

        html.prepend(QLatin1String("<meta name=\"qrichtext\" content=\"1\" />"));
    }

    parse(html, resourceProvider ? resourceProvider : doc);
//    dumpHtml();
}

static QTextListFormat::Style nextListStyle(QTextListFormat::Style style)
{
    if (style == QTextListFormat::ListDisc)
        return QTextListFormat::ListCircle;
    else if (style == QTextListFormat::ListCircle)
        return QTextListFormat::ListSquare;
    return style;
}

void QTextHtmlImporter::import()
{
    cursor.beginEditBlock();
    bool hasBlock = true;
    bool forceBlockMerging = false;
    for (int i = 0; i < count(); ++i) {
        const QTextHtmlParserNode *node = &at(i);

        /*
         * process each node in three stages:
         * 1) check if the hierarchy changed and we therefore passed the
         *    equivalent of a closing tag -> we may need to finish off
         *    some structures like tables
         *
         * 2) check if the current node is a special node like a
         *    <table>, <ul> or <img> tag that requires special processing
         *
         * 3) if the node should result in a QTextBlock create one and
         *    finally insert text that may be attached to the node
         */

        /* emit 'closing' table blocks or adjust current indent level
         * if we
         *  1) are beyond the first node
         *  2) the current node not being a child of the previous node
         *      means there was a tag closing in the input html
         */
        if (i > 0 && (node->parent != i - 1)) {
            const bool blockTagClosed = closeTag(i);
            if (hasBlock && blockTagClosed)
                hasBlock = false;

            // make sure there's a block for 'Blah' after <ul><li>foo</ul>Blah
            if (blockTagClosed
                && !hasBlock
                && !node->isBlock
                && !node->text.isEmpty()
                && node->displayMode != QTextHtmlElement::DisplayNone) {

                QTextBlockFormat block = node->blockFormat();
                block.setIndent(indent);

                appendBlock(block, node->charFormat());

                hasBlock = true;
            }
        }

        if (node->displayMode == QTextHtmlElement::DisplayNone) {
            if (node->id == Html_title)
                doc->setMetaInformation(QTextDocument::DocumentTitle, node->text);
            // ignore explicitly 'invisible' elements
            continue;
        } else if (node->id == Html_body) {
            containsCompleteDoc = true;
            if (node->background.style() != Qt::NoBrush) {
                QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
                fmt.setBackground(node->background);
                doc->rootFrame()->setFrameFormat(fmt);
                const_cast<QTextHtmlParserNode *>(node)->background = QBrush();
            }
        } else if (node->isListStart) {

            QTextListFormat::Style style = node->listStyle;

            if (node->id == Html_ul && !node->hasOwnListStyle && node->parent) {
                const QTextHtmlParserNode *n = &at(node->parent);
                while (n) {
                    if (n->id == Html_ul) {
                        style = nextListStyle(node->listStyle);
                    }
                    if (n->parent)
                        n = &at(n->parent);
                    else
                        n = 0;
                }
            }

            QTextListFormat listFmt;
            listFmt.setStyle(style);

            ++indent;
            if (node->hasCssListIndent)
                listFmt.setIndent(node->cssListIndent);
            else
                listFmt.setIndent(indent);

            List l;
            l.format = listFmt;
            lists.append(l);

            if (node->text.isEmpty())
                continue;
        } else if (node->id == Html_table) {
            Table t = scanTable(i);
            tables.append(t);
            hasBlock = false;
            continue;
        } else if (node->id == Html_tr && !tables.isEmpty()) {
            continue;
        } else if (node->id == Html_img) {
            QTextImageFormat fmt;
            fmt.setName(node->imageName);

            QTextCharFormat nodeFmt = node->charFormat();
            if (nodeFmt.hasProperty(QTextFormat::IsAnchor))
                fmt.setAnchor(nodeFmt.isAnchor());
            if (nodeFmt.hasProperty(QTextFormat::AnchorHref))
                fmt.setAnchorHref(nodeFmt.anchorHref());
            if (nodeFmt.hasProperty(QTextFormat::AnchorName))
                fmt.setAnchorName(nodeFmt.anchorName());

            if (node->imageWidth >= 0)
                fmt.setWidth(node->imageWidth);
            if (node->imageHeight >= 0)
                fmt.setHeight(node->imageHeight);

            cursor.insertImage(fmt, QTextFrameFormat::Position(node->cssFloat));
            hasBlock = false;
            continue;
        } else if (node->id == Html_hr) {
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth, node->width);
            appendBlock(blockFormat);
            hasBlock = false;
            continue;
        }

        if (node->isBlock) {
            QTextBlockFormat block;
            QTextCharFormat charFmt;

            if (node->isTableCell && !tables.isEmpty()) {
                Table &t = tables.last();
                if (!t.isTextFrame && !t.currentCell.atEnd()) {
                    const QTextTableCell cell = t.currentCell.cell();
                    if (cell.isValid())
                        cursor.setPosition(cell.firstPosition());
                }
                hasBlock = true;

                if (node->background.style() != Qt::NoBrush) {
                    charFmt.setBackground(node->background);
                    cursor.mergeBlockCharFormat(charFmt);
                }
            }

            if (hasBlock) {
                block = cursor.blockFormat();
                charFmt = cursor.blockCharFormat();
            }

            // collapse
            block.setTopMargin(qMax(block.topMargin(), (qreal)topMargin(i)));

            int bottomMargin = this->bottomMargin(i);

            // for list items we may want to collapse with the bottom margin of the
            // list.
            if (node->isListItem) {
                if (node->parent && at(node->parent).isListStart) {
                    const int listId = node->parent;
                    const QTextHtmlParserNode *list = &at(listId);
                    if (list->children.last() == i /* == index of node */)
                        bottomMargin = qMax(bottomMargin, this->bottomMargin(listId));
                }
            }

            block.setBottomMargin(bottomMargin);

            block.setLeftMargin(leftMargin(i));
            block.setRightMargin(rightMargin(i));

            if (!node->isListItem
                && indent != 0
                && (lists.isEmpty()
                    || !hasBlock
                    || !lists.last().list
                    || lists.last().list->itemNumber(cursor.block()) == -1
                   )
               )
               block.setIndent(indent);

            block.merge(node->blockFormat());
            charFmt.merge(node->charFormat());

            // ####################
//                block.setFloatPosition(node->cssFloat);

            if (node->wsm == QTextHtmlParserNode::WhiteSpacePre)
                block.setNonBreakableLines(true);

            if (node->background.style() != Qt::NoBrush && !node->isTableCell)
                block.setBackground(node->background);

            if (hasBlock && (!node->isEmptyParagraph || forceBlockMerging)) {
                if (cursor.position() == 0) {
                    containsCompleteDoc = true;
                }
                cursor.setBlockFormat(block);
                cursor.setBlockCharFormat(charFmt);
            } else {
                if (i == 1 && cursor.position() == 0 && node->isEmptyParagraph) {
                    containsCompleteDoc = true;
                    cursor.setBlockFormat(block);
                    cursor.setBlockCharFormat(charFmt);
                } else {
                    appendBlock(block, charFmt);
                }
            }

            if (node->isListItem && !lists.isEmpty()) {
                List &l = lists.last();
                if (l.list) {
                    l.list->add(cursor.block());
                } else {
                    l.list = cursor.createList(l.format);
                }
            }

            forceBlockMerging = false;
            if (node->id == Html_body || node->id == Html_html)
                forceBlockMerging = true;

            if (node->isEmptyParagraph) {
                hasBlock = false;
                continue;
            }

            hasBlock = true;
        }

        if (node->isAnchor && !node->anchorName.isEmpty()) {
            setNamedAnchorInNextOutput = true;
            namedAnchor = node->anchorName;
        }
        if (node->text.isEmpty())
            continue;
        hasBlock = false;

        QTextCharFormat format = node->charFormat();
        QString text = node->text;
        if (setNamedAnchorInNextOutput) {
            QTextCharFormat fmt = format;
            fmt.setAnchor(true);
            fmt.setAnchorName(namedAnchor);
            cursor.insertText(QString(text.at(0)), fmt);

            text.remove(0, 1);
            format.setAnchor(false);
            format.setAnchorName(QString());

            setNamedAnchorInNextOutput = false;
        }

        int textStart = 0;
        for (int i = 0; i < text.length(); ++i) {
            QChar ch = text.at(i);

            const int textEnd = i;

            if (ch == QLatin1Char('\n')
                || ch == QChar::ParagraphSeparator) {

                if (textEnd > textStart)
                    cursor.insertText(text.mid(textStart, textEnd - textStart), format);

                textStart = i + 1;

                QTextBlockFormat fmt = cursor.blockFormat();

                if (fmt.hasProperty(QTextFormat::BlockBottomMargin)) {
                    QTextBlockFormat tmp = fmt;
                    tmp.clearProperty(QTextFormat::BlockBottomMargin);
                    cursor.setBlockFormat(tmp);
                }

                fmt.clearProperty(QTextFormat::BlockTopMargin);
                cursor.insertBlock(fmt);
            }
        }
        if (textStart < text.length())
            cursor.insertText(text.mid(textStart, text.length() - textStart), format);
    }

    cursor.endEditBlock();
}

// returns true if a block tag was closed
bool QTextHtmlImporter::closeTag(int i)
{
    const QTextHtmlParserNode *closedNode = &at(i - 1);
    const int endDepth = depth(i) - 1;
    int depth = this->depth(i - 1);
    bool blockTagClosed = false;

    while (depth > endDepth) {
        if (closedNode->id == Html_tr && !tables.isEmpty()) {
            Table &t = tables.last();

            if (!t.isTextFrame) {
                ++t.currentRow;

                // for broken html with rowspans but missing tr tags
                while (!t.currentCell.atEnd() && t.currentCell.row < t.currentRow)
                    ++t.currentCell;
            }

            blockTagClosed = true;
        } else if (closedNode->id == Html_table && !tables.isEmpty()) {
            tables.resize(tables.size() - 1);

            if (tables.isEmpty()) {
                cursor = doc->rootFrame()->lastCursorPosition();
            } else {
                Table &t = tables.last();
                if (t.isTextFrame)
                    cursor = t.frame->lastCursorPosition();
                else if (!t.currentCell.atEnd())
                    cursor = t.currentCell.cell().lastCursorPosition();
            }

            // we don't need an extra block after tables, so we don't
            // claim to have closed one for the creation of a new one
            // in import()
            blockTagClosed = false;
        } else if (closedNode->isTableCell && !tables.isEmpty()) {
            Table &t = tables.last();
            if (!t.isTextFrame)
                ++tables.last().currentCell;
            blockTagClosed = true;
        } else if (closedNode->isListStart && !lists.isEmpty()) {
            lists.resize(lists.size() - 1);
            --indent;
            blockTagClosed = true;
        } else if (closedNode->id == Html_hr
                   || closedNode->id == Html_center
                   || closedNode->id == Html_h1
                   || closedNode->id == Html_h2
                   || closedNode->id == Html_h3
                   || closedNode->id == Html_h4
                   || closedNode->id == Html_h5
                   || closedNode->id == Html_h6
                  ) {
            blockTagClosed = true;
        } else if (closedNode->id == Html_p || closedNode->id == Html_pre) {
            // blockTagClosed may result in the creation of a
            // new block
            if (!closedNode->text.isEmpty())
                blockTagClosed = true;
        }

        closedNode = &at(closedNode->parent);
        --depth;
    }

    return blockTagClosed;
}

QTextHtmlImporter::Table QTextHtmlImporter::scanTable(int tableNodeIdx)
{
    Table table;
    table.columns = 0;

    QVector<QTextLength> columnWidths;
    QVector<int> rowSpanCellsPerRow;

    int tableHeaderRowCount = 0;
    QVector<int> rowNodes;
    rowNodes.reserve(at(tableNodeIdx).children.count());
    foreach (int row, at(tableNodeIdx).children)
        switch (at(row).id) {
            case Html_tr:
                rowNodes += row;
                break;
            case Html_thead:
            case Html_tbody:
            case Html_tfoot:
                foreach (int potentialRow, at(row).children)
                    if (at(potentialRow).id == Html_tr) {
                        rowNodes += potentialRow;
                        if (at(row).id == Html_thead)
                            ++tableHeaderRowCount;
                    }
                break;
            default: break;
        }

    int effectiveRow = 0;
    foreach (int row, rowNodes) {
        int colsInRow = 0;

        foreach (int cell, at(row).children)
            if (at(cell).isTableCell) {

                const QTextHtmlParserNode &c = at(cell);
                const int currentColumn = colsInRow;
                colsInRow += c.tableCellColSpan;

                if (c.tableCellRowSpan > 1) {
                    rowSpanCellsPerRow.resize(effectiveRow + c.tableCellRowSpan + 1);

                    for (int r = effectiveRow + 1; r < effectiveRow + c.tableCellRowSpan; ++r)
                        rowSpanCellsPerRow[r]++;
                }

                columnWidths.resize(qMax(columnWidths.count(), colsInRow));
                for (int i = currentColumn; i < currentColumn + c.tableCellColSpan; ++i)
                    if (columnWidths.at(i).type() == QTextLength::VariableLength)
                        columnWidths[i] = c.width;
            }

        table.columns = qMax(table.columns, colsInRow + rowSpanCellsPerRow.value(effectiveRow, 0));

        ++effectiveRow;
        rowSpanCellsPerRow.append(0);
    }
    table.rows = effectiveRow;

    if (table.rows == 0 || table.columns == 0)
        return table;

    QTextFrameFormat fmt;
    const QTextHtmlParserNode &node = at(tableNodeIdx);
    if (node.isTextFrame) {
        // for plain text frames we set the frame margin
        // for all of top/bottom/left/right, so in the import
        // here it doesn't matter which one we pick
        fmt.setMargin(node.uncollapsedMargin(QTextHtmlParser::MarginTop));
    } else {
        QTextTableFormat tableFmt;
        tableFmt.setCellSpacing(node.tableCellSpacing);
        tableFmt.setCellPadding(node.tableCellPadding);
        if (node.alignment)
            tableFmt.setAlignment(node.alignment);
        tableFmt.setColumns(table.columns);
        tableFmt.setColumnWidthConstraints(columnWidths);
        tableFmt.setHeaderRowCount(tableHeaderRowCount);
        fmt = tableFmt;
    }

    fmt.setBorder(node.tableBorder);
    fmt.setWidth(node.width);
    fmt.setHeight(node.height);
    if (node.pageBreakPolicy != QTextFormat::PageBreak_Auto)
        fmt.setPageBreakPolicy(node.pageBreakPolicy);

    if (node.direction < 2)
        fmt.setLayoutDirection(Qt::LayoutDirection(node.direction));
    if (node.background.style() != Qt::NoBrush)
        fmt.setBackground(node.background);
    else
        fmt.clearBackground();
    fmt.setPosition(QTextFrameFormat::Position(node.cssFloat));

    if (node.isTextFrame) {
        table.frame = cursor.insertFrame(fmt);
        table.isTextFrame = true;
    } else {
        QTextTable *textTable = cursor.insertTable(table.rows, table.columns, fmt.toTableFormat());
        table.frame = textTable;

        int effectiveRow = 0;

        TableCellIterator it(textTable);
        foreach (int row, rowNodes) {
            // skip missing cells from the previous row
            while (it.row < effectiveRow && !it.atEnd())
                ++it;

            foreach (int cell, at(row).children)
            if (at(cell).isTableCell) {
                const QTextHtmlParserNode &c = at(cell);

                if (c.tableCellColSpan > 1 || c.tableCellRowSpan > 1)
                    textTable->mergeCells(it.row, it.column, c.tableCellRowSpan, c.tableCellColSpan);

                ++it;
            }

            ++effectiveRow;
        }

        table.currentCell = TableCellIterator(textTable);
    }
    return table;
}

void QTextHtmlImporter::appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt)
{
    if (setNamedAnchorInNextOutput) {
        charFmt.setAnchor(true);
        charFmt.setAnchorName(namedAnchor);
        setNamedAnchorInNextOutput = false;
    }

    cursor.insertBlock(format, charFmt);
}

/*!
    \fn QTextDocumentFragment QTextDocumentFragment::fromHtml(const QString &text)

    Returns a QTextDocumentFragment based on the arbitrary piece of
    HTML in the given \a text. The formatting is preserved as much as
    possible; for example, "<b>bold</b>" will become a document
    fragment with the text "bold" with a bold character format.
*/
QTextDocumentFragment QTextDocumentFragment::fromHtml(const QString &html)
{
    return fromHtml(html, 0);
}

/*!
    \fn QTextDocumentFragment QTextDocumentFragment::fromHtml(const QString &text, const QTextDocument *resourceProvider)
    \since 4.2

    Returns a QTextDocumentFragment based on the arbitrary piece of
    HTML in the given \a text. The formatting is preserved as much as
    possible; for example, "<b>bold</b>" will become a document
    fragment with the text "bold" with a bold character format.

    If the provided HTML contains references to external resources such as imported style sheets, then
    they will be loaded through the \a resourceProvider.
*/
QTextDocumentFragment QTextDocumentFragment::fromHtml(const QString &html, const QTextDocument *resourceProvider)
{
    QTextDocumentFragment res;
    res.d = new QTextDocumentFragmentPrivate;

    QTextHtmlImporter importer(res.d->doc, html, resourceProvider);
    importer.import();
    res.d->containsCompleteDocument = importer.containsCompleteDocument();
    return res;
}
