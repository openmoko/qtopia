/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTDOCUMENTFRAGMENT_P_H
#define QTEXTDOCUMENTFRAGMENT_P_H

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

#include "QtGui/qtextdocument.h"
#include "private/qtexthtmlparser_p.h"
#include "private/qtextdocument_p.h"
#include "QtGui/qtexttable.h"
#include "QtCore/qatomic.h"
#include "QtCore/qlist.h"
#include "QtCore/qmap.h"
#include "QtCore/qpointer.h"
#include "QtCore/qvarlengtharray.h"
#include "QtCore/qdatastream.h"

class QTextDocumentFragmentPrivate;

class QTextCopyHelper
{
public:
    QTextCopyHelper(const QTextCursor &_source, const QTextCursor &_destination, bool forceCharFormat = false, const QTextCharFormat &fmt = QTextCharFormat());

    void copy();

private:
    void appendFragments(int pos, int endPos);
    int appendFragment(int pos, int endPos, int objectIndex = -1);
    int convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet = -1);
    inline int convertFormatIndex(int oldFormatIndex, int objectIndexToSet = -1)
    { return convertFormatIndex(src->formatCollection()->format(oldFormatIndex), objectIndexToSet); }
    inline QTextFormat convertFormat(const QTextFormat &fmt)
    { return dst->formatCollection()->format(convertFormatIndex(fmt)); }

    int insertPos;

    bool forceCharFormat;
    int primaryCharFormatIndex;

    QTextCursor cursor;
    QTextDocumentPrivate *dst;
    QTextDocumentPrivate *src;
    QTextFormatCollection &formatCollection;
    const QString originalText;
    QMap<int, int> objectIndexMap;
};

class QTextDocumentFragmentPrivate
{
public:
    QTextDocumentFragmentPrivate();
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);
    inline ~QTextDocumentFragmentPrivate() { delete doc; }

    void insert(QTextCursor &cursor) const;

    QAtomic ref;
    QTextDocument *doc;

    uint containsCompleteDocument : 1;
    uint importedFromPlainText : 1;
private:
    Q_DISABLE_COPY(QTextDocumentFragmentPrivate)
};

class QTextHtmlImporter : public QTextHtmlParser
{
    struct Table;
public:
    QTextHtmlImporter(QTextDocument *_doc, const QString &html, const QTextDocument *resourceProvider = 0);

    void import();

    bool containsCompleteDocument() const { return containsCompleteDoc; }

private:
    bool closeTag(int i);

    Table scanTable(int tableNodeIdx);

    void appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt = QTextCharFormat());

    struct List
    {
        QTextListFormat format;
        QPointer<QTextList> list;
    };
    QVector<List> lists;
    int indent;

    // insert a named anchor the next time we emit a char format,
    // either in a block or in regular text
    bool setNamedAnchorInNextOutput;
    QString namedAnchor;

#ifdef Q_CC_SUN
    friend struct QTextHtmlImporter::Table;
#endif
    struct TableCellIterator
    {
        inline TableCellIterator(QTextTable *t = 0) : table(t), row(0), column(0) {}

        inline TableCellIterator &operator++() {
            if (atEnd())
                return *this;
            do {
                const QTextTableCell cell = table->cellAt(row, column);
                if (!cell.isValid())
                    break;
                column += cell.columnSpan();
                if (column >= table->columns()) {
                    column = 0;
                    ++row;
                }
            } while (row < table->rows() && table->cellAt(row, column).row() != row);

            return *this;
        }

        inline bool atEnd() const { return table == 0 || row >= table->rows(); }

        QTextTableCell cell() const { return table->cellAt(row, column); }

        QTextTable *table;
        int row;
        int column;
    };

    struct Table
    {
        Table() : isTextFrame(false), rows(0), columns(0), currentRow(0) {}
        QPointer<QTextFrame> frame;
        bool isTextFrame;
        int rows;
        int columns;
        int currentRow; // ... for buggy html (see html_skipCell testcase)
        TableCellIterator currentCell;
    };
    QVector<Table> tables;

    QTextDocument *doc;
    QTextCursor cursor;
    bool containsCompleteDoc;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
