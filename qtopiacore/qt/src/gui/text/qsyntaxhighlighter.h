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

#ifndef QSYNTAXHIGHLIGHTER_H
#define QSYNTAXHIGHLIGHTER_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER

#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QTextDocument;
class QSyntaxHighlighterPrivate;
class QTextCharFormat;
class QFont;
class QColor;
class QTextBlockUserData;
class QTextEdit;

class Q_GUI_EXPORT QSyntaxHighlighter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSyntaxHighlighter)
public:
    QSyntaxHighlighter(QObject *parent);
    QSyntaxHighlighter(QTextDocument *parent);
    QSyntaxHighlighter(QTextEdit *parent);
    virtual ~QSyntaxHighlighter();

    void setDocument(QTextDocument *doc);
    QTextDocument *document() const;

public Q_SLOTS:
    void rehighlight();

protected:
    virtual void highlightBlock(const QString &text) = 0;

    void setFormat(int start, int count, const QTextCharFormat &format);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    QTextCharFormat format(int pos) const;

    int previousBlockState() const;
    int currentBlockState() const;
    void setCurrentBlockState(int newState);

    void setCurrentBlockUserData(QTextBlockUserData *data);
    QTextBlockUserData *currentBlockUserData() const;

private:
    Q_DISABLE_COPY(QSyntaxHighlighter)
    Q_PRIVATE_SLOT(d_func(), void _q_reformatBlocks(int from, int charsRemoved, int charsAdded))
    Q_PRIVATE_SLOT(d_func(), void _q_delayedRehighlight())
};

QT_END_HEADER

#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif // QSYNTAXHIGHLIGHTER_H
